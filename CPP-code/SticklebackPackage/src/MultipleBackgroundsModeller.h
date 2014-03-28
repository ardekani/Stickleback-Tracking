#ifndef __MULTIBACK_H_INCLUDED__
#define __MULTIBACK_H_INCLUDED__
//#pragma once
//#include "cv.h"
//#include "highgui.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <iomanip>


struct candidateFrame{

	int frameNum;
	double distance;
	bool operator <(const candidateFrame& rhs)
	{
		return distance < rhs.distance;
	}
};

struct frameDetectedInfo{

    int frameNumber;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4f> prevFrameFittedLine; // this is supposed to keep the info for position and direction of fish in previous frame
	std::vector<cv::Vec4f> fittedLines;


};

//typedef std::pair<int,double> frame_distance; //frame number and distance to the target frame

class MultipleBackgroundsModeller
{
public:
    MultipleBackgroundsModeller(void);
    ~MultipleBackgroundsModeller(void);
    bool                                initialize(std::string inputVideoFileName, std::string inputMaskFileName, int wSize, int mPeriod, int startFrame , int lastFrame, std::string featuresFileName="", int numOfFramesToLoadInMemory=3000, std::string outputPathForChangeMasks="");
    bool                                finish();
    //void findThisFrameChangeMask(unsigned int);
    bool                                processFrameNumber(int frameNum);
    void                                setChunkSize(int inp) {m_chunkSize = inp;}

    bool                                writeAllExtractedInfoTotheFile(void);
    bool                                showChangeMasks();
	

	bool                                saveChangeMasksToThefile(std::string savePath);
private:

    int                                 m_windowSize;
    int                                 m_masterPeriod;
    int                                 m_currFrameNum;
    int                                 m_startFrameToProcessInThisChunk;
    int                                 m_lastFrameToProcessInThisChunk;
    bool                                m_useShortListOfFrames; // should it try to use the periodic nature of the video for guessing close frames or not
    CvCapture*                          m_videoCapture;
    IplImage*                           m_imMask;
    IplImage*                           m_currentFrameImg;
    IplImage*                           m_currentFrameImgChangeMask;

    std::vector<IplImage *>             m_temp_changeMasks;
    std::vector<IplImage *>             m_currentClusterOfFramesImg;
    std::vector<IplImage *>             m_currentClusterOfChangeMaskImg;
    std::vector<candidateFrame>         m_candidateFrames;
    std::vector<int>                    m_currentCluster_frameNumbers;
    CvVideoWriter*                      m_video_Out;
    CvVideoWriter*                      m_video_Out_changeMask;
    bool                                m_isInit;
    int                                 m_isFrameProcessed[30000];                          // to keep track of frame numbers that have been processed // a hash table... 
    std::ofstream                       m_outputInfoStream;
    std::ofstream                       m_outputAllFramesInfoSorted;
    //std::vector<frameDetectedInfo>      m_allFramesDetectedInfo;
    std::vector<IplImage*>              m_rawAVIFrameBank;
    std::map<int, std::vector<int>>     m_allFramesFeatures; //frame number as key, and features as an int vector
    std::pair<int,int>                  m_loadedAVIFramesRange;
    int                                 m_chunkSize;
    std::vector<candidateFrame>         m_candidateFramesDistanceList;
    std::vector<int>                    m_shortListOfPotentialGoodFrames;

    std::map<int, std::vector<double>>  m_allFramesExtractedInfo; //frame number as key, and features as an int vector

    std::string                         m_changeMaskOutputSavePath; // the folder that keeps change masks in jpeg image


    bool                                loadRawAVIFramesToMemory(int startFrame, int stopFrame);

    bool                                buildCandidateFramesListForCurrentFrame(void); // find a list of potential close frames to the current frame - find the clusters that the current frame belongs to
    bool                                findCurrentClusterFrameNumbers(void);
    bool                                loadAllFeaturesFromFile(std::string fn); //populate m_allFramesFeatures
    IplImage*                           readFrame(int frameNum);

//bool MultipleBackgroundsModeller::calculateChangeMasks(void);
//bool MultipleBackgroundsModeller::writeOutputVideo(void);

    bool                                calculateChangeMasks(); //(int clusterIndx)
    //bool                                calculateTempChangeMasks(int cluster_member_index);
    bool                                extractInfoForCurrentCluster(void); //current frame's info... add it to m_allFramesDetectedInfo
    bool                                writeInfoForCurrentClusterTotheFile(void); //current frame's info... add it to m_allFramesDetectedInfo


    bool                                writeOutputVideo(void);
    ///this should be called man!
    bool MultipleBackgroundsModeller::findShortListOfFrames(void);

//frameDetectedInfo m_currFrameDetectedInfo; // this is gonna keep the contours, postion of fish in previous frames and so on.
//std::vector<std::vector<clock_t>> m_profiler; // to keep the processing time for each step..
//bool MultipleBackgroundsModeller::buildDistanceVectorForCurrentFrame(void);
    std::vector<int>    returnBestFrames();
    bool unloadRawAVIFramesFromMemory(void);
    int m_globalFirstFrameToProcess; // in the video
    int m_globalLastFrameToProcess;  // in the video

};

#endif //__MULTIBACK_H_INCLUDED__