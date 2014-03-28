#pragma once
#ifndef __UTIL_H_INCLUDED__
#define __UTIL_H_INCLUDED__
#include "cv.h"
#include "highgui.h"


int returnAbsDistanceOfVecs(std::vector<int> a1,std::vector<int> a2);
void Tokenize(const std::string& str, std::vector<std::string>& tokens,const std::string& delimiters);

//IplImage* readFrame(int frameNum, CvCapture *videoCapture);
//#ifdef __LOAD_AVI_TO_MEM
//IplImage* readFrame(int frameNum, std::vector<IplImage*> frameBank);
//#else
//IplImage* readFrame(int frameNum, CvCapture *videoCapture, std::string pathToFrames="");
//
//#endif

//bool readFrame(IplImage*frameImg, int frameNum, CvCapture *videoCapture, std::string pathToFiles="");

double calculateDistanceOfTwoImages(IplImage *img1, IplImage *img2, IplImage *imgMask);
IplImage* subtractTwoImages(IplImage *img1, IplImage *img2, IplImage *imgMask);
bool andAnImageVec(std::vector<IplImage *> inputVec, IplImage* &res);
//bool denoiseAChangeMask(IplImage* changeMask, IplImage* &res);
IplImage* denoiseAChangeMask(IplImage* changeMask);
IplImage* mergeChangeMask(IplImage* currentFrameImg, IplImage* currentFrameImgChangeMask);
IplImage* drawBoundingBox(IplImage* currentFrameImg, IplImage* currentFrameImgChangeMask); //being lazy! this function and mergeChangeMask should be just one and should be written differently!
bool returnContourCenter( CvSeq* contour, CvPoint2D32f &center);

bool hasEnoughDistanceWithOtherMembers(std::vector<int>inVec, int quary, int thresh);
bool loadAVIFile(CvCapture *videoCapture, int startFrame, int stopFrame,std::vector<IplImage*> &toRet);
#endif //__UTIL_H_INCLUDED__