#pragma once
#include "MultipleBackgroundsModeller.h"
#define __SHOW_OUTPUT 1;
#define __JUST_KEEP_LARGEST 0 //TODO: COMBINE THE EXTRACTINFO AND WRITE OUTPUT FUNCTIONS...
#define __PROFILE_IT 0
#define __NUM_OF_BEST_FRAMES 8
#define __REMOVE_CLOSE_FRAMES_FROM_BESTFRAME_LIST 0
#define __WRITE 0


#define __WRITE_VIDEO_OUTPUT 0


/// below defs are for revised code, some of the above definitions are useless now
#define __VERBOSE 1
#define __SHOW_CHANGEMASKS 0
#define __NUMBER_OF_TEMP_CHANGE_MASK_TO_USE 4


MultipleBackgroundsModeller::MultipleBackgroundsModeller(void)
{
    m_videoCapture = NULL;
    m_video_Out = NULL;
    m_video_Out_changeMask = NULL;
    m_imMask = NULL;
    m_currentFrameImg = NULL;
    m_currentFrameImgChangeMask = NULL;
    m_startFrameToProcessInThisChunk = 0;
    m_lastFrameToProcessInThisChunk = -1;
    m_isInit = false;
    m_allFramesFeatures.clear();
    m_candidateFramesDistanceList.clear();
}

MultipleBackgroundsModeller::~MultipleBackgroundsModeller(void)
{
}


bool MultipleBackgroundsModeller::loadRawAVIFramesToMemory(int startFrame, int stopFrame)
{
    if (m_videoCapture == NULL)
        return (false);

    //clean up if needed
    if (m_rawAVIFrameBank.size()>1)
    {
        for (size_t i = 0;i<m_rawAVIFrameBank.size();i++)
        {
            if (m_rawAVIFrameBank[i] != NULL)
            {
                cvReleaseImage(&m_rawAVIFrameBank[i]);
                m_rawAVIFrameBank[i] = NULL;
            }
        }
        m_rawAVIFrameBank.clear();
    }

    cvSetCaptureProperty(m_videoCapture,CV_CAP_PROP_POS_FRAMES,double(startFrame));
    IplImage* currFrame = NULL;
    IplImage* frameImg = NULL;
    for (int i = startFrame;i<stopFrame;i++)
    {
        currFrame = cvQueryFrame(m_videoCapture);
        frameImg = cvCloneImage(currFrame);
        m_rawAVIFrameBank.push_back(frameImg);
        //printf("\r%.2lf is loaded",(i*1.0-startFrame)/(stopFrame-startFrame));
		//cvShowImage("frame",frameImg);
		//cvWaitKey(1);
    }
    return (true);
}
bool MultipleBackgroundsModeller::loadAllFeaturesFromFile(std::string fn)
{
    std::ifstream file;
    file.open(fn.c_str());
    if(!file.is_open())
    {
        std::cout<<"can not load features from the file\n";
        return false;
    }

    std::string line;
    std::vector<std::string> items;
    std::vector<int>tempVec;
    while ( file.good() )
    {
        getline ( file, line); // read a string until next endline: http://www.cplusplus.com/reference/string/getline/
        items.clear();
        tempVec.clear();
        Tokenize(line,items,",");
        if (items.size() < 10) // if a line is not complete
            continue;

        for (size_t ii = 1 ;ii< items.size();ii++) // include first element since it is the frame number
            tempVec.push_back(atoi(items[ii].c_str()));
        m_allFramesFeatures[atoi(items[0].c_str())] = tempVec;
    }
    file.close();

return true;
}

