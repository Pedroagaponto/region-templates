#include "fgm.hpp"

void generate_drs(RegionTemplate* rt, 
	const std::map<int, ArgumentBase*> &expanded_args) {

	// verify every argument
	for (pair<int, ArgumentBase*> p : expanded_args) {
		// if an argument is a region template data region
		if (p.second->getType() == ArgumentBase::RT) {
			if (((ArgumentRT*)p.second)->isFileInput) {
				// create the data region and add it to the input region template
				DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
				ddr2d->setName(p.second->getName());
				std::ostringstream oss;
				oss << p.first;
				ddr2d->setId(oss.str());
				ddr2d->setVersion(p.first);
				ddr2d->setIsAppInput(true);
				ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
				ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
				ddr2d->setInputFileName(p.second->toString());
				rt->insertDataRegion(ddr2d);
			}
		}
	}
}

void generate_drs(RegionTemplate* rt, PipelineComponentBase* stage,
	const std::map<int, ArgumentBase*> &expanded_args) {

	// verify every argument
	for (int inp : stage->getInputs()) {
		// if an argument is a region template data region
		if (expanded_args.at(inp)->getType() == ArgumentBase::RT) {
			// if (((ArgumentRT*)expanded_args.at(inp))->isFileInput) {
				// create the data region and add it to the input region template
				DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
				ddr2d->setName(expanded_args.at(inp)->getName());
				std::ostringstream oss;
				oss << inp;
				ddr2d->setId(oss.str());
				ddr2d->setVersion(inp);
				ddr2d->setIsAppInput(true);
				ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
				ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
				ddr2d->setInputFileName(expanded_args.at(inp)->toString());
				rt->insertDataRegion(ddr2d);
			// }
		}
	}
}

void add_arguments_to_stages(std::map<int, PipelineComponentBase*> &merged_stages, 
	std::map<int, ArgumentBase*> &merged_arguments, string name) {

	int i=0;
	for (pair<int, PipelineComponentBase*>&& stage : merged_stages) {
		// create the RT isntance
		RegionTemplate *rt = new RegionTemplate();
		rt->setName(name);

		// add input arguments to stage, adding them as RT as needed
		for (int arg_id : stage.second->getInputs()) {
			ArgumentBase* new_arg = merged_arguments[arg_id]->clone();
			new_arg->setParent(merged_arguments[arg_id]->getParent());
			stage.second->addArgument(new_arg);
			if (new_arg->getType() == ArgumentBase::RT) {
				// std::cout << "input RT : " << merged_arguments[arg_id]->getName() << std::endl;
				// insert the region template on the parent stage if the argument is a DR and if the RT wasn't already added
				if (((RTPipelineComponentBase*)stage.second)->
						getRegionTemplateInstance(rt->getName()) == NULL) {
					((RTPipelineComponentBase*)stage.second)->addRegionTemplateInstance(rt, rt->getName());
				}
				((RTPipelineComponentBase*)stage.second)->addInputOutputDataRegion(
					rt->getName(), new_arg->getName(), RTPipelineComponentBase::INPUT);
			}
			if (merged_arguments[arg_id]->getParent() != 0) {
				// verify if the dependency stage was reused
				int parent = merged_arguments[arg_id]->getParent();
				// std::cout << "[before]Dependency: " << stage.second->getId() << ":" << stage.second->getName()
				// 	<< " ->addDependency( " << parent << " )" << std::endl;
				if (merged_stages[merged_arguments[arg_id]->getParent()]->reused != NULL)
					parent = merged_stages[merged_arguments[arg_id]->getParent()]->reused->getId();
				// std::cout << "Dependency: " << stage.second->getId() << ":" << stage.second->getName()
				// 	<< " ->addDependency( " << parent << " )" << std::endl;
				((RTPipelineComponentBase*)stage.second)->addDependency(parent);
			}
		}

		// add output arguments to stage, adding them as RT as needed
		for (int arg_id : stage.second->getOutputs()) {
			ArgumentBase* new_arg = merged_arguments[arg_id]->clone();
			new_arg->setParent(merged_arguments[arg_id]->getParent());
			new_arg->setIo(ArgumentBase::output);
			stage.second->addArgument(new_arg);
			if (merged_arguments[arg_id]->getType() == ArgumentBase::RT) {
				// std::cout << "output RT : " << merged_arguments[arg_id]->getName() << std::endl;
				// insert the region template on the parent stage if the argument is a DR and if the RT wasn't already added
				if (((RTPipelineComponentBase*)stage.second)->getRegionTemplateInstance(rt->getName()) == NULL)
					((RTPipelineComponentBase*)stage.second)->addRegionTemplateInstance(rt, rt->getName());
				((RTPipelineComponentBase*)stage.second)->addInputOutputDataRegion(rt->getName(), 
					merged_arguments[arg_id]->getName(), RTPipelineComponentBase::OUTPUT);
			}
		}
	}
}

