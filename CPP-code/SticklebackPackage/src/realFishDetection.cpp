/*
\file realfishdetection.cpp
main file for detecting stickleback fish 
\author: Reza Ardekani
\date: last update: 7/10/2013
*/

#include <time.h>
#include "MultipleBackgroundsModeller.h"
#ifdef WIN32
#include "windows.h"
#include "direct.h"
#endif
#define __SHOW 0
#define __DEBUG 1
//#define __LOAD_AVI_TO_MEM
#define __SAVE_CHANGE_MASKS_TO_THE_FILE 1

int main(int argc, char** argv)
{
    int firstFrameToProcess, lastFrameToProcess, myWindowSize, myMasterPeriod;
    std::string inputVideoFileName, inputMaskFileName, featuresFileName,  thePathToFrameFiles;
    std::string listOfFiles; //list of video files

    std::string baseDIR = "D:\\Peichel\\PJxP_Family1\\school_only\\";
    std::string outputSavePath = "";

#if __DEBUG
        //inputVideoFileName = "D:\\Peichel\\PJxP_Family1\\school_only\\1.2_PG-clip-2011-05-04 13;54;28.avi";

//        inputVideoFileName = "D:\\Peichel\\PJxP_Family1\\school_only\\1.1_GG-clip-2011-06-08 14;44;12.avi";
//        inputVideoFileName = "D:\\Peichel\\PJxP_Family1\\school_only\\1.1_RO-clip-2011-06-08 15;44;59.avi";

        inputVideoFileName = "D:\\Peichel\\PJxP_Family1\\school_only\\1.4_RP-clip-2011-05-06 14;01;36.avi";


        int startFrame = 58;
        int stopFrame = 9058; 

#else


    if (argc<2)
    {
        inputVideoFileName = "";
        std::cout<<"no input file list"<<std::endl;
        //return (-1);
        //listOfFiles="D:\\Peichel\\PJxP_Family1\\videoinfo4of4_fullPath_schoolOnly1_2_test.csv";
        listOfFiles="C:\\Peichel\\PJxP_Family1\\videoinfo4of4_fullPath_schoolOnly_sec1_4.csv";
    }
    else
    {
        listOfFiles = argv[1];
    }
    printf("\nLoading list of the videos from %s",listOfFiles.c_str());
    std::ifstream myInputFile(listOfFiles.c_str());
    std::stringstream ss;
    char buff[2000];
    if(!myInputFile.is_open())
    {
        std::cout<<"\nproblem with the file: "<<listOfFiles<<std::endl;

    }

    int numberOfFiles = 0;
    myInputFile.getline(buff,2000); //to skip the header line
    while(!(myInputFile.eof()))
    {

        myInputFile.getline(buff,2000);
        int startFrame,stopFrame,matchFrame;
        char fileName[500];
        ss<<buff;
        for (int i = 0;i<4;i++)
        {
            ss.getline(buff,500,',');
            std::cout<<"i = " <<i<<" , "<<buff<<std::endl;
            ss<<"";
            ss.clear(); //important! to turn the eof() flagg off
            if (i==0)
                inputVideoFileName = buff;
            if (i==1)
                startFrame = atoi(buff);

            if (i==2)
                stopFrame = atoi(buff);

            if (i==3)
                matchFrame = atoi(buff);

        }

#endif
				//change the extension from .mp4 to .avi
        inputVideoFileName = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
#if __SAVE_CHANGE_MASKS_TO_THE_FILE
        std::string folderName = inputVideoFileName.substr(baseDIR.length(),inputVideoFileName.length()); // this is the first part of the file name
        folderName+="_changeMasks";
#ifdef WIN32
        chdir(baseDIR.c_str());
        CreateDirectoryA (folderName.c_str(), NULL);
        outputSavePath = baseDIR + folderName +  "\\";
#endif

#endif

        inputVideoFileName+=".avi";
        inputMaskFileName = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
        inputMaskFileName+= "_Mask.bmp";

        int myFirstFrame = startFrame; // first frame in the file that we want to include in similar frame finding
        int myLastFrame = stopFrame; // last frame in the file that we want to include in similar frame finding

        thePathToFrameFiles = "";
//#ifndef __LOAD_AVI_TO_MEM
        featuresFileName = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
        featuresFileName += "_allFeatures_20.csv";
//#endif

        //featuresFileName = "";
        MultipleBackgroundsModeller myModeller;

        std::string featStatus, readStatus;
        if (featuresFileName == "")
            featStatus = "withoutFeat";
        else
            featStatus = "WithFeat";

        if (thePathToFrameFiles == "")
            readStatus = "fromVideo";
        else
            readStatus = "fromFile";

        myWindowSize = 20;
        myMasterPeriod = 360;

        std::ostringstream stringStream;
        stringStream << "_ff"<<myFirstFrame<<"_lf"<<myLastFrame<<"_ws"<<myWindowSize<<"_mp"<<myMasterPeriod<<"_"<<featStatus<<"_"<<readStatus;
        std::string appendixFileName= stringStream.str();


        if (!myModeller.initialize(inputVideoFileName,inputMaskFileName,myWindowSize,myMasterPeriod,myFirstFrame,myLastFrame,featuresFileName,3000,outputSavePath))
        {
            printf("\nCan not initialize the processor for this video: %s",inputVideoFileName.c_str());
            getchar();
            //return -1;
#if !__DEBUG
            continue; // the main loop over the videos
#endif
        }

        printf("\n start processing \n");
        clock_t startTime = clock();
        clock_t eachFrameStartTime;

        //for (int fn =30;fn<9030;fn++)
        for (int fn =myFirstFrame;fn<=myLastFrame;fn++)
        {
            //IplImage* test = readFrame(fn,myModeller.videoCapture);
            //cvShowImage("test",test);


            eachFrameStartTime = clock();
            if(!myModeller.processFrameNumber(fn))
                printf("\nFailed");

//#if __SAVE_CHANGE_MASKS_TO_THE_FILE
//            myModeller.saveChangeMasksToThefile(outputSavePath);
//#endif

            if (fn % 100 == 0)
            {
                printf("\nframe %d of %s",fn,inputVideoFileName.c_str());            
                printf("\ntime Taken is %.5lf miliseconds", double(1000*(clock()-eachFrameStartTime)/CLOCKS_PER_SEC));
            }

            

#if __SHOW
        cvShowImage("final change mask",myModeller.currentFrameImgChangeMask);
        cvWaitKey(1);
#endif
        }

        myModeller.writeAllExtractedInfoTotheFile();
        myModeller.finish();
        printf("\nDone!");
        printf("\ntime Taken is %.2lf seconds", double(clock()-startTime)/CLOCKS_PER_SEC);
//        getchar();
#if !__DEBUG
    }
#endif
    //getchar();
    return 0;
}