bool MultipleBackgroundsModeller::buildCandidateFramesListForCurrentFrame(void)
{
#if __VERBOSE
    printf("\n sart of buildCandidateFramesListForCurrentFrame");
#endif
    m_candidateFramesDistanceList.clear();
    candidateFrame tmp;
    std::vector<int>v1,v2;
    v1 = m_allFramesFeatures[m_currFrameNum];

    if (m_useShortListOfFrames)
    {
        findShortListOfFrames();// build m_shortListOfPotentialGoodFrames
        
        for (size_t i = 0;i<m_shortListOfPotentialGoodFrames.size();i++)
        {
            int fn = m_shortListOfPotentialGoodFrames[i];

            v2 = m_allFramesFeatures[fn];
            int dist = returnAbsDistanceOfVecs(v1,v2);
            tmp.frameNum = fn;
            tmp.distance = double(dist/10000.0);
            m_candidateFramesDistanceList.push_back(tmp);

        }
    }
    else // use all other frames -- the m_candidateFramesDistanceList will be long as m_lastFrameToProcessInThisChunk - m_startFrameToProcessInThisChunk
    {
        for (int fn = m_startFrameToProcessInThisChunk;fn<m_lastFrameToProcessInThisChunk;fn++)
        {
            v2 = m_allFramesFeatures[fn];
            int dist = returnAbsDistanceOfVecs(v1,v2);
            tmp.frameNum = fn;
            tmp.distance = double(dist/10000.0);
            m_candidateFramesDistanceList.push_back(tmp);
        }
    }
    return true;
}

//an auxulary function
std::vector<int> MultipleBackgroundsModeller::returnBestFrames()
{
    //first get rid of high distance frames
    for (int i = 0;i<m_candidateFramesDistanceList.size();i++)
    {
        if(m_candidateFramesDistanceList[i].distance>60.0)
            m_candidateFramesDistanceList[i].distance = 60.0;
    }
    //now find local minimums
    std::vector<int> toRet;

    bool isABorderlineFrame = false; // this is needed if we are going to use a compact list of canndidates using materperiod and window size
    for (int i = 1;i<m_candidateFramesDistanceList.size()-1;i++)
    {
        isABorderlineFrame = (abs(m_candidateFramesDistanceList[i].frameNum - m_candidateFramesDistanceList[i-1].frameNum)>10) || (abs(m_candidateFramesDistanceList[i].frameNum - m_candidateFramesDistanceList[i+1].frameNum)>10) ;

        if (!isABorderlineFrame)
            if ( (m_candidateFramesDistanceList[i].distance<30.0) && (m_candidateFramesDistanceList[i].distance<m_candidateFramesDistanceList[i-1].distance) && (m_candidateFramesDistanceList[i].distance<m_candidateFramesDistanceList[i+1].distance) )
                toRet.push_back(m_candidateFramesDistanceList[i].frameNum);
    }
    return(toRet);

}


