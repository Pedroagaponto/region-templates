/*
 * ThreadPool.cpp
 *
 *  Created on: Aug 18, 2011
 *      Author: george
 */

#include "ThreadPool.h"

void *callThread(void *arg){
	ThreadPool *tp = (ThreadPool *)((threadData*) arg)->threadPoolPtr;
	int procType = (int)((threadData*) arg)->procType;
	int tid = (int)((threadData*) arg)->tid;
	int cpuId=tid+2;


	tp->processTasks(procType, tid);
	free(arg);
	pthread_exit(NULL);
}

ThreadPool::ThreadPool(TasksQueue *tasksQueue, ExecutionEngine *execEngine) {
	// init thread pool with user provided task queue
	this->tasksQueue = tasksQueue;
	this->execEngine = execEngine;

	CPUWorkerThreads = NULL;

	pthread_mutex_init(&initExecutionMutex, NULL);
	pthread_mutex_lock(&initExecutionMutex);

	pthread_mutex_init(&createdThreads, NULL);
	pthread_mutex_lock(&createdThreads);

	numCPUThreads = 0;
	firstToFinish=true;
	execDone=false;
}

ThreadPool::~ThreadPool() {
	execDone=true;
	this->finishExecWaitEnd();

	if(CPUWorkerThreads != NULL){
		free(CPUWorkerThreads);
	}

	pthread_mutex_destroy(&initExecutionMutex);

	float loadImbalance = 0.0;
	if(numCPUThreads > 1){
		// calculate time in microseconds
		double tS = firstToFinishTime.tv_sec*1000000 + (firstToFinishTime.tv_usec);
		double tE = lastToFinishTime.tv_sec*1000000  + (lastToFinishTime.tv_usec);
		loadImbalance = (tE - tS)/1000000.0;
	}
	printf("Load imbalance = %f\n", loadImbalance);

}

//void * ThreadPool::getGPUTempData(int tid){
//	//return this->gpuTempData[tid];
//}

bool ThreadPool::createThreadPool(int cpuThreads, int *cpuThreadsCoreMapping, int gpuThreads, int *gpuThreadsCoreMapping, bool dataLocalityAware, bool prefetching)
{

//	this->gpuTempDataSize = gpuTempDataSize;
	this->dataLocalityAware = dataLocalityAware;
	this->prefetching = prefetching;
	if(prefetching)
		std::cout << "Prefetching is on!"<<std::endl;

	// Create CPU threads.
	if(cpuThreads > 0){
		numCPUThreads = cpuThreads;
		CPUWorkerThreads = (pthread_t *) malloc(sizeof(pthread_t) * cpuThreads);

		for (int i = 0; i < cpuThreads; i++ ){
			threadData *arg = (threadData *) malloc(sizeof(threadData));
			arg->tid = i;
			arg->procType = ExecEngineConstants::CPU;
			arg->threadPoolPtr = this;
			int ret = pthread_create(&(CPUWorkerThreads[arg->tid]), NULL, callThread, (void *)arg);
			if (ret){
				printf("ERROR: Return code from pthread_create() is %d\n", ret);
				exit(-1);
			}

			// wait untill thead is created
			pthread_mutex_lock(&createdThreads);
		}
	}

	return true;
}


void ThreadPool::initExecution()
{
	pthread_mutex_unlock(&initExecutionMutex);
}

void ThreadPool::enqueueUploadTaskParameters(Task* task, cv::gpu::Stream& stream) {
	for(int i = 0; i < task->getNumberArguments(); i++){
		task->getArgument(i)->upload(stream);
	}
}


void ThreadPool::downloadTaskOutputParameters(Task* task, cv::gpu::Stream& stream) {
	for(int i = 0; i < task->getNumberArguments(); i++){
		// Download only those parameters that are of type: output, or input_output.
		// Input type arguments can be deleted directly.
		if(task->getArgument(i)->getType() != ExecEngineConstants::INPUT){
			task->getArgument(i)->download(stream);
		}
	}
	stream.waitForCompletion();

	for(int i = 0; i < task->getNumberArguments(); i++){
		task->getArgument(i)->deleteGPUData();
	}

}

void ThreadPool::enqueueDownloadTaskParameters(Task* task, cv::gpu::Stream& stream) {
	for(int i = 0; i < task->getNumberArguments(); i++){
		task->getArgument(i)->download(stream);
	}
}

void ThreadPool::deleteOutputParameters(Task* task) {
	for(int i = 0; i < task->getNumberArguments(); i++){
		task->getArgument(i)->deleteGPUData();
	}
}

