/*
 * ReusableTask.hpp
 *
 *  Created on: Jul 18, 2016
 *      Author: willian
 */

#ifndef REUSABLE_TASK_H_
#define REUSABLE_TASK_H_

#include <map>

#include "Task.h"
#include "Argument.h"
#include "RegionTemplate.h"
#include "PipelineComponentBase.h"


// Define factory function type that creates objects of type ReusableTask and its subclasses
class ReusableTask;
class RegionTemplate;
class PipelineComponentBase;
typedef ReusableTask* (task_factory_t1)(list<ArgumentBase*> args, RegionTemplate* inputRt);
typedef ReusableTask* (task_factory_t2)();

class ReusableTask: public Task {
public:
	// list of tasks' ids that are dependent on this task
	int parentTask;
    vector<int> parentTasks;
    int accumulate, height;

	// if the task isn't going to be executed then it's a mock
	// used to enable corect arguments delete, preventing mem leaking
	bool mock;

	ReusableTask() {parentTask = -1;mock = true;};
	virtual ~ReusableTask() {};

	virtual bool reusable(ReusableTask* t) = 0;
	virtual void updateDR(RegionTemplate* rt) = 0;
	// sets the interstage arguments with the pointers of the task t
	virtual void updateInterStageArgs(ReusableTask* t) = 0;
	virtual void resolveDependencies(ReusableTask* t) = 0;

	// Write component data to a buffer
	virtual int serialize(char *buff) = 0;
	// Initialize component data from a buffer generated by serialize function
	virtual int deserialize(char *buff) = 0;
	virtual ReusableTask* clone() = 0;
	virtual int size() = 0;

	virtual void print() = 0;


	// Factory class is used to build "reflection", and instantiate objects of
	// ReusableTask subclasses that register with it
	class ReusableTaskFactory {
	private:
		// This maps name of task types to the function that creates instances of those components
		static std::map<std::string,task_factory_t1*> factoryMap1;
		static std::map<std::string,task_factory_t2*> factoryMap2;

	public:
		// Used to register the task factory function with this factory class
		static bool taskRegister(std::string name, task_factory_t1 *compFactory1, task_factory_t2 *compFactory2);

		// Retrieve pointer to function that creates task registered with name="name"
		static task_factory_t1 *getTaskFactory1(std::string name);
		static task_factory_t2 *getTaskFactory2(std::string name);

		// Retrieve instance of task registered as "name"
		static ReusableTask *getTaskFromName(std::string name, list<ArgumentBase*> args, RegionTemplate* inputRt);

		// constructor used only on PipelineComponentBase clone method
		static ReusableTask *getTaskFromName(std::string name);
		// ExecutionEngine *getResourceManager() const;
	};
	
};

#endif /* REUSABLE_TASK_H_ */