bool MultipleBackgroundsModeller::initialize(std::string inputVideoFileName, std::string inputMaskFileName, int wSize, int mPeriod, int startFrame , int lastFrame, std::string featuresFileName, int numOfFramesToLoadInMemory, std::string outputPathForChangeMasks)
{
    printf("\nInitializing..");
    m_changeMaskOutputSavePath = outputPathForChangeMasks; // set the output path

    m_useShortListOfFrames = false; //do not use the short list of the frames.. search between whole m_chunkSize frames... its safer and gives better rsult

    m_videoCapture = cvCreateFileCapture(inputVideoFileName.c_str());
    if (m_videoCapture == NULL)
    {
        std::cout<<"can not open the video file\n";
        return false;
    }
    std::string appendx = "_raw";
    std::string videoOutfn,videoOutChangeMaskfn,infoOutfn,candidateOutfn;
    videoOutfn = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
    infoOutfn = videoOutfn + "_allFramesInfo" + appendx+ ".txt";
    candidateOutfn = videoOutfn + "_allFramesInfoSorted" + appendx+ ".txt";
    videoOutChangeMaskfn = videoOutfn + "_changeMask" + appendx + ".avi";
    videoOutfn+="_fishDetected_output" + appendx + ".avi";
    m_outputInfoStream.open(infoOutfn.c_str());
    m_outputAllFramesInfoSorted.open(candidateOutfn.c_str());

    if (m_videoCapture == NULL)
    {
        printf("\nCan not open the video");
        return false;
    }

    CvSize mySize = cvSize((int)cvGetCaptureProperty(m_videoCapture,CV_CAP_PROP_FRAME_WIDTH),(int)cvGetCaptureProperty(m_videoCapture,CV_CAP_PROP_FRAME_HEIGHT));

#if __WRITE_VIDEO_OUTPUT
    //	video_Out = cvCreateAVIWriter(videoOutfn.c_str(), CV_FOURCC('F','F','D','S') ,30.0,mySize,true);
    //	video_Out_changeMask = cvCreateAVIWriter(videoOutChangeMaskfn.c_str(),  CV_FOURCC('D','I','V','X') ,30.0,mySize,true);
    video_Out = cvCreateAVIWriter(videoOutfn.c_str(), CV_FOURCC_DEFAULT ,30.0,mySize,true);
    video_Out_changeMask = cvCreateAVIWriter(videoOutChangeMaskfn.c_str(),  CV_FOURCC_DEFAULT ,30.0,mySize,true);
#endif
    m_imMask = NULL;
    m_imMask = cvLoadImage(inputMaskFileName.c_str(),0); // read at as gray-Scale
    if(m_imMask==NULL)
    {
        printf("\nCan not load the mask file %s",inputMaskFileName.c_str());
        return(false);
    }
    cvThreshold(m_imMask,m_imMask, 128, 255,CV_THRESH_BINARY);
    if (m_imMask == NULL)
    {
        printf("\nCan not open the mask file");
        return false;
    }

    if(!loadAllFeaturesFromFile(featuresFileName))
    {
        printf("\nCan not open the feature file");     
        return false;
    }


    m_chunkSize = numOfFramesToLoadInMemory;


    int numFramesInFile = (unsigned long int)cvGetCaptureProperty(m_videoCapture,CV_CAP_PROP_FRAME_COUNT);
    if (lastFrame>numFramesInFile)
    {
        printf("\nwarning - wrong last frame setting");
//        m_lastFrameToProcessInThisChunk = numFramesInFile-1;

        m_globalLastFrameToProcess = numFramesInFile-1;
    }
    else
    {
         m_globalLastFrameToProcess = numFramesInFile-1;
    }

    if (m_globalLastFrameToProcess>startFrame + m_chunkSize)
    {
        m_lastFrameToProcessInThisChunk = startFrame + m_chunkSize;

    }



    if(startFrame < 0 || startFrame > numFramesInFile)
    {
        printf("\nwarning - wrong first frame setting");
        m_startFrameToProcessInThisChunk = 0;
        m_globalFirstFrameToProcess = 0;
    }
    else
    {
        m_startFrameToProcessInThisChunk = startFrame; //TODO : check the conditions
        m_globalFirstFrameToProcess = startFrame;
    }


    m_masterPeriod = mPeriod;
    m_windowSize = wSize;

    m_isInit = true;
#ifdef __LOAD_AVI_TO_MEM

    std::cout<<" m_startFrameToProcessInThisChunk, m_lastFrameToProcessInThisChunk  = "<< m_startFrameToProcessInThisChunk << " - - -" << m_lastFrameToProcessInThisChunk<<std::endl;
    m_loadedAVIFramesRange.first  = m_startFrameToProcessInThisChunk;
    m_loadedAVIFramesRange.second = m_lastFrameToProcessInThisChunk;
    loadRawAVIFramesToMemory(m_loadedAVIFramesRange.first, m_loadedAVIFramesRange.second);
#endif

// set all frames as non-processed
for (int i = 0;i<30000;i++)
    m_isFrameProcessed[i]=0;


    printf("\ninitialized"); 
    return true;
}



