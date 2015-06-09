/*
 * DataRegionFactory.cpp
 *
 *  Created on: Feb 15, 2013
 *      Author: george
 */

#include "DataRegionFactory.h"

DataRegionFactory::DataRegionFactory() {
}

DataRegionFactory::~DataRegionFactory() {
}

std::string number2String(int x){
	std::ostringstream ss;
	ss<<x;
	return ss.str();
}

bool DataRegionFactory::readDDR2DFS(DenseDataRegion2D *dataRegion, int chunkId, std::string path){
	if(chunkId == -1){
		cv::Mat chunkData;
		std::cout << "readDDR2DFS: dataRegion: "<<dataRegion->getName()<< " id: "<< dataRegion->getId()<<" outputExt: "<< dataRegion->getOutputExtension() << std::endl;
		if(dataRegion->getOutputExtension() == DataRegion::XML){
			// if it is an Mat stored as a XML file
			std::string inputFile;

			if(dataRegion->getIsAppInput()){
				inputFile = dataRegion->getInputFileName();
			}else{
				if(!path.empty())inputFile.append(path);

				inputFile.append(dataRegion->getName());
				inputFile.append("-").append(dataRegion->getId());
				inputFile.append("-").append(number2String(dataRegion->getVersion()));
				inputFile.append("-").append(number2String(dataRegion->getTimestamp()));

				inputFile.append(".xml");

				// create lock.
				std::string lockFile = inputFile;
				lockFile.append(".loc");
				FILE *pFile = fopen(lockFile.c_str(), "r");
				if(pFile == NULL){
					std::cout << "ERROR: lock file: "<< lockFile << std::endl;
					return false;
				}

			}
			cv::FileStorage fs2(inputFile, cv::FileStorage::READ);
			if(fs2.isOpened()){
				fs2["mat"] >> chunkData;
				fs2.release();
				std::cout << "LOADING XML input data: " << inputFile << " dataRegion: "<< dataRegion->getName() << " chunk.rows: "<< chunkData.rows<< " chunk.cols: "<< chunkData.cols<<std::endl;
			}else{
				std::cout << "Failed to read Data region. Failed to open FILE: " << inputFile << std::endl;
			}
		}else{
			if(dataRegion->getOutputExtension() == DataRegion::PBM){
				std::string inputFile;

				if(dataRegion->getIsAppInput()){
					inputFile = dataRegion->getInputFileName();
				}else{
					if(!path.empty())inputFile.append(path);

					inputFile.append(dataRegion->getName());
					inputFile.append("-").append(dataRegion->getId());
					inputFile.append("-").append(number2String(dataRegion->getVersion()));
					inputFile.append("-").append(number2String(dataRegion->getTimestamp()));

					inputFile.append(".pbm");
					// create lock.
					std::string lockFile = inputFile;
					lockFile.append(".loc");
					FILE *pFile = fopen(lockFile.c_str(), "r");
					if(pFile == NULL){
						std::cout << "ERROR:::: could not read lock file:"<< lockFile << std::endl;
						return false;
					}
				}
				// It is stored as an image
				chunkData = cv::imread(inputFile, -1);
				//chunkData = cv::imread(dataRegion->getId(), -1);
				if(chunkData.empty()){
					std::cout << "Failed to read image:" << inputFile << std::endl;

				}else{
					BoundingBox ROIBB (Point(0, 0, 0), Point(chunkData.cols-1, chunkData.rows-1, 0));
					dataRegion->setData(chunkData);
					dataRegion->setBb(ROIBB);
				}

			}
		}
	}else{
		// Get info about the Id of the file in which the data is stored
		std::pair<BoundingBox, std::string> data_pair = dataRegion->getBB2IdElement(chunkId);
		if(data_pair.second.size() > 0){

			// read the data file
			cv::Mat data = cv::imread(data_pair.second, -1);
			if(data.empty()){
				std::cout << "Failed to read image:" << dataRegion->getId() << std::endl;
				return false;
			}else{
				// if it was successfully, insert data into the data region vector chunked data
				dataRegion->insertChukedData(data_pair.first, data);
			}
		}else{
			return false;
		}

	}
	return true;
}

bool DataRegionFactory::writeDDR2DFS(DenseDataRegion2D* dataRegion, std::string path) {
	bool retVal = true;
//	if(dataRegion->getData().rows != 0 && dataRegion->getData().cols !=0){
		std::cout << "DataRegion: "<< dataRegion->getName() << " extension: "<< dataRegion->getOutputExtension() << std::endl;
		if(dataRegion->getOutputExtension() == DataRegion::PBM){
			std::string outputFile;
			if(!path.empty())outputFile.append(path);


			outputFile.append(dataRegion->getName());
			outputFile.append("-").append(dataRegion->getId());
			outputFile.append("-").append(number2String(dataRegion->getVersion()));
			outputFile.append("-").append(number2String(dataRegion->getTimestamp()));

			outputFile.append(".pbm");

			std::cout << "rows: "<< dataRegion->getData().rows <<std::endl;
			retVal = cv::imwrite(outputFile, dataRegion->getData());

			// create lock.
			outputFile.append(".loc");
			FILE *pFile = fopen(outputFile.c_str(), "w");
			if(pFile == NULL){
				std::cout << "ERROR:::: could not write lock file" << std::endl;
			}

		}else{
			if(dataRegion->getOutputExtension() == DataRegion::XML){
				std::string outputFile;
				if(!path.empty()) outputFile.append(path);

				outputFile.append(dataRegion->getName());
				outputFile.append("-").append(dataRegion->getId());
				outputFile.append("-").append(number2String(dataRegion->getVersion()));
				outputFile.append("-").append(number2String(dataRegion->getTimestamp()));
				outputFile.append(".xml");

				cv::FileStorage fs(outputFile, cv::FileStorage::WRITE);
				// check if file has been opened correctly and write data
				if(fs.isOpened()){
					fs << "mat" << dataRegion->getData();
					fs.release();
				}else{
					retVal =false;
				}
				// create lock.
				outputFile.append(".loc");
				FILE *pFile = fopen(outputFile.c_str(), "w");
				if(pFile == NULL){
					std::cout << "ERROR:::: could not write lock file" << std::endl;
				}
			}else{
				std::cout << "UNKNOW file extension: "<< dataRegion->getOutputExtension() << std::endl;
			}
		}
//	}
	return retVal;
}