void fgm::merge_stages_fine_grain(int algorithm, const std::map<int, PipelineComponentBase*> &all_stages, 
	const std::map<int, PipelineComponentBase*> &stages_ref, std::map<int, PipelineComponentBase*> &merged_stages, 
	std::map<int, ArgumentBase*> expanded_args, int max_bucket_size, bool shuffle, string dakota_filename) {

	// attempt merging for each stage type
	for (std::map<int, PipelineComponentBase*>::const_iterator ref=stages_ref.cbegin(); ref!=stages_ref.cend(); ref++) {
		// get only the stages from the current stage_ref
		std::list<PipelineComponentBase*> current_stages;
		filter_stages(all_stages, ref->second->getName(), current_stages, shuffle);

		std::cout << "[merge_stages_fine_grain] Generating tasks..." << std::endl;
		
		// generate all tasks
		int nrS = 0;
		double max_nrS_mksp = 0;
		for (list<PipelineComponentBase*>::iterator s=current_stages.begin(); s!=current_stages.end(); ) {
			// if the stage isn't composed of reusable tasks then 
			(*s)->tasks = task_generator(ref->second->tasksDesc, *s, expanded_args);
			if ((*s)->tasks.size() == 0) {
				merged_stages[(*s)->getId()] = *s;
				
				// makespan calculations
				nrS++;
				if ((*s)->getMksp() > max_nrS_mksp)
					max_nrS_mksp = (*s)->getMksp();

				s = current_stages.erase(s);
			} else
				s++;
		}

		// if there are no stages left to attempt to merge, or only one stage, don't perform any merging
		if (current_stages.size() == 1) {
			merged_stages[(*current_stages.begin())->getId()] = *current_stages.begin();
			continue;
		} else if (current_stages.size() == 0) {
			continue;
		}

		std::list<std::list<PipelineComponentBase*>> solution;
		int max_cuts = ceil(current_stages.size()/max_bucket_size);

		switch (algorithm) {
			case 0:
				// no fine grain merging - ok
				for (PipelineComponentBase* p : current_stages) {
					std::list<PipelineComponentBase*> single_stage_bucket;
					single_stage_bucket.emplace_back(p);
					solution.emplace_back(single_stage_bucket);
				}
				break;

			case 1:
				// naive merging - ok
				for (std::list<PipelineComponentBase*>::iterator s=current_stages.begin(); s!=current_stages.end(); s++) {
					int i;
					std::list<PipelineComponentBase*> bucket;
					bucket.emplace_back(*s);
					for (i=1; i<ceil(max_bucket_size); i++) {
						if ((++s)==current_stages.end())
							break;
						bucket.emplace_back(*s);
						std::cout << "\tadded " << (*s)->getId() << " to the bucket" << std::endl;
					}
					solution.emplace_back(bucket);
					if (s==current_stages.end())
						break;
				}
				break;

			case 2:
				// smart recursive cut - ok
				solution = recursive_cut(current_stages, all_stages, 
					max_bucket_size, max_cuts, expanded_args, ref->second->tasksDesc);
				break;

			case 3:
				// reuse-tree merging - ok
				solution = reuse_tree_merging(current_stages, all_stages, 
					max_bucket_size, expanded_args, ref->second->tasksDesc, false);
				break;
			
			case 4:
				// reuse-tree merging with double prunning
				solution = reuse_tree_merging(current_stages, all_stages, 
					max_bucket_size, expanded_args, ref->second->tasksDesc, true);
				break;
		}

		// write merging solution
		ofstream solution_file;
		solution_file.open(dakota_filename + "-b" + std::to_string(max_bucket_size) + "merging_solution.log", ios::trunc);

		std::cout << std::endl << "solution:" << std::endl;
		solution_file << "solution:" << std::endl;
		int total_tasks=0;
		for (std::list<PipelineComponentBase*> b : solution) {
			std::cout << "\tbucket with " << b.size() << " stages and cost "
				<< calc_stage_proc(b, expanded_args, ref->second->tasksDesc) << ":" << std::endl;
			solution_file << "\tbucket with " << b.size() << " stages and cost "
				<< calc_stage_proc(b, expanded_args, ref->second->tasksDesc) << ":" << std::endl;
			total_tasks += calc_stage_proc(b, expanded_args, ref->second->tasksDesc);
			for (PipelineComponentBase* s : b) {
				std::cout << "\t\tstage " << s->getId() << ":" << s->getName() << ":" << std::endl;
				// solution_file << "\t\tstage " << s->getId() << ":" << s->getName() << ":" << std::endl;
			}
		}
		solution_file.close();

		// write some statistics abou the solution
		ofstream statistics_file;
		statistics_file.open(dakota_filename + "-b" + std::to_string(max_bucket_size) + "merging_statistics.log", ios::trunc);

		statistics_file << current_stages.size() << "\t";
		statistics_file << current_stages.size()*ref->second->tasksDesc.size() << "\t";
		statistics_file << total_tasks << "\t";
		statistics_file << solution.size() << "\t";
		statistics_file << total_tasks/solution.size() << "\t";

		statistics_file.close();

		// merge all stages in each bucket, given that they are mergable
		for (std::list<PipelineComponentBase*> bucket : solution) {
			// std::cout << "bucket merging" << std::endl;
			std::list<PipelineComponentBase*> curr = merge_stages_full(bucket, expanded_args, ref->second->tasksDesc);
			// send rem stages to merged_stages
			for (PipelineComponentBase* s : curr) {
				// std::cout << "\tadding stage " << s->getId() << std::endl;
				merged_stages[s->getId()] = s;
			}
		}
	}
}