bool MultipleBackgroundsModeller::findShortListOfFrames(void)
{
    m_shortListOfPotentialGoodFrames.clear();
    if (m_currFrameNum<m_startFrameToProcessInThisChunk || m_currFrameNum>m_lastFrameToProcessInThisChunk || m_startFrameToProcessInThisChunk>m_lastFrameToProcessInThisChunk)
    {
        printf("\nInvalid current frame number or start frame or last frame");
        return false;
    }

    int TotalNumPeriods = (int) (m_lastFrameToProcessInThisChunk-m_startFrameToProcessInThisChunk)/m_masterPeriod; // total number of repeats of each chunk
    int whichPer = (int)floor((m_currFrameNum/m_masterPeriod)*1.0);
    int tempframeNum;

    for (int i = 1;i<whichPer+1;i++) //candidate frames before the current frame, if (whichFrame ==0) this loop doesn't run
        for (int j = -m_windowSize;j<=m_windowSize;j++)
        {
            tempframeNum = m_currFrameNum - i*m_masterPeriod + j;
            if (tempframeNum == m_currFrameNum) // don't include the frame itself but include its neighbors.
                    continue;
            if ((tempframeNum < m_startFrameToProcessInThisChunk) || (tempframeNum>m_lastFrameToProcessInThisChunk) ) // to avoid negative frame numbers
                continue;
            else
                m_shortListOfPotentialGoodFrames.push_back(tempframeNum);
        }

    for (int i = 0;i<TotalNumPeriods-whichPer+1;i++) //candidate frames after the current frame
        for (int j = -m_windowSize;j<=m_windowSize;j++)
        {
            tempframeNum = m_currFrameNum + i*m_masterPeriod + j;
            if (tempframeNum == m_currFrameNum) // don't include the frame itself but include its neighbors.
                continue;
            if ((tempframeNum < m_startFrameToProcessInThisChunk) || (tempframeNum>m_lastFrameToProcessInThisChunk) ) // to avoid negative frame numbers
                continue;
            else
                m_shortListOfPotentialGoodFrames.push_back(tempframeNum);
        }
    return true;
}

bool MultipleBackgroundsModeller::findCurrentClusterFrameNumbers(void)
{
    m_currentCluster_frameNumbers.clear();

    clock_t starttime = clock();
    if(!buildCandidateFramesListForCurrentFrame())
    {
        printf("\ncan not  buildCandidateFramesListForCurrentFrame");
        return(false);
    };
#if __VERBOSE       
    printf("\ntime Taken for buildCandidateFramesListForCurrentFrame is %.5lf miliseconds", double(1000*(clock()-starttime)/CLOCKS_PER_SEC));
#endif

    std::vector<int> best24 = returnBestFrames();
    if(best24.size()<1)
    {
        printf("\nreturnBestFrames did not work");
        return(false);
    }
//    for (int ii = 0;ii<__NUM_OF_BEST_FRAMES;ii++)
  //  int numberOfBestFramesToUse;
  //best24.size() >  __NUM_OF_BEST_FRAMES ? numberOfBestFramesToUse = best24.size() : numberOfBestFramesToUse = __NUM_OF_BEST_FRAMES;
  //
  m_currentCluster_frameNumbers.push_back(m_currFrameNum);
  for (size_t ii = 0;ii<best24.size();ii++)//numberOfBestFramesToUse;ii++)
        m_currentCluster_frameNumbers.push_back(best24[ii]);
    
	return true;
}


bool MultipleBackgroundsModeller::finish(void)
{
    printf("\n finishing up...");
    if(m_isInit)
    {
        cvReleaseCapture(&m_videoCapture);
        cvReleaseVideoWriter(&m_video_Out);
        cvReleaseVideoWriter(&m_video_Out_changeMask);
        cvReleaseImage(&m_imMask);
        cvReleaseImage(&m_currentFrameImg);
        cvReleaseImage(&m_currentFrameImg);
    }

    for (size_t i = 0;i<m_rawAVIFrameBank.size();i++)
        if(m_rawAVIFrameBank[i]!=NULL)
        {
            cvReleaseImage(&m_rawAVIFrameBank[i]);
            m_rawAVIFrameBank[i] = NULL;
        }
    m_rawAVIFrameBank.clear();

    m_outputInfoStream.close(); // close the stream file
    m_outputAllFramesInfoSorted.close();
    return true;
}

bool MultipleBackgroundsModeller::unloadRawAVIFramesFromMemory(void)
{
#if __VERBOSE
    printf("\n\n-----unloadRawAVIFramesFromMemory ----");
#endif
    for (size_t i = 0;i<m_rawAVIFrameBank.size();i++)
        if(m_rawAVIFrameBank[i]!=NULL)
        {
            cvReleaseImage(&m_rawAVIFrameBank[i]);
            m_rawAVIFrameBank[i] = NULL;
        }
    m_rawAVIFrameBank.clear();
    return true;
}