bool DataRegionFactory::readDDR2DATASPACES(DenseDataRegion2D *dataRegion) {
	int getReturn = 0;
#ifdef	WITH_DATA_SPACES

	std::cout << "ReadDDRDATASPACES: "<< dataRegion->getName() << std::endl; 
	
	if(dataRegion->getName().compare("BGR") == 0){
		cv::Mat dataChunk(4096, 4096, CV_8UC3, 0.0);
		char *matrix = dataChunk.ptr<char>(0);
        std::cout << "DATASPACES get BGR: "<< dataRegion->getId() << std::endl;
//                begin = Util::ClockGetTime();
                getReturn = dspaces_get(dataRegion->getId().c_str(), 1, sizeof(char), 0, 0, 0, (4096*3)-1, 4095, 0, matrix);
//                end = Util::ClockGetTime();
		if(getReturn < 0) dataChunk.release();
                std::cout << "DataSpaces Get return: "<< getReturn << std::endl;
		dataRegion->setData(dataChunk);
	}

	if(dataRegion->getName().compare("mask") == 0){
		cv::Mat dataChunk(4096, 4096, CV_8U);
		char *matrix = dataChunk.ptr<char>(0);
                std::cout << "DATASPACES get mask: "<< dataRegion->getId() << std::endl;
//                begin = Util::ClockGetTime();
                getReturn = dspaces_get(dataRegion->getId().c_str(), 1, sizeof(char), 0, 0, 0, 4095, 4095, 0, matrix);
//                end = Util::ClockGetTime();
		if(getReturn < 0) dataChunk.release();
                std::cout << "DataSpaces Get return: "<< getReturn << std::endl;
		dataRegion->setData(dataChunk);
	}

#endif
	return true;
}



bool DataRegionFactory::writeDDR2DATASPACES(DenseDataRegion2D* dataRegion) {
#ifdef WITH_DATA_SPACES
	std::cout << "Writing data region do DataSpaces: "<< dataRegion->getName() << std::endl;

//	cv::imwrite(dataRegion->getId(), dataRegion->getData());
	if(dataRegion->getData().cols > 0 && dataRegion->getData().rows > 0){
		char *matrix = dataRegion->getData().ptr<char>(0);

		std::cout << "dataSpaces put, dataregion: "<< dataRegion->getId() << std::endl;

		dspaces_put(dataRegion->getId().c_str(), 1, sizeof(char), 0, 0, 0, (dataRegion->getData().cols * dataRegion->getData().channels())-1, dataRegion->getData().rows-1, 0, matrix);
		dspaces_put_sync();
	}else{
		std::cout << "Data region: "<< dataRegion->getName() << " is empty. Not writing to DS!" << std::endl;
	}

#endif
	return true;
}


bool DataRegionFactory::instantiateDataRegion(DataRegion* dr, int chunkId, std::string path) {
	DenseDataRegion2D *dr2D = dynamic_cast<DenseDataRegion2D*>(dr);
	
	switch(dr->getType()){
		case RegionTemplateType::DENSE_REGION_2D:

			switch(dr->getInputType()){
				case DataSourceType::FILE_SYSTEM:
					readDDR2DFS(dr2D, chunkId, path);
					break;
				case DataSourceType::DATA_SPACES:
					readDDR2DATASPACES(dr2D);
//					std::cout << "Data Spaces data source not implemented yet" << std::endl;
					break;
				default:
					std::cout << "Unknown data source type:" << dr->getInputType() << std::endl;
					break;
			}
			break;
		case RegionTemplateType::DENSE_REGION_3D:
			std::cout << "Dense data region 3D instantiation to be implemented" << std::endl;
			break;
		default:
			std::cout << "Unknown data region type:"<< dr->getType() << " name:" << dr->getName() <<std::endl;
			break;
	}
	return true;
}

bool DataRegionFactory::stageDataRegion(DataRegion* dr) {
	DenseDataRegion2D *dr2D = dynamic_cast<DenseDataRegion2D*>(dr);

	switch(dr->getType()){
		case RegionTemplateType::DENSE_REGION_2D:
			std::cout << dr->getName() << " outpuType: " <<dr->getOutputType() << std::endl; 
			switch(dr->getOutputType()){
				case DataSourceType::FILE_SYSTEM:
					std::cout << "write to FS " << std::endl;
					writeDDR2DFS(dr2D);
					break;
				case DataSourceType::DATA_SPACES:
					std::cout << "write to DS " << std::endl;
					writeDDR2DATASPACES(dr2D);
//					std::cout << "Data Spaces data source not implemented yet" << std::endl;
					break;
				default:
					std::cout << "Unknown data source type:" << dr->getOutputType() << std::endl;
					break;
			}
			break;
		case RegionTemplateType::DENSE_REGION_3D:
			std::cout << "Dense data region 3D instantiation to be implemented" << std::endl;
			break;
		default:
			std::cout << "Unknown data region type:"<< dr->getType() << std::endl;
			break;
	}
	return true;
}
