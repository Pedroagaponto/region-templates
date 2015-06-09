/*
 * RTPipelineComponentBase.h
 *
 *  Created on: Feb 13, 2013
 *      Author: george
 */

#ifndef RTPIPELINECOMPONENTBASE_H_
#define RTPIPELINECOMPONENTBASE_H_

#include "Constants.h"
#include "PipelineComponentBase.h"
#include "RegionTemplate.h"
#include "DataRegionFactory.h"
#include "./util/Util.h"
#include "TaskIO.h"
#include <set>

class PipelineComponentBase;


class RTPipelineComponentBase : public PipelineComponentBase {
	private:
		// names of the region templates that this component processes: /namespace/name
		//	std::vector<std::string> regionTemplateNames;

		std::set<std::pair<std::string, std::string> > input_data_regions;
		std::set<std::pair<std::string, std::string> > output_data_regions;

		// Instances of the region templates within required path to be
		// processed by an instance of this pipeline
		std::map<std::string, RegionTemplate*> regionTemplates;

		DataRegionFactory drf;
	//	Cache *cachePtr;

	//protected:
		void setCache(Cache *cache);
		void setLocation(int location);

	public:
		RTPipelineComponentBase();
		virtual ~RTPipelineComponentBase();

		void addInputOutputDataRegion(std::string regionTemplateName, std::string dataRegionName, int type=RTPipelineComponentBase::INPUT);

		// Associates a particular instances of region template to
		// this pipeline stage instance
		int addRegionTemplateInstance(RegionTemplate *rt, std::string name);

		// Retrieves a particular instance of region template associated
		// with this pipeline instance for computation
		RegionTemplate *getRegionTemplateInstance(std::string dataRegionName);

		// Retrieves a particular instance of region template associated with an index
		RegionTemplate *getRegionTemplateInstance(int index);

		void updateRegionTemplateInfo(RegionTemplate *rt);

		// Retrieve number of region templates instances available in this component container
		int getNumRegionTemplates();

		int instantiateRegionTemplates();
		int stageRegionTemplates();
		int createIOTask();

		// Write component data to a buffer
		int serialize(char *buff);

		// Initialize component data from a buffer generated by serialize function
		int deserialize(char *buff);

		// Serialization size: number of bytes need to store components
		int size();

		// amount of bytes used if given data RTPipelineComponent is schedule for execution w/ Worker=workerId
		long getAmountOfDataReuse(int workerId);

		static const int INPUT		=	1;
		static const int OUTPUT		=	2;
		static const int INPUT_OUTPUT	=	3;
};

#endif /* RTPIPELINECOMPONENTBASE_H_ */