bool MultipleBackgroundsModeller::processFrameNumber(int frameNum)
{
    if(m_isFrameProcessed[frameNum] == 1)
    {
#if __VERBOSE
        std::cout<<"\ndid not process  "<<frameNum;
#endif
        return(true); // this frame has already been processed
    }

    /// we are done with one chunk (usually 3000 frames)
    if(frameNum > m_lastFrameToProcessInThisChunk)
    {
#if __VERBOSE
        printf("\ntime to load new chunk.....");
#endif

        unloadRawAVIFramesFromMemory(); // first unload the avi;
        m_startFrameToProcessInThisChunk = m_lastFrameToProcessInThisChunk;
        m_lastFrameToProcessInThisChunk = m_startFrameToProcessInThisChunk + m_chunkSize;
        if (m_lastFrameToProcessInThisChunk > m_globalLastFrameToProcess)
            m_lastFrameToProcessInThisChunk = m_globalFirstFrameToProcess;

        m_loadedAVIFramesRange.first  = m_startFrameToProcessInThisChunk;
        m_loadedAVIFramesRange.second = m_lastFrameToProcessInThisChunk;
        loadRawAVIFramesToMemory(m_loadedAVIFramesRange.first, m_loadedAVIFramesRange.second);
    }




    m_currFrameNum = frameNum; // clean-up/update info for current frame

#if __VERBOSE
    printf("\nfindCurrentClusterFrameNumbers()");
#endif

    clock_t startTime = clock();
    if(!findCurrentClusterFrameNumbers())
    {
        printf("\ncan not findCurrentClusterFrameNumbers");
        return false;
    }
#if __VERBOS
    printf("\ntime Taken for finding Clusters is %.5lf miliseconds", double(1000*(clock()-startTime)/CLOCKS_PER_SEC));
    // now findBestFrames has the list of current clustered frames;
#endif
    for (size_t i = 0; i<m_currentClusterOfFramesImg.size();i++)
        if(m_currentClusterOfFramesImg[i]!=NULL)
            cvReleaseImage(&m_currentClusterOfFramesImg[i]);
    m_currentClusterOfFramesImg.clear();
    m_currentClusterOfFramesImg.resize(m_currentCluster_frameNumbers.size());

    //ReadFrames
#if __VERBOSE
    printf("\nreadFrames");
#endif
    for (size_t i = 0; i < m_currentCluster_frameNumbers.size(); i++)
    {
        m_currentClusterOfFramesImg[i] = readFrame(m_currentCluster_frameNumbers[i]);
    }
#if __VERBOSE
        printf("\nclean up previous change masks");
#endif
    //clean up the previous change masks - if it is there
    for (size_t i = 0; i<m_currentClusterOfChangeMaskImg.size();i++)
        if(m_currentClusterOfChangeMaskImg[i]!=NULL)
            cvReleaseImage(&m_currentClusterOfChangeMaskImg[i]);
    m_currentClusterOfChangeMaskImg.clear();
    m_currentClusterOfChangeMaskImg.resize(m_currentCluster_frameNumbers.size());

#if __VERBOSE
     printf("\ncalculateChangeMasks");
#endif

    calculateChangeMasks();

#if __VERBOSE
     printf("\nwriting ChangeMasks to the file");
#endif

    if(m_changeMaskOutputSavePath!="")
        saveChangeMasksToThefile(m_changeMaskOutputSavePath);


#if __WRITE_VIDEO_OUTPUT
	if (!writeOutputVideo())
		return false;
#endif 

    //if (!extractAndWriteInfoToFile())
    //	return false;
#if __VERBOSE
     printf("\nextractInfoForCurrentCluster");
#endif

    extractInfoForCurrentCluster();


// mark frames in the current cluster as processed
//// THIS VALUES m_currentCluster_frameNumbers   ARE NOT CORRECT .. CHECK IT..

    for (size_t i = 0;i<m_currentCluster_frameNumbers.size();i++)
        m_isFrameProcessed[m_currentCluster_frameNumbers[i]] = 1; 

//    m_currentCluster_frameNumbers.clear();
    return true;

}

