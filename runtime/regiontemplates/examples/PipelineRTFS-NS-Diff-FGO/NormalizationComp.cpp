/*
 * NormalizationComp.cpp
 *
 *  GENERATED CODE
 *  DO NOT CHANGE IT MANUALLY!!!!!
 */

#include "NormalizationComp.hpp"

#include <string>
#include <sstream>

/**************************************************************************************/
/**************************** PipelineComponent functions *****************************/
/**************************************************************************************/

NormalizationComp::NormalizationComp() {
	this->setComponentName("NormalizationComp");
}

NormalizationComp::~NormalizationComp() {}

int NormalizationComp::run() {

	// Print name and id of the component instance
	std::cout << "Executing component: " << this->getComponentName() << " instance id: " << this->getId() <<std::endl;
	RegionTemplate * inputRt = this->getRegionTemplateInstance("tile");

	std::string input_img_name;
	std::string normalized_rt_name;
	float* target_mean;
	float* target_std;

	int set_cout = 0;
	for(int i=0; i<this->getArgumentsSize(); i++){
		if (this->getArgument(i)->getName().compare("input_img") == 0) {
			input_img_name = (std::string)((ArgumentString*)this->getArgument(i))->getArgValue();
			set_cout++;
		}

		if (this->getArgument(i)->getName().compare("normalized_rt") == 0) {
			normalized_rt_name = (std::string)((ArgumentString*)this->getArgument(i))->getArgValue();
			set_cout++;
		}

		if (this->getArgument(i)->getName().compare("target_mean") == 0) {
			target_mean = (float*)((ArgumentFloatArray*)this->getArgument(i))->getArgValue();
			set_cout++;
		}

		if (this->getArgument(i)->getName().compare("target_std") == 0) {
			target_std = (float*)((ArgumentFloatArray*)this->getArgument(i))->getArgValue();
			set_cout++;
		}

	}

	if (set_cout < this->getArgumentsSize())
		std::cout << __FILE__ << ":" << __LINE__ <<" Missing common arguments on NormalizationComp" << std::endl;

	this->addInputOutputDataRegion("tile", input_img_name, RTPipelineComponentBase::INPUT);

	this->addInputOutputDataRegion("tile", normalized_rt_name, RTPipelineComponentBase::OUTPUT);


	if(inputRt != NULL){
		DenseDataRegion2D *input_img = NULL;

		DenseDataRegion2D *normalized_rt = NULL;

		try{
			input_img = dynamic_cast<DenseDataRegion2D*>(inputRt->getDataRegion(input_img_name, "", 0, workflow_id));

			normalized_rt = dynamic_cast<DenseDataRegion2D*>(inputRt->getDataRegion(normalized_rt_name, "", 0, workflow_id));

			std::cout << "NormalizationComp. paramenterId: "<< workflow_id <<std::endl;
		}catch(...){
			std::cout <<"ERROR NormalizationComp " << std::endl;
		}

		// Create processing task
		TaskNormalizationComp * task = new TaskNormalizationComp(input_img, normalized_rt, target_mean, target_std);

		this->executeTask(task);


	}else{
		std::cout << __FILE__ << ":" << __LINE__ <<" RT == NULL" << std::endl;
	}

	return 0;
}

// Create the component factory
PipelineComponentBase* componentFactoryNormalizationComp() {
	return new NormalizationComp();
}

// register factory with the runtime system
bool registeredNormalizationComp = PipelineComponentBase::ComponentFactory::componentRegister("NormalizationComp", &componentFactoryNormalizationComp);


/**************************************************************************************/
/*********************************** Task functions ***********************************/
/**************************************************************************************/

TaskNormalizationComp::TaskNormalizationComp(DenseDataRegion2D* input_img_temp, DenseDataRegion2D* normalized_rt_temp, float* target_mean, float* target_std) {
	
	this->input_img_temp = input_img_temp;
	this->normalized_rt_temp = normalized_rt_temp;
	this->target_mean = target_mean;
	this->target_std = target_std;

	
}

TaskNormalizationComp::~TaskNormalizationComp() {
	if(input_img_temp != NULL) delete input_img_temp;

}

bool TaskNormalizationComp::run(int procType, int tid) {
	
	cv::Mat input_img = this->input_img_temp->getData();

	cv::Mat normalized_rt = this->normalized_rt_temp->getData();

	uint64_t t1 = Util::ClockGetTimeProfile();

	std::cout << "TaskNormalizationComp executing." << std::endl;	

	normalized_rt = ::nscale::Normalization::normalization(input_img, target_mean, target_std);
	
	this->normalized_rt_temp->setData(normalized_rt);

	uint64_t t2 = Util::ClockGetTimeProfile();

	std::cout << "Task NormalizationComp time elapsed: "<< t2-t1 << std::endl;
}