void ThreadPool::preassignmentSelectiveDownload(Task* task, Task* preAssigned, cv::gpu::Stream& stream) {
	vector<int> downloadingTasksIds;

	// Perform parameter matching, and download what will not be used
	// for all parameters of the current task
	for(int i = 0; i < task->getNumberArguments(); i++){
		// check if matches one parameter of the preAssigned task
		int j;
		for(j = 0; j < preAssigned->getNumberArguments(); j++){
			// if matches, leave the loop
			if(task->getArgument(i)->getId() == preAssigned->getArgument(j)->getId() ){
				break;
			}
		}
		// Did not find a matching, download
		if(j == preAssigned->getNumberArguments()){
//			std::cout << "Parameters i = "<< i <<" did not match!" << std::endl;
			if(task->getArgument(i)->getType() != ExecEngineConstants::INPUT){
//				std::cout << "	Type is not input = "<< task->getArgument(i)->getType() <<std::endl;
				task->getArgument(i)->download(stream);
				downloadingTasksIds.push_back(i);

			}else{
//				std::cout << "	Type is input only = "<< task->getArgument(i)->getType() <<std::endl;
				task->getArgument(i)->deleteGPUData();
			}
		}
	}
	stream.waitForCompletion();
	// Now release GPU memory for arguments that we have downloaded
	for(int i = 0; i < downloadingTasksIds.size(); i++){
		task->getArgument(downloadingTasksIds[i])->deleteGPUData();
	}

}


void ThreadPool::processTasks(int procType, int tid)
{
	// Inform that this threads was created.
	//sem_post(&createdThreads);
	pthread_mutex_unlock(&createdThreads);

	printf("procType:%d  tid:%d waiting init of execution\n", procType, tid);
	pthread_mutex_lock(&initExecutionMutex);
	pthread_mutex_unlock(&initExecutionMutex);

	Task* curTask = NULL;
	Task* preAssigned = NULL;
	Task* prefetchTask = NULL;
	Task* downloadingTask = NULL;
	vector<int> downloadArgIds;

	// ProcessTime example
	struct timeval startTime;
	struct timeval endTime;

	while(true){

		curTask = this->tasksQueue->getTask(procType);

		afterGetTask:

		// Did not succeed in returning a task for computation
		if(curTask == NULL){//
			// if It is not the end of execution
			if(this->execEngine->getCountTasksPending() != 0 || this->execDone == false){

				// Did not actually took a task, so increment the number of tasks processed
				this->tasksQueue->incrementTasksToProcess();

				usleep(10000);

				// All tasks ready to execute are finished, but there still existing tasks pending due unsolved dependencies
				continue;
			}else{
				printf("procType:%d  tid:%d Task NULL #t_pending=%d\n", procType, tid, this->execEngine->getCountTasksPending());
				break;
			}
		}else{
			// It this tasks was canceled, resolve its dependencies and try to get another task
			if(curTask->getStatus() != ExecEngineConstants::ACTIVE ){//

				if(curTask->getTaskType() == ExecEngineConstants::TRANSACTION_TASK){
					curTask->run(procType, tid);
				}

				delete curTask;
				continue;
			}

		}

		procPoint:

		gettimeofday(&startTime, NULL);//

		double tSComp = startTime.tv_sec*1000000 + (startTime.tv_usec);

//		printf("StartTime:%f\n",(tSComp)/1000000);
		try{

			std::cout << "Executing, task.id: "<< curTask->getId() << std::endl;
			curTask->run(procType, tid);

			if(curTask->getStatus() != ExecEngineConstants::ACTIVE){

				delete curTask;

				continue;
			}

		}catch(...){
			printf("ERROR in tasks execution. EXCEPTION\n");
			curTask->setStatus(ExecEngineConstants::CANCELED);

			// TODO: delete any available GPU data, and continue (what if the CPU is cancelled)
			delete curTask;
			continue;
		}

		gettimeofday(&endTime, NULL);



		// calculate time in microseconds
		double tS = startTime.tv_sec*1000000 + (startTime.tv_usec);
		double tE = endTime.tv_sec*1000000  + (endTime.tv_usec);
		printf("procType:%d taskId:%d  tid:%d procTime = %f\n", procType, curTask->getId(), tid,  (tE-tS)/1000000);

		// October 04, 2013. Commenting out line bellow to work with GPUs without compiling w/ cuda/gpu suppport
	//	if(procType == ExecEngineConstants::CPU){
			try{
				delete curTask;
			}catch(...){
				printf("ERROR DELETE\n");
			}
//		}

//		if(curTask != NULL)
//			delete curTask;
//
//		if(preAssigned != NULL){
//			curTask = preAssigned;
//			preAssigned = NULL;
//			goto afterGetTask;
//		}

	}

	printf("Leaving procType:%d  tid:%d #t_pending=%d\n", procType, tid, this->execEngine->getCountTasksPending());
//	if(ExecEngineConstants::GPU == procType && gpuTempDataSize >0){
//		cudaFreeMemWrapper(gpuTempData[tid]);
//	}
	// I don't care about controlling concurrent access here. Whether two threads
	// enter this if, their resulting gettimeofday will be very similar if not the same.
	if(firstToFinish){
		firstToFinish = false;
		gettimeofday(&firstToFinishTime, NULL);
	}

	gettimeofday(&lastToFinishTime, NULL);
}

int ThreadPool::getGPUThreads()
{
	return numGPUThreads;
}



void ThreadPool::finishExecWaitEnd()
{
	// make sure to init the execution if was not done by the user.
	initExecution();

	for(int i= 0; i < numCPUThreads; i++){
		pthread_join(CPUWorkerThreads[i] , NULL);
	}
	for(int i= 0; i < numGPUThreads; i++){
		pthread_join(GPUWorkerThreads[i] , NULL);
	}

}


int ThreadPool::getCPUThreads()
{
	return numCPUThreads;
}