bool MultipleBackgroundsModeller::calculateChangeMasks() //(int clusterIndx)
{
    //just in case, do the clean up
    for (size_t i = 0; i<m_temp_changeMasks.size();i++)
        if(m_temp_changeMasks[i]!=NULL)
            cvReleaseImage(&m_temp_changeMasks[i]);
    m_temp_changeMasks.clear();


    for (size_t i = 0;i<m_currentClusterOfFramesImg.size();i++)
    {
        if(m_isFrameProcessed[m_currentCluster_frameNumbers[i]] == 1) //skip this if the frame is already processed
            continue;

        std::vector<int>pickedupIndicesForTempChangeMasks; // we do not need to use all of the other frames in the cluster, 3 is enough
        for(size_t j = 0; j<m_currentClusterOfFramesImg.size() && (pickedupIndicesForTempChangeMasks.size() < __NUMBER_OF_TEMP_CHANGE_MASK_TO_USE);j++)
        {
            if ( (i!=j) && (abs(m_currentCluster_frameNumbers[i]- m_currentCluster_frameNumbers[j]) > (   2 /*( (int)(floor(m_chunkSize/900.0)) - 1)*/  *m_masterPeriod + 50))) // do not calculate the change mask between a frame and itself and do not use close frames..
            {
                pickedupIndicesForTempChangeMasks.push_back(j);
            }
        }

        for(size_t j = 0; j<pickedupIndicesForTempChangeMasks.size();j++)
        {
 //           if ( (i!=j) && (abs(m_currentCluster_frameNumbers[i]- m_currentCluster_frameNumbers[j]) > (   3 /*( (int)(floor(m_chunkSize/900.0)) - 1)*/  *m_masterPeriod + 50))) // do not calculate the change mask between a frame and itself and do not use close frames..
            {
                m_temp_changeMasks.push_back(subtractTwoImages(m_currentClusterOfFramesImg[pickedupIndicesForTempChangeMasks[j]],m_currentClusterOfFramesImg[i],  m_imMask));
                //m_temp_changeMasks.push_back(subtractTwoImages(m_currentClusterOfFramesImg[j],m_currentClusterOfFramesImg[i],  m_imMask));

            }
        }
#if __VERBOSE
        printf("\nm_temp_changeMasks.size() = %d",m_temp_changeMasks.size());
#endif
        if(m_temp_changeMasks.size() > 0)
            andAnImageVec(m_temp_changeMasks,m_currentClusterOfChangeMaskImg[i]); //and all of the change masks
        else
        {
            printf("\nCvZero");
            m_currentClusterOfChangeMaskImg[i] = cvCreateImage(cvSize(m_currentClusterOfFramesImg[i]->width,m_currentClusterOfFramesImg[i]->height),8,1);
            cvZero(m_currentClusterOfChangeMaskImg[i]);
        }
        //clean up the temp vector
        for (size_t ii = 0; ii<m_temp_changeMasks.size();ii++)
               cvReleaseImage(&m_temp_changeMasks[ii]);
        m_temp_changeMasks.clear();


        cvErode(m_currentClusterOfChangeMaskImg[i],m_currentClusterOfChangeMaskImg[i]);

#if __SHOW_CHANGEMASKS
        showChangeMasks();
#endif
//
//        char str[200];
//        //char toWrite[200];
//        ////sprintf(toWrite,"frame %d",m_currentCluster_frameNumbers[i]);
//        sprintf(str,"window %d",i);//, frame %d",i,m_currentCluster_frameNumbers[i]);
//        ////CvFont font;
//        ////cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.75, 0.75, 0, 1.5, CV_AA);
//        ////cvPutText(m_currentClusterOfChangeMaskImg[i], toWrite,cvPoint(20,20) , &font, cvScalar(255));
///*        cvShowImage(str,m_currentClusterOfChangeMaskImg[i]);
//        cvWaitKey(1);*/
//        //


    }

    return(true);
}

bool MultipleBackgroundsModeller::showChangeMasks()
{
    for (size_t i = 0; i<m_currentClusterOfChangeMaskImg.size(); i++)
    {
        char str[200];
        //char toWrite[200];
        ////sprintf(toWrite,"frame %d",m_currentCluster_frameNumbers[i]);
        sprintf(str,"window %d",i);//, frame %d",i,m_currentCluster_frameNumbers[i]);
        ////CvFont font;
        ////cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.75, 0.75, 0, 1.5, CV_AA);
        ////cvPutText(m_currentClusterOfChangeMaskImg[i], toWrite,cvPoint(20,20) , &font, cvScalar(255));
        cvShowImage(str,m_currentClusterOfChangeMaskImg[i]);
        cvWaitKey(1);
    }

    return true;
}

