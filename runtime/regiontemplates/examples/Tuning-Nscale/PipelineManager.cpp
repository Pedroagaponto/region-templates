#include "SysEnv.h"
#include <regiontemplates/autotuning/TuningInterface.h>
#include <regiontemplates/autotuning/geneticalgorithm/GeneticAlgorithm.h>
#include <regiontemplates/autotuning/activeharmony/ActiveHarmonyTuning.h>
#include "RegionTemplateCollection.h"
#include "Segmentation.h"
#include "DiceMaskComp.h"
#include "DiceNotCoolMaskComp.h"
#include "ParameterSet.h"
#include "hclient.h"
#include "NormalizationComp.h"

#define INF    100000

void parseInputArguments(int argc, char **argv, std::string &inputFolder, std::string &AHpolicy,
                         std::string &initPercent);

RegionTemplateCollection *RTFromFiles(std::string inputFolderPath);


int main(int argc, char **argv) {

    int numClients;
    TuningInterface *tuningClient;
    int max_number_of_tests;

    //Multi-Objective Tuning weights
    double timeWeight = 1;
    double metricWeight = 1;

    //Multi-Objective Tuning normalization times
    double tSlowest = 50000; //Empirical Data
    double tFastest = 1000; //Empirical Data


    // Folder when input data images are stored
    std::string inputFolderPath, AHpolicy, initPercent;
    std::vector<RegionTemplate *> inputRegionTemplates;
    RegionTemplateCollection *rtCollection;
    parseInputArguments(argc, argv, inputFolderPath, AHpolicy, initPercent);


    //USING AH
    if (AHpolicy.find("nm") != std::string::npos || AHpolicy.find("NM") != std::string::npos ||
        AHpolicy.find("pro") != std::string::npos || AHpolicy.find("PRO") != std::string::npos) {
        max_number_of_tests = 20;
        numClients = 1;
        tuningClient = new ActiveHarmonyTuning(AHpolicy, max_number_of_tests, numClients);
    } else {

        //USING GA
        int max_number_of_generations = 5;
        int mutationchance = 30;
        int crossoverrate = 50;
        int propagationamount = 1;

        numClients = 4; //popsize
        max_number_of_tests = max_number_of_generations * numClients;
        tuningClient = new GeneticAlgorithm(max_number_of_generations, numClients, mutationchance,
                                            crossoverrate,
                                            propagationamount,
                                            1);

    }


    std::vector<int> segComponentIds[numClients];
    std::vector<int> diceComponentIds[numClients];
    std::vector<int> diceNotCoolComponentIds[numClients];
    std::map<std::string, double> perfDataBase; //Checks if a param has been tested already


    // Handler to the distributed execution system environment
    SysEnv sysEnv;

    // Tell the system which libraries should be used
    sysEnv.startupSystem(argc, argv, "libcomponenttuningnscale.so");

    // Create region templates description without instantiating data
    rtCollection = RTFromFiles(inputFolderPath);


    // Tuning Client SETUP //

    if (tuningClient->initialize(argc, argv) != 0) {
        fprintf(stderr, "Failed to initialize tuning session.\n");
        return -1;
    };


    if (tuningClient->declareParam("blue", 210, 240, 10) != 0 ||
        tuningClient->declareParam("green", 210, 240, 10) != 0 ||
        tuningClient->declareParam("red", 210, 240, 10) != 0 ||
        tuningClient->declareParam("T1", 2.5, 7.5, 0.5) != 0 ||
        tuningClient->declareParam("T2", 2.5, 7.5, 0.5) != 0 ||
        tuningClient->declareParam("G1", 5, 80, 5) != 0 ||
        tuningClient->declareParam("minSize", 2, 40, 2) != 0 ||
        tuningClient->declareParam("maxSize", 900, 1500, 50) != 0 ||
        tuningClient->declareParam("G2", 2, 40, 2) != 0 ||
        tuningClient->declareParam("minSizePl", 5, 80, 5) != 0 ||
        tuningClient->declareParam("minSizeSeg", 2, 40, 2) != 0 ||
        tuningClient->declareParam("maxSizeSeg", 900, 1500, 50) != 0 ||
        tuningClient->declareParam("fillHoles", 4, 8, 4) != 0 ||
        tuningClient->declareParam("recon", 4, 8, 4) != 0 ||
        tuningClient->declareParam("watershed", 4, 8, 4) != 0) {
        fprintf(stderr, "Failed to define tuning session\n");
        return -1;
    }

    if (tuningClient->configure() != 0) {
        fprintf(stderr, "Failed to initialize tuning session.\n");
        return -1;
    };

    double perf[numClients];
    float *totaldiffs = (float *) malloc(sizeof(float) * max_number_of_tests);
    float *dicePerIteration = (float *) malloc(sizeof(float) * max_number_of_tests);
    float *diceNotCoolPerIteration = (float *) malloc(sizeof(float) * max_number_of_tests);
    float maxdiff = 0;
    float minperf = std::numeric_limits<float>::infinity();


    uint64_t *totalexecutiontimes = (uint64_t *) malloc(sizeof(uint64_t) * max_number_of_tests);
    double *totalExecutionTimesNormalized = (double *) malloc(sizeof(double) * max_number_of_tests);

    int versionNorm = 0, versionSeg = 0;
    bool executedAlready[numClients];


    /* main loop */
    for (; !tuningClient->hasConverged();) {
        cout << "ITERATION: " << tuningClient->getIteration() << endl;


        //Get new param suggestions from the tuning client
        tuningClient->fetchParams();
        //Apply fitness function for each individual
        for (int i = 0; i < numClients; i++) {
            perf[i] = INF;

            std::ostringstream oss;
            oss << "PARAMS";
            typedef std::map<std::string, double *>::iterator it_type;
            for (it_type iterator = tuningClient->getParamSet(i)->paramSet.begin();
                 iterator != tuningClient->getParamSet(i)->paramSet.end(); iterator++) {
                //iterator.first key
                //iterator.second value
                oss << " - " << iterator->first << ": " << *(iterator->second);
            }

            // / if not found in performance database
            if (perfDataBase.find(oss.str()) != perfDataBase.end()) {
                perf[i] = perfDataBase.find(oss.str())->second;
                std::cout << "Parameters already tested: " << oss.str() << " perf: " << perf << std::endl;

                executedAlready[i] = true;
            } else {
                executedAlready[i] = false;
            }

        }


        int segCount = 0;
        // Build application dependency graph
        // Instantiate application dependency graph
        for (int i = 0; i < rtCollection->getNumRTs(); i++) {

            int previousSegCompId = 0;
            // CREATE NORMALIZATION STEP
            ParameterSet parSetNormalization;
            std::vector<ArgumentBase *> targetMeanOptions;
            ArgumentFloatArray *targetMeanAux = new ArgumentFloatArray(ArgumentFloat(-0.632356));
            targetMeanAux->addArgValue(ArgumentFloat(-0.0516004));
            targetMeanAux->addArgValue(ArgumentFloat(0.0376543));
            targetMeanOptions.push_back(targetMeanAux);
            parSetNormalization.addArguments(targetMeanOptions);
            parSetNormalization.resetIterator();
            std::vector<ArgumentBase *> argSetInstanceNorm = parSetNormalization.getNextArgumentSetInstance();
            NormalizationComp *norm = new NormalizationComp();
            // normalization parameters
            norm->addArgument(new ArgumentInt(versionNorm));
            norm->addArgument(argSetInstanceNorm[0]);
            norm->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
            sysEnv.executeComponent(norm);

            for (int j = 0; j < numClients; j++) {

                if (executedAlready[j] == false) {

                    std::cout << "BEGIN: LoopIdx: " << tuningClient->getIteration() * numClients + (j);

                    typedef std::map<std::string, double *>::iterator it_type;
                    for (it_type iterator = tuningClient->getParamSet(j)->paramSet.begin();
                         iterator != tuningClient->getParamSet(j)->paramSet.end(); iterator++) {
                        std::cout << " - " << iterator->first << ": " << *(iterator->second);
                    }

                    std::cout << std::endl;

                    // Creating segmentation component
                    Segmentation *seg = new Segmentation();

                    // version of the data region red. Each parameter instance in norm creates a output w/ different version
                    seg->addArgument(new ArgumentInt(versionNorm));
                    // version of the data region generated by the segmentation stage
                    seg->addArgument(new ArgumentInt(versionSeg));

                    // add remaining (application specific) parameters from the argSegInstance
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("blue", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("green", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("red", j))));
                    seg->addArgument(
                            new ArgumentFloat((float) (tuningClient->getParamValue("T1", j))));
                    seg->addArgument(
                            new ArgumentFloat((float) (tuningClient->getParamValue("T2", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("G1", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("G2", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("minSize", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("maxSize", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("minSizePl", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("minSizeSeg", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("maxSizeSeg", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("fillHoles", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("recon", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("watershed", j))));


                    // and region template instance that it is suppose to process
                    seg->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
                    seg->addDependency(norm->getId());

                    std::cout << "Creating DiffMask" << std::endl;
                    DiceMaskComp *diceComp = new DiceMaskComp();
                    DiceNotCoolMaskComp *diceNotCoolComp = new DiceNotCoolMaskComp();

                    // version of the data region that will be read. It is created during the segmentation.
                    diceComp->addArgument(new ArgumentInt(versionSeg));
                    diceNotCoolComp->addArgument(new ArgumentInt(versionSeg));

                    // region template name
                    diceComp->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
                    diceComp->addDependency(seg->getId());
                    diceNotCoolComp->addRegionTemplateInstance(rtCollection->getRT(i),
                                                               rtCollection->getRT(i)->getName());
                    diceNotCoolComp->addDependency(diceComp->getId());

                    // add to the list of diff component ids.
                    segComponentIds[j].push_back(seg->getId());
                    diceComponentIds[j].push_back(diceComp->getId());
                    diceNotCoolComponentIds[j].push_back(diceNotCoolComp->getId());

                    sysEnv.executeComponent(seg);
                    sysEnv.executeComponent(diceComp);
                    sysEnv.executeComponent(diceNotCoolComp);

                    std::cout << "Manager CompId: " << diceComp->getId() << " fileName: " <<
                    rtCollection->getRT(i)->getDataRegion(0)->getInputFileName() << std::endl;
                    segCount++;
                    versionSeg++;

                }
            }
            versionNorm++;
        }

        // End Creating Dependency Graph
        sysEnv.startupExecution();


        //==============================================================================================
        //Fetch results from execution workflow
        //==============================================================================================
        for (int j = 0; j < numClients; j++) {
            float dice = 0;
            float secondaryMetric = 0;
            float diceNotCoolValue = 0;

            std::ostringstream oss;
            oss << "PARAMS";
            typedef std::map<std::string, double *>::iterator it_type;
            for (it_type iterator = tuningClient->getParamSet(j)->paramSet.begin();
                 iterator != tuningClient->getParamSet(j)->paramSet.end(); iterator++) {
                oss << " - " << iterator->first << ": " << *(iterator->second);
            }

            if (executedAlready[j] == false) {
                for (int i = 0; i < diceComponentIds[j].size(); i++) {
                    char *diceResultData = sysEnv.getComponentResultData(diceComponentIds[j][i]);
                    char *diceNotCoolResultData = sysEnv.getComponentResultData(diceNotCoolComponentIds[j][i]);
                    std::cout << "Diff Id: " << diceComponentIds[j][i] << " \tdiceResultData: ";
                    if (diceResultData != NULL) {
                        std::cout << "size: " << ((int *) diceResultData)[0] << " \thadoopgis-metric: " <<
                        ((float *) diceResultData)[1] <<
                        " \tsecondary: " << ((float *) diceResultData)[2] << " \tdiceNotCool: " <<
                        ((float *) diceNotCoolResultData)[1] << std::endl;
                        dice += ((float *) diceResultData)[1];
                        secondaryMetric += ((float *) diceResultData)[2];
                        diceNotCoolValue += ((float *) diceNotCoolResultData)[1];
                    } else {
                        std::cout << "NULL" << std::endl;
                    }
                    char *segExecutionTime = sysEnv.getComponentResultData(segComponentIds[j][i]);
                    if (segExecutionTime != NULL) {
                        totalexecutiontimes[tuningClient->getIteration() * numClients +
                                            (j)] = ((int *) segExecutionTime)[1];
                        cout << "Segmentation execution time:" <<
                        totalexecutiontimes[tuningClient->getIteration() * numClients + (j)] << endl;
                    }
                    sysEnv.eraseResultData(diceNotCoolComponentIds[j][i]);
                    sysEnv.eraseResultData(diceComponentIds[j][i]);
                    sysEnv.eraseResultData(segComponentIds[j][i]);
                }
                diceComponentIds[j].clear();
                segComponentIds[j].clear();
                diceNotCoolComponentIds[j].clear();


                //########################################################################################
                //Single Objective Tuning
                //########################################################################################
                // perf[j] = (double) 1 / diff; //If using Hadoopgis
                // perf[j] = diff; //If using PixelCompare.

                //########################################################################################
                //Multi Objective Tuning
                //########################################################################################

                float diff = (dice + diceNotCoolValue) / 2;
                if (diff <= 0) diff = FLT_EPSILON;

                double timeNormalized =
                        (tSlowest - (double) totalexecutiontimes[tuningClient->getIteration() * numClients + (j)]) /
                        (tSlowest - tFastest);

                perf[j] = (double) 1 /
                          (double) (metricWeight * diff + timeWeight * timeNormalized); //Multi Objective Tuning

                totalExecutionTimesNormalized[tuningClient->getIteration() * numClients + (j)] = timeNormalized;
                if (perf[j] < 0) perf[j] = 0;

                std::cout << "END: LoopIdx: " << tuningClient->getIteration() * numClients + (j);
                typedef std::map<std::string, double *>::iterator it_type;
                for (it_type iterator = tuningClient->getParamSet(j)->paramSet.begin();
                     iterator != tuningClient->getParamSet(j)->paramSet.end(); iterator++) {
                    //iterator.first key
                    //iterator.second value
                    std::cout << " - " << iterator->first << ": " << *(iterator->second);
                }

                cout << endl << endl << "\tDiff: " << diff << " Secondary Metric: " << secondaryMetric <<
                " Time Normalized: " << timeNormalized << " Segmentation Time: " <<
                totalexecutiontimes[tuningClient->getIteration() * numClients + (j)] << " Perf: " << perf[j] << endl;

                totaldiffs[tuningClient->getIteration() * numClients + (j)] = diff;
                dicePerIteration[tuningClient->getIteration() * numClients + (j)] = dice;
                diceNotCoolPerIteration[tuningClient->getIteration() * numClients + (j)] = diceNotCoolValue;

                if (minperf > perf[j] && perf[j] != 0) {
                    minperf = perf[j];
                    maxdiff = diff;
                }

                perfDataBase[oss.str()] = perf[j];
            } else {
                perf[j] = perfDataBase[oss.str()];

                std::cout << "ATTENTION! Param set executed already:" << std::endl;
                std::cout << "END: LoopIdx: " << tuningClient->getIteration() * numClients + (j);
                std::cout << oss.str() << endl;

                std::cout << " perf: " << perf[j] << std::endl;
            }

            // Report the performance we've just measured.
            tuningClient->reportScore(perf[j], j);
        }

        //Checks if at least one test in this iteration succeeded.
        bool shouldIterate = false;
        for (int k = 0; k < numClients; ++k) {
            shouldIterate |= !executedAlready[k];
        }
        //Iterates the tuning algorithm
        if (shouldIterate == true) tuningClient->nextIteration();

    }

    std::cout << "\t\tResults:" << std::endl;
    for (int i = 0; i < max_number_of_tests; ++i) {
        double perfWeighted =
                (double) 1 / (double) (metricWeight * totaldiffs[i] + timeWeight * totalExecutionTimesNormalized[i]);
        if (perfWeighted < 0) perfWeighted = 0;

        std::cout << std::fixed << std::setprecision(6) << "\t\tTest: " << i << " \tDiff: " << totaldiffs[i] <<
        "  \tExecution Time: " <<
        totalexecutiontimes[i] << " \tTime Normalized: " << totalExecutionTimesNormalized[i] << " \tDice: " <<
        dicePerIteration[i] << " \tDiceNC: " << diceNotCoolPerIteration[i];
        std::cout << "  \tPerf(weighted): " << perfWeighted << std::endl;
    }
    std::cout << "\tBest Diff: " << maxdiff << std::endl;
    std::cout << "\tBest answer for MultiObjective Tuning has MinPerfWeighted: " << minperf << std::endl;



    // Finalize all processes running and end execution
    sysEnv.finalizeSystem();

    delete rtCollection;

    return 0;
}


namespace patch {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
}

void parseInputArguments(int argc, char **argv, std::string &inputFolder, std::string &AHpolicy,
                         std::string &initPercent) {
    // Used for parameters parsing
    for (int i = 0; i < argc - 1; i++) {
        if (argv[i][0] == '-' && argv[i][1] == 'i') {
            inputFolder = argv[i + 1];
        }
        if (argv[i][0] == '-' && argv[i][1] == 'o') {
            initPercent = argv[i + 1];
        }
        if (argv[i][0] == '-' && argv[i][1] == 'f') {
            AHpolicy = argv[i + 1];
        }
    }
}


RegionTemplateCollection *RTFromFiles(std::string inputFolderPath) {
    // Search for input files in folder path
    std::string referenceMaskExtension = "_mask.txt"; //In case of labeled masks in text format;
    //std::string referenceMaskExtension = ".mask.png"; //In case of binary mask in png format;


    FileUtils fileUtils(referenceMaskExtension);
    std::vector<std::string> fileList;
    fileUtils.traverseDirectoryRecursive(inputFolderPath, fileList);
    RegionTemplateCollection *rtCollection = new RegionTemplateCollection();
    rtCollection->setName("inputimage");

    std::cout << "Input Folder: " << inputFolderPath << std::endl;

    // Create one region template instance for each input data file
    // (creates representations without instantiating them)
    for (int i = 0; i < fileList.size(); i++) {

        // Create input mask data region
        DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
        ddr2d->setName("RAW");
        std::ostringstream oss;
        oss << i;
        ddr2d->setId(oss.str());
        ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
        ddr2d->setIsAppInput(true);
        ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
        std::string inputFileName = fileUtils.replaceExt(fileList[i], referenceMaskExtension, ".tiff");
        ddr2d->setInputFileName(inputFileName);

        // Create reference mask data region
        DenseDataRegion2D *ddr2dRefMask = new DenseDataRegion2D();
        ddr2dRefMask->setName("REF_MASK");
        ddr2dRefMask->setId(oss.str());
        ddr2dRefMask->setInputType(DataSourceType::FILE_SYSTEM_TEXT_FILE);
        ddr2dRefMask->setIsAppInput(true);
        ddr2dRefMask->setOutputType(DataSourceType::FILE_SYSTEM_TEXT_FILE);
        cout << endl << "MASK FILE: " << fileList[i] << endl;
        ddr2dRefMask->setInputFileName(fileList[i]);

        // Adding data regions to region template
        RegionTemplate *rt = new RegionTemplate();
        rt->setName("tile");
        rt->insertDataRegion(ddr2d);
        rt->insertDataRegion(ddr2dRefMask);

        // Adding region template instance to collection
        rtCollection->addRT(rt);
    }

    return rtCollection;
}
