/*  File: findROI.cpp
Purpose: To help the detection of the inner part of the fish tank as ROI for video processing
what it does right now is to generate an average frame from all the frames.
Author: Reza Ardekani
Date: 7/15/2013
*/

#define SB_OK 0
#define SB_ERR 1
#include <cv.h>
#include <highgui.h>
#include "util.h"
#include <fstream>
#include <iostream>

int calculateAndWriteTheBackground(std::string inputVideoFileName)
{
	std::string outputMaskFileName;
	outputMaskFileName = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
	outputMaskFileName+="_averageMask.bmp";
	CvCapture* videoCapture = cvCreateFileCapture(inputVideoFileName.c_str());

	if (videoCapture == NULL)
	{
		printf("\nCan not open the video: %s",inputVideoFileName.c_str());
		return (SB_ERR);
	}

	int totalFrames = (int)cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_COUNT);
	int h = (int)cvGetCaptureProperty(videoCapture,CV_CAP_PROP_FRAME_HEIGHT);
	int w = (int)cvGetCaptureProperty(videoCapture,CV_CAP_PROP_FRAME_WIDTH);

	//Allocate all needed memories..

	IplImage* sumMatrix = cvCreateImage(cvSize(w,h), IPL_DEPTH_64F, 3);
	cvZero(sumMatrix);
	IplImage* currentFrame64bits = cvCreateImage(cvSize(w,h), IPL_DEPTH_64F, 3);
	IplImage* sumMatrix8bit = cvCreateImage(cvSize(w,h), 8, 3);

	int countRun=0;
	IplImage* x;
	for (int fn = 1000;fn<8000;fn+=50,countRun++)
	{
		x = readFrame(fn,videoCapture);
		if (x==NULL)
			return(SB_ERR);

		cvConvertScale(x,currentFrame64bits,1.0);
		cvAdd(currentFrame64bits,sumMatrix,sumMatrix);
		cvReleaseImage(&x);
	}
	cvConvertScale(sumMatrix,sumMatrix8bit,1.0/countRun);

	cvSaveImage(outputMaskFileName.c_str(),sumMatrix8bit);
	//memory clean up
	cvReleaseImage(&sumMatrix);
	cvReleaseImage(&currentFrame64bits);
	cvReleaseImage(&sumMatrix8bit);
	cvReleaseCapture(&videoCapture);

	printf("\nDone");
	return (SB_OK);
}
using namespace std;
int main(int argc, char** argv)
{
	//std::string listOfVideos = "G:\\Peichel\\PJxP_Family1\\school_only\\listoffiles.txt";
	//std::string listOfVideos = "G:\\Peichel\\PJxP_Family4\\school_only\\listofFilesshorter.txt";
	std::string listOfVideos = "G:\\Peichel\\PJxPJ_F2_2011\\school_only\\listoffiles.txt";
	std::string listOfFailedVideo = "G:\\Peichel\\PJxP_Family1\\school_only\\listoffiles_failed.txt";
	FILE* fout = fopen(listOfFailedVideo.c_str(),"w");

	std::string line;
	std::ifstream infile (listOfVideos.c_str());
	std::ofstream outfile (listOfFailedVideo.c_str());
	if (infile.is_open())
	{
		while ( infile.good() )
		{
			getline (infile,line);
			std::cout<<line;
			if (SB_OK!=calculateAndWriteTheBackground(line))
			{
				outfile<<line;
				printf("-->failed");
			}
		}
		infile.close();
	}
	else std::cout << "Unable to open file"; 

	fclose(fout);
	getchar();
	return 0;
}