bool MultipleBackgroundsModeller::writeOutputVideo()
{
	if (m_video_Out == NULL)
	{
		printf("\nI can not write the output");
		return false;
	}
#if __SAVE_CHANGE_MASK
	if (video_Out_changeMask == NULL)
	{
		printf("\nI can not write the output");
		return false;
	}
#endif


	CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.75, 0.75, 0, 1.5, CV_AA);


//	IplImage * currFrameClone;
	//currFrameClone = drawBoundingBox(currentFrameImg,currentFrameImgChangeMask);

//	currFrameClone = cvCloneImage(currentFrameImgChangeMask);


#if __WRITE
	char filename[200];
	sprintf(filename, "D:\\Stickleback\\resultFrame\\origFrame%d.bmp",m_currFrameNum);
	cvSaveImage(filename,currentFrameImg);

	sprintf(filename, "D:\\Stickleback\\resultFrame\\cloneFrame%d.bmp",m_currFrameNum);
	cvSaveImage(filename,currFrameClone);
#endif

	char str[100];
	CvPoint ptt = cvPoint(10, 50);
	sprintf(str,"fn=%d",m_currFrameNum);

	//cvPutText(currFrameClone, str,ptt , &font, cvScalar(255, 255, 255, 0));
	//IplImage * currFrameChangeMaskClone = cvCloneImage(currentFrameImgChangeMask);

	//cvPutText(currFrameChangeMaskClone, str, ptt, &font, cvScalar(255,255, 255, 0));

//#if __SAVE_CHANGE_MASK
//	cvWriteFrame(video_Out_changeMask,currentFrameImgChangeMask);
//#endif
//	cvWriteFrame(video_Out,currFrameClone);
//	cvReleaseImage(&currFrameClone);
//	cvReleaseImage(&currFrameChangeMaskClone);

	return true;
}

bool MultipleBackgroundsModeller::extractInfoForCurrentCluster()
{

    std::vector<std::vector<cv::Point> > contours;
    std::vector<std::vector<cv::Point> > filterdContours;
    std::vector<cv::Vec4i> hierarchy;
    for (size_t clstrFrame = 0; clstrFrame <m_currentCluster_frameNumbers.size(); clstrFrame++) // loop over all frames in the current cluster
    {
        if(m_isFrameProcessed[m_currentCluster_frameNumbers[clstrFrame]] == 1) //this frame has already been processed and info are already in the file / m_allExtractedinfo variable
            continue; 
        contours.clear();
        filterdContours.clear();
        hierarchy.clear();

        IplImage* tmp = cvCloneImage(m_currentClusterOfChangeMaskImg[clstrFrame]);
        cv::findContours( tmp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
        cvReleaseImage(&tmp);
#if __VERBOSE
        printf("\noriginal contour size = %d", contours.size());
#endif
        for( size_t i = 0; i < contours.size(); i++ )
        {
#if __VERBOSE
            printf("\nContours[%d] = %d",i,contours[i].size());
#endif
            if( contours[i].size() >= 15 && contours[i].size()<50 )
                filterdContours.push_back(contours[i]);

        }
        contours = filterdContours;
#if __VERBOSE
        printf("\nafter filtering contour size = %d", contours.size());
        printf("\n --------- frame number = %d --------------",m_currentCluster_frameNumbers[clstrFrame]);
        //cvWaitKey(-1);
#endif        
        int  numOfContours = contours.size();
        std::vector<double> currentFrameInfoVec;
        currentFrameInfoVec.push_back(double(numOfContours));
        //m_allFramesExtractedInfo[m_currentCluster_frameNumbers[i] = 

        std::vector<cv::RotatedRect> minEllipse( contours.size() );
        std::vector<cv::Vec4f> lines (contours.size()) ;
        //m_outputInfoStream<<m_currentCluster_frameNumbers[clstrFrame]<<","<<numOfContours; // just to make sure we write the correct value after filtering the small contours...

        for( size_t j = 0; j < contours.size(); j++ )
        {
            minEllipse[j] = cv::fitEllipse( cv::Mat(contours[j]) ); 
            cv::fitLine(cv::Mat(contours[j]),lines[j],CV_DIST_L2,1, 0.001,0.001);
            //m_outputInfoStream<<","<<std::setprecision(5)<<lines[j][0];
            currentFrameInfoVec.push_back(lines[j][0]);
            //m_outputInfoStream<<","<<std::setprecision(5)<<lines[j][1];
            currentFrameInfoVec.push_back(lines[j][1]);
            //m_outputInfoStream<<","<<(int)lines[j][2];
            currentFrameInfoVec.push_back(lines[j][2]);
            //m_outputInfoStream<<","<<(int)lines[j][3];
            currentFrameInfoVec.push_back(lines[j][3]);
            //m_outputInfoStream<<","<<std::setprecision(5)<<fabs(cv::contourArea(cv::Mat(contours[j])));
            cv::Size2f tmp = minEllipse[j].size;
            //m_outputInfoStream<<","<<std::setprecision(5)<<tmp.width<<","<<std::setprecision(5)<<tmp.height;
            currentFrameInfoVec.push_back(tmp.width);
            currentFrameInfoVec.push_back(tmp.height);
        }
            //m_outputInfoStream<<"\n";

            m_allFramesExtractedInfo[m_currentCluster_frameNumbers[clstrFrame]] = currentFrameInfoVec;
            currentFrameInfoVec.clear();
    }
return true;
}

bool    MultipleBackgroundsModeller::writeAllExtractedInfoTotheFile()
{
        for (size_t i = 0;i<m_allFramesExtractedInfo.size();i++)
        {
            m_outputAllFramesInfoSorted<<i+m_globalFirstFrameToProcess<<",";
            std::vector<double>temp = m_allFramesExtractedInfo[i];
            for(size_t j = 0; j<temp.size();j++)
                m_outputAllFramesInfoSorted<<temp[j]<<",";

            m_outputAllFramesInfoSorted<<"\n";
        }
    return (true);
}


IplImage*    MultipleBackgroundsModeller::readFrame(int frameNum)
{
#if __VERBOSE
    printf("\n in read frame, tries to read frame %d , the load range is %d , %d",frameNum,m_loadedAVIFramesRange.first,m_loadedAVIFramesRange.second);
#endif
    if (frameNum>=m_loadedAVIFramesRange.first && frameNum<m_loadedAVIFramesRange.second) // the frame is loaded already 
    {
        IplImage* frameImg = NULL;
        frameImg = cvCloneImage(m_rawAVIFrameBank[frameNum-m_startFrameToProcessInThisChunk]);
#if __VERBOSE
    printf("\n in loaded range");
#endif
        return(frameImg);
    }else
    {
        cvSetCaptureProperty(m_videoCapture,CV_CAP_PROP_POS_FRAMES,double(frameNum));
        IplImage* currFrame = NULL;
        IplImage* frameImg = NULL;
        currFrame = cvQueryFrame(m_videoCapture);
        if(currFrame == NULL)
        {
            printf("I can not read from the video file");
            return(frameImg); // a null image
        }
        else
            frameImg = cvCloneImage(currFrame);
#if __VERBOSE
        printf("\n not in loaded range-- had to read from file");
#endif

        return frameImg;
    }


}



bool MultipleBackgroundsModeller::saveChangeMasksToThefile(std::string savePath)
{
    printf("\ncurrent frame number = %d",m_currFrameNum);
    for (size_t i = 0;i<m_currentClusterOfChangeMaskImg.size();i++)
    {
        char frameFileName[5000];
        sprintf(frameFileName,"%schangeMask%.5d.jpg",savePath.c_str(),m_currentCluster_frameNumbers[i]);
        if(m_currentClusterOfChangeMaskImg[i] == NULL)
        {
            printf("\n change mask %d = NULL, can not save");
            //return false; //DO NOT RETURN, SINCE YOU WILL BE SKIPPING SOME FRAMES
        }
        else
        {
            cvSaveImage(frameFileName,m_currentClusterOfChangeMaskImg[i]);
        }
    }
    return true;
}