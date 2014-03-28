#pragma once
#include "util.h"
#include <math.h>
#include <iostream>

#define PIXEL(img, i, j, k)		(*( ((img)->imageData) + ((img)->widthStep * (i)) + ((img)->nChannels * (j)) + (k) ) )
#define UPIXEL(img, i, j, k)	(*( (unsigned char*) ( ((img)->imageData) + ((img)->widthStep * (i)) + ((img)->nChannels * (j)) + (k)) ))

#define __MIN_SIZE
#define __MAX_SIZE

#define __JUST_KEEP_LARGEST 1;
#define __READ_FRAMES_FROMFILE 0;

#define __JUST_KEEP_LARGEST_WHILE_DENOISING 0;
#define __USE_MOREPHOLOGY_FOR_DENOISING 0;
using namespace cv;
void Tokenize(const std::string& str, std::vector<std::string>& tokens,const std::string& delimiters = " ")
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
	return;
}

int returnAbsDistanceOfVecs(std::vector<int> a1,std::vector<int> a2)
{
    if (a1.size()!=a2.size())
        return(INT_MAX);

//	assert(a1.size()==a2.size()); // would fail if we ask about feature from a frame number that doesn't exist in feature file.
	int dist = 0;
	for (int ii = 1;ii<a1.size();ii++) //exclude frame number (the first element of the vector)
		dist+=abs(a1[ii]-a2[ii]);
	if (dist < 1) // to make sure we don't match a  frame with itself, so distance between the frame and itself is INT_MAX
		dist = INT_MAX;
	return (dist);

}

inline char pixelDistance(unsigned char *thisFrame, unsigned char *bg, int nChannels)
{ 	//Can be faster if you don't use double calculations. use ints.
	if(nChannels == 3)
		return (char) ( 0.299 * (((int) thisFrame[2]) - bg[2]) + 0.587 * (((int) thisFrame[1]) - bg[1]) + 0.114 * ( ((int)thisFrame[0]) - bg[0]) );		
	else 
		return (char) (thisFrame[0] - bg[0]);

}
//TODO: return true/false and return output in paramaters
//IplImage* readFrame(int frameNum, CvCapture *videoCapture, std::string pathToFiles)
//{
//	IplImage *frameImg = NULL;
//
//#if __READ_FRAMES_FROMFILE
//	{
//		char tmp[200]; //READ FROM FILES ! 
//		sprintf(tmp,"frame%d.bmp", frameNum);
//		std::string fileName;
//		fileName= pathToFiles + tmp;
//		frameImg = cvLoadImage(fileName.c_str());
//		if (frameImg == NULL)
//			printf("\nCan not read frame %d in readFrameFromFile, filename: %s",frameNum,fileName.c_str());
//
//	}
//
//#else
//
//		cvSetCaptureProperty(videoCapture,CV_CAP_PROP_POS_FRAMES,double(frameNum));
//		//std::cout << cvGetCaptureProperty(videoCapture,CV_CAP_PROP_POS_FRAMES) << std::endl;
//
//		IplImage* currFrame = NULL;
//		currFrame = cvQueryFrame(videoCapture);
//		if(currFrame == NULL)
//		{
//			printf("I can not read from the video file");
//			getchar();
//		}
//
//		frameImg = cvCloneImage(currFrame);
////	}
//#endif
//	return frameImg;
//}
//#ifdef __LOAD_AVI_TO_MEM
//IplImage* readFrame(int frameNum, std::vector<IplImage*> frameBank)
//{
//    if(frameBank.size()<frameNum)
//    {
//        printf("\n this frame does not exist in the frame bank");
//        return (NULL);
//    }
//    IplImage* frameImg = NULL;
//        frameImg = cvCloneImage(frameBank[frameNum]);
//    return(frameImg);
//}

//#else
//IplImage* readFrame(int frameNum, CvCapture *videoCapture, std::string pathToFiles)
//{
//	cvSetCaptureProperty(videoCapture,CV_CAP_PROP_POS_FRAMES,double(frameNum));
//	IplImage* currFrame = NULL;
//	IplImage* frameImg = NULL;
//	currFrame = cvQueryFrame(videoCapture);
//	if(currFrame == NULL)
//	{
//		printf("I can not read from the video file");
//		return(frameImg); // a null image
//	}
//	else
//		frameImg = cvCloneImage(currFrame);
//	return frameImg;
//}
//
//#endif



//new distance
double calculateDistanceOfTwoImages(IplImage *img1, IplImage *img2, IplImage *imgMask)
{
	double toRet =-1;
	double tmpDist = 0;
	IplImage *diffImage = cvCloneImage(img1);cvZero(diffImage);
	//IplImage *maskedDiffImage = cvCloneImage(diffImage);cvZero(maskedDiffImage);
	IplImage *maskedDiff1Channel = cvCreateImage(cvSize(diffImage->width,diffImage->height),8,1);
	cvAbsDiff(img1, img2, diffImage);
	cvCvtColor(diffImage, maskedDiff1Channel, CV_RGB2GRAY);
	cvAnd(maskedDiff1Channel,imgMask,maskedDiff1Channel);

	CvScalar diffSum;
	diffSum = cvSum(maskedDiff1Channel);
	toRet = (double)diffSum.val[0]/10000.0;

	cvReleaseImage(&diffImage);
	cvReleaseImage(&maskedDiff1Channel);
	return toRet; 

}

//double calculateDistanceOfTwoImages(IplImage *img1, IplImage *img2, IplImage *imgMask)
//{
//	//cvShowImage("img1 in dstanc",img1);
//	//cvShowImage("img2 in dstanc",img2);
//
//	double toRet =-1;
//	double tmpDist = 0;
//	IplImage *diffImage = cvCloneImage(img1);cvZero(diffImage);
//	cvAbsDiff(img1, img2, diffImage);
//	//cvShowImage("diffImage", diffImage);
//	//cvWaitKey(1);
//	CvScalar diffAvg;
//	diffAvg = cvAvg(diffImage, imgMask);
//	toRet = (diffAvg.val[0] + diffAvg.val[1] + diffAvg.val[2])/3.0;
//	cvReleaseImage(&diffImage);
//	return toRet; 
//
//}


//IplImage* subtractTwoImages(IplImage *img1, IplImage *img2, IplImage *imgMask)
//{
//	IplImage *diffImage = cvCloneImage(img1);
//	cvZero(diffImage);
//	cvSub(img1,img2,diffImage,imgMask);
//	cvShowImage("diffImage", diffImage);
//	cvWaitKey(-1);
//	IplImage *diffImageGray = cvCreateImage(cvGetSize(diffImage),8,1); 
//	cvCvtColor( diffImage, diffImageGray, CV_RGB2GRAY );
//
//	//IplImage* im_bw = cvCreateImage(cvGetSize(diffImageGray),IPL_DEPTH_8U,1);
//	//cvThreshold(diffImageGray, im_bw, 50, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
//	//cvReleaseImage(&diffImageGray);
//	cvReleaseImage(&diffImage);
//
//	//return im_bw;
//
//	return diffImageGray;
//
//	//double myThresh = 30;
//	//
//	//cvZero(diffImage);
//	//subtractTwoImages
//	//cvAbsDiff(img1, img2, diffImage);
//	//cvShowImage("diffImage", diffImage);
//	//cvAnd(diffImageGray, diffImageGray, diffImageGray,m_imMask); // just apply the mask.
//	//cvThreshold(diffImageGray,diffImageGray,myThresh, 255.0,  CV_THRESH_BINARY | CV_THRESH_OTSU)
//	//cvShowImage("diffImageGray", diffImageGray);
//	//cvWaitKey(1);
//	//cvReleaseImage(&diffImage);
//	//;
//
//}


IplImage* subtractTwoImages(IplImage *img1, IplImage *img2, IplImage *imgMask)
{
	IplImage* changeMask;
	changeMask = cvCreateImage( cvGetSize(img1),IPL_DEPTH_8U, 1); //Reza: these can be single channel, coz we just care about changes/ later: now it is! :D
	cvZero(changeMask);

	char threshold = 15;
	int i,j; // Defined above so as to unable parallelization (OpenMP)
	//#pragma omp parallel shared(thisFrame, changeMask) private(i,j)		
	{
		//#pragma omp for schedule(dynamic)
		for(i= 0; i<img1->height; ++i)
		{
			for(j=0; j<img1->width; ++j) 
			{
				if(pixelDistance( &UPIXEL(img1, i, j, 0), &UPIXEL(img2, i, j, 0), 3) <threshold )
				{
					//for(int k = 0; k < img2->nChannels; ++k)
					//{
					//	if (fc % 1000 == 0)
					//		 UPIXEL(img2, i, j, k) = (unsigned char) (alpha * UPIXEL(img1, i, j, k) + (1.0-alpha) * UPIXEL(img2, i, j, k));
					//}
				}
				else
				{
					PIXEL(changeMask,i, j, 0) |= 0xFF; //Set all bits to 1 (White color)
					PIXEL(changeMask,i, j, 0) &= PIXEL(imgMask,i, j, 0);
				}
			}
		}
	}
//	cvShowImage("changeMask",changeMask);
	return changeMask;

}

bool andAnImageVec(std::vector<IplImage *>inputVec, IplImage* &result)
{
	if (inputVec.size() == 0)
		printf("\nInput vector of andAnImageVec has no member");
	//IplImage* changeMask, *img1;
	IplImage* img1 = inputVec[0];
	result = cvCreateImage( cvGetSize(img1),IPL_DEPTH_8U, 1); 


	cvZero(result); cvNot(result,result); //CHECK THIS FOR MEMORY LEAK!

	int i,j; // Defined above so as to unable parallelization (OpenMP)
	//#pragma omp parallel shared(thisFrame, changeMask) private(i,j)		
	{
		//#pragma omp for schedule(dynamic)
		for (int bf = 0;bf<inputVec.size();bf++)
		{
			for(i= 0; i<img1->height; ++i)
			{
				for(j=0; j<img1->width; ++j) 
				{
						PIXEL(result,i, j, 0) &= PIXEL(inputVec[bf],i, j, 0);
				}
			}
		}
	}

	//cvShowImage("And Of All ChangeMasks",changeMask);
	//cvWaitKey(-1);
	return true;

}



//IplImage * denoiseAChangeMask(IplImage* changeMask)
#if 0
bool denoiseAChangeMask(IplImage* changeMask, IplImage* &denoisedChangeMask)
{
	//cvShowImage("chngmsk",changeMask);
	//IplImage* denoisedChangeMask;//, *tmp;	
	//denoisedChangeMask = cvCreateImage( cvGetSize(changeMask),IPL_DEPTH_8U, 1);
	cvZero(denoisedChangeMask); 


#if __JUST_KEEP_LARGEST_WHILE_DENOISING 

	CvMemStorage* storage2 = cvCreateMemStorage();
	CvSeq* first_contour2 = NULL;
	int Nc2 = cvFindContours(changeMask,storage2,&first_contour2,sizeof(CvContour),CV_RETR_EXTERNAL);//	CV_RETR_LIST);

	if (Nc2 == 0) // if we don't detect any thing just return an empty change mask
		return true;

	CvSeq* largestContour = first_contour2; // it has at leaset one member, 

	for( CvSeq* c=first_contour2; c!=NULL; c=c->h_next ) //make the contourlist I just want to include contours with Area > MINAREAOFCONTOURS;
			//if (   (fabs(cvContourArea(c)) > 100) &&  (fabs(cvContourArea(c))< 200)         )
			if (   (fabs(cvContourArea(c))) >  fabs(  cvContourArea(largestContour)  )  )
				largestContour = c;


	cvDrawContours(denoisedChangeMask,largestContour, cvScalar(255,255,255), cvScalar(255), -1, CV_FILLED, 8);

	cvReleaseMemStorage(&storage2);

	return true;


#endif


#if __USE_MOREPHOLOGY_FOR_DENOISING 
	IplImage* myTemp;
	myTemp = cvCreateImage( cvGetSize(denoisedChangeMask),IPL_DEPTH_8U, 1);

	cvErode(changeMask,myTemp,NULL,1);
	cvDilate(myTemp,denoisedChangeMask,NULL,2);//changed from 2 (last aprameter)
	cvReleaseImage(&myTemp);
#else
	denoisedChangeMask=cvCloneImage(changeMask);
#endif


	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* first_contour = NULL;
	int Nc = cvFindContours(denoisedChangeMask,storage,&first_contour,sizeof(CvContour),CV_RETR_EXTERNAL);//	CV_RETR_LIST);

	cvZero(denoisedChangeMask);
	if (Nc == 0) // if we don't detect any thing just return an empty change mask
		return true;

	for( CvSeq* c=first_contour; c!=NULL; c=c->h_next ) //make the contourlist I just want to include contours with Area > MINAREAOFCONTOURS;
			//if (   (fabs(cvContourArea(c)) > 100) &&  (fabs(cvContourArea(c))< 200)         )
			//if (   (fabs(cvContourArea(c)) > 20) &&  (fabs(cvContourArea(c))< 250)         )
			if (   (fabs(cvContourArea(c)) > 20) )// &&  (fabs(cvContourArea(c))< 250)         )
				cvDrawContours(denoisedChangeMask,c, cvScalar(255,255,255), cvScalar(255), -1, CV_FILLED, 8);

	cvReleaseMemStorage(&storage);


	//std::vector<std::vector<cv::Point> > contours;
	//std::vector<cv::Vec4i> hierarchy;
	//std::vector::const_iterator<cv::Vec4i> vv;

	//std::vector<cv::RotatedRect> minEllipse( contours.size() );

	//IplImage* tmp = cvCloneImage(denoisedChangeMask);
	//cv::findContours( tmp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
	//cvReleaseImage(&tmp);
	//for( int i = 0; i < contours.size(); i++ )
	//{ 
	//	minEllipse[i] = cv::fitEllipse( cv::Mat(contours[i]) );  
	//	cv::Size2f tmpsz = minEllipse[i].size;
	//	vv = hierarchy.begin();
	//	if (tmpsz.height > 35)
	//	{
	//		contours.erase(i);
	//		hierarchy.erase(i);
	//	}
	//}

	//cvZero(denoisedChangeMask);
	//cv::drawContours(denoisedChangeMask,contours,-1,CV_RGB(255,255,255),1,8,hierarchy);
	//cvShowImage("dnchmgsk",denoisedChangeMask);

	return true;

}

#endif
IplImage* mergeChangeMask(IplImage* currentFrameImg, IplImage* currentFrameImgChangeMask)
{
	IplImage* currFrameClone;
	currFrameClone = cvCloneImage(currentFrameImg);

	int i,j; // Defined above so as to unable parallelization (OpenMP)
	//#pragma omp parallel shared(thisFrame, changeMask) private(i,j)		
	{
		//#pragma omp for schedule(dynamic)
		for(i= 0; i<currentFrameImg->height; ++i)
			for(j=0; j<currentFrameImg->width; ++j) 
					PIXEL(currFrameClone,i, j, 0) |= PIXEL(currentFrameImgChangeMask,i, j, 0); //Set all bits to 1 (White color)
	}
	return currFrameClone;
}

//IplImage* drawBoundingBox(IplImage* currentFrameImg, IplImage* currentFrameImgChangeMask)
//{
//	//this needs to be changed.. its here as a temprary..!
//	IplImage* currFrameClone;
//	currFrameClone = cvCloneImage(currentFrameImg);
//
//	//CvMemStorage* storage = cvCreateMemStorage();
//	//CvSeq* first_contour = NULL;
//
//	//int Nc = cvFindContours(currentFrameImgChangeMask,storage,&first_contour,sizeof(CvContour),CV_RETR_EXTERNAL);//	CV_RETR_LIST);
//
//	//CvSeq *largestContour = first_contour;
//
//	//for( CvSeq* c=first_contour; c!=NULL; c=c->h_next )//make the contourlist I just want to include contours with Area > MINAREAOFCONTOURS;
//	//	if (fabs(cvContourArea(c)) >  fabs(cvContourArea(largestContour)))
//	//		largestContour = c;
//
//
//	//cvDrawContours(currFrameClone,largestContour,cvScalar(255,0,0),cvScalar(255,0,0),-1,CV_FILLED,8);
//
//	std::vector<std::vector<cv::Point> > contours;
//	std::vector<cv::Vec4i> hierarchy;
//
//	// the stupid  find Contour modifies the image when it is extracting the contour
//	//thats why I have to clone the image to keep original currFrameImgChangeMask
//	IplImage* tmp = cvCloneImage(currentFrameImgChangeMask);
//	//cvSaveImage("currentFrameImgChangeMask.bmp",currentFrameImgChangeMask);
//	//cvSaveImage("currentFrameImg.bmp",currentFrameImg);
//
//	cv::findContours( tmp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
//	cvReleaseImage(&tmp);
//
//#if __JUST_KEEP_LARGEST
//	if (contours.size()>1)
//	{
//		//printf("\ncontours.size() = %d",contours.size());
//		std::vector<std::vector<cv::Point> > temp;
//		temp = contours;
//		std::vector<cv::Point> largestContour;
//		if (contours.size()>0) // just for sanity check ..
//			largestContour = contours[0]; // it has at leaset one member, 
//		for (int i = 1;i<contours.size();i++)
//			if (fabs(cv::contourArea(cv::Mat(contours[i]))) > fabs(cv::contourArea(cv::Mat(largestContour))))
//				largestContour = contours[i];
//
//		contours.clear();
//		contours.push_back(largestContour);
//	}
//
//#endif
//
//
//  /// Find the rotated rectangles and ellipses for each contour
//	//std::vector<cv::RotatedRect> minRect( contours.size() );
//	std::vector<cv::RotatedRect> minEllipse( contours.size() );
//	std::vector<cv::Vec4f> lines (contours.size()) ;
//
//  for( int i = 0; i < contours.size(); i++ )
//     {// minRect[i] = minAreaRect( Mat(contours[i]) );
//		
//       if( contours[i].size() > 5 )
//         { 
//			// minEllipse[i] = fitEllipse( Mat(contours[i]) );  //UN COMMENT THIS IFYOU WANT TO HAVE ELLIPSE
//	   		 cv::fitLine(cv::Mat(contours[i]),lines[i],CV_DIST_L2,1, 0.001,0.001); // vx , vy, x0, y0 in line structure
//				   
//		}
//     }
//
//// vx , vy, x0, y0 in line structure
//  cv::Mat imgMat(currFrameClone);
//  CvFont font;
//  cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
//
//  for( int i = 0; i< contours.size(); i++ )
//     {
//
//		 cv::Scalar color = cv::Scalar( 0, 0, 255 );
//		 cv::ellipse( imgMat, minEllipse[i], color, 1, 8 );
//		 
//	  }
//
//if (contours.size() != 0)
//	cv::drawContours(imgMat,contours,-1,Scalar(255,0,0),-1);
//
//  for( int i = 0; i< contours.size(); i++ )
//     {
//		float t = (float)(20);
//		CvPoint pt1, pt2;
//		pt1.x = cvRound(lines[i][2] - lines[i][0]*t);
//		pt1.y = cvRound(lines[i][3] - lines[i][1]*t);
//		pt2.x = cvRound(lines[i][2] + lines[i][0]*t);
//		pt2.y = cvRound(lines[i][3] + lines[i][1]*t);
//		cvLine( currFrameClone, pt1, pt2, CV_RGB(0,255,0), 1.5, CV_AA, 0 );
//		//char str[100];
//		//sprintf(str,"angle = %2lf",(180/3.14)*atan2(lines[i][1],lines[i][0]));
//		//cvPutText(currFrameClone, str, cvPoint(200, 200 ), &font, cvScalar(255,0,0));
//  }
//
//
//	//cvReleaseMemStorage(&storage);
//
//
//	return currFrameClone;
//}
//
//
//
//
//
//
//
//

IplImage* drawBoundingBox(IplImage* currentFrameImg, IplImage* currentFrameImgChangeMask)
{
	//this needs to be changed.. its here as a temprary..!
	IplImage* currFrameClone;
	currFrameClone = cvCloneImage(currentFrameImg);

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	// the stupid  find Contour modifies the image when it is extracting the contour
	//thats why I have to clone the image to keep original currFrameImgChangeMask
	IplImage* tmp = cvCloneImage(currentFrameImgChangeMask);
	//cvSaveImage("currentFrameImgChangeMask.bmp",currentFrameImgChangeMask);
	//cvSaveImage("currentFrameImg.bmp",currentFrameImg);

	cv::findContours( tmp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
	cvReleaseImage(&tmp);

#if __JUST_KEEP_LARGEST
	if (contours.size()>1)
	{
		//printf("\ncontours.size() = %d",contours.size());
		std::vector<std::vector<cv::Point> > temp;
		temp = contours;
		std::vector<cv::Point> largestContour;
		if (contours.size()>0) // just for sanity check ..
			largestContour = contours[0]; // it has at leaset one member, 
		for (int i = 1;i<contours.size();i++)
			if (fabs(cv::contourArea(cv::Mat(contours[i]))) > fabs(cv::contourArea(cv::Mat(largestContour))))
				largestContour = contours[i];

		contours.clear();
		contours.push_back(largestContour);
	}

#endif


  /// Find the rotated rectangles and ellipses for each contour
	//std::vector<cv::RotatedRect> minRect( contours.size() );
	std::vector<cv::RotatedRect> minEllipse( contours.size() );
	std::vector<cv::Vec4f> lines (contours.size()) ;

  for( int i = 0; i < contours.size(); i++ )
     {// minRect[i] = minAreaRect( Mat(contours[i]) );
		
       if( contours[i].size() > 5 )
         { 
			// minEllipse[i] = fitEllipse( Mat(contours[i]) );  //UN COMMENT THIS IFYOU WANT TO HAVE ELLIPSE
	   		 cv::fitLine(cv::Mat(contours[i]),lines[i],CV_DIST_L2,1, 0.001,0.001); // vx , vy, x0, y0 in line structure
				   
		}
     }

// vx , vy, x0, y0 in line structure
  cv::Mat imgMat(currFrameClone);
  CvFont font;
  cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

  for( int i = 0; i< contours.size(); i++ )
     {

		 cv::Scalar color = cv::Scalar( 0, 0, 255 );
		 cv::ellipse( imgMat, minEllipse[i], color, 1, 8 );
		 
	  }

if (contours.size() != 0)
	cv::drawContours(imgMat,contours,-1,Scalar(255,0,0),-1);

  for( int i = 0; i< contours.size(); i++ )
     {
		float t = (float)(20);
		CvPoint pt1, pt2;
		pt1.x = cvRound(lines[i][2] - lines[i][0]*t);
		pt1.y = cvRound(lines[i][3] - lines[i][1]*t);
		pt2.x = cvRound(lines[i][2] + lines[i][0]*t);
		pt2.y = cvRound(lines[i][3] + lines[i][1]*t);
		cvLine( currFrameClone, pt1, pt2, CV_RGB(0,255,0), 1.5, CV_AA, 0 );
		//char str[100];
		//sprintf(str,"angle = %2lf",(180/3.14)*atan2(lines[i][1],lines[i][0]));
		//cvPutText(currFrameClone, str, cvPoint(200, 200 ), &font, cvScalar(255,0,0));
  }


	//cvReleaseMemStorage(&storage);


	return currFrameClone;
}



bool returnContourCenter( CvSeq* contour, CvPoint2D32f &center)
{
	if (contour == NULL)
	{
		printf("\n contour is NULL!");
		return false;
	}

	double totalX=0.0,totalY=0.0;
	double total=0.0;
	for( int k=0; k<contour->total;k++)
	{
		CvPoint* p = (CvPoint*)cvGetSeqElem(contour,k);
		totalX += p->x;
		totalY += p->y;
		total++;
	}
	center.x = (float)(totalX/total);
	center.y = (float)(totalY/total);
	return true;

}

bool hasEnoughDistanceWithOtherMembers(std::vector<int>inVec, int quary, int thresh=10)
{
	for (int ii = 0;ii<inVec.size();ii++)
		if (abs(inVec[ii]-quary)<thresh)
			return false;

	return true;

}

//bool denoiseAChangeMask(IplImage* changeMask, IplImage* &denoisedChangeMask)
IplImage* denoiseAChangeMask(IplImage* changeMask)//, IplImage* &denoisedChangeMask)
{
	IplImage* denoisedChangeMask = cvCreateImage(cvSize(changeMask->width,changeMask->height),8,1);

#if __JUST_KEEP_LARGEST_WHILE_DENOISING 

	CvMemStorage* storage2 = cvCreateMemStorage();
	CvSeq* first_contour2 = NULL;
	int Nc2 = cvFindContours(changeMask,storage2,&first_contour2,sizeof(CvContour),CV_RETR_EXTERNAL);//	CV_RETR_LIST);

	if (Nc2 == 0) // if we don't detect any thing just return an empty change mask
	{
		cvZero(denoisedChangeMask);
		return (denoisedChangeMask);
	}

	CvSeq* largestContour = first_contour2; // it has at leaset one member, 

	for( CvSeq* c=first_contour2; c!=NULL; c=c->h_next ) //make the contourlist I just want to include contours with Area > MINAREAOFCONTOURS;
			//if (   (fabs(cvContourArea(c)) > 100) &&  (fabs(cvContourArea(c))< 200)         )
			if (   (fabs(cvContourArea(c))) >  fabs(  cvContourArea(largestContour)  )  )
				largestContour = c;

	cvDrawContours(denoisedChangeMask,largestContour, cvScalar(255,255,255), cvScalar(255), -1, CV_FILLED, 8);
	cvReleaseMemStorage(&storage2);

	return (denoisedChangeMask);
#endif

#if __USE_MOREPHOLOGY_FOR_DENOISING 
	IplImage* myTemp;
	myTemp = cvCreateImage( cvGetSize(denoisedChangeMask),IPL_DEPTH_8U, 1);

	cvErode(changeMask,myTemp,NULL,1);
	cvDilate(myTemp,denoisedChangeMask,NULL,2);//changed from 2 (last aprameter)
	cvReleaseImage(&myTemp);
#else
//	denoisedChangeMask=cvCloneImage(changeMask);



	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* first_contour = NULL;
//	int Nc = cvFindContours(denoisedChangeMask,storage,&first_contour,sizeof(CvContour),CV_RETR_EXTERNAL);//	CV_RETR_LIST);
	int Nc = cvFindContours(changeMask,storage,&first_contour,sizeof(CvContour),CV_RETR_EXTERNAL);//	CV_RETR_LIST);
	cvZero(denoisedChangeMask);
	if (Nc == 0) // if we don't detect any thing just return an empty change mask
		return (denoisedChangeMask);

	for( CvSeq* c=first_contour; c!=NULL; c=c->h_next ) //make the contourlist I just want to include contours with Area > MINAREAOFCONTOURS;
			//if (   (fabs(cvContourArea(c)) > 100) &&  (fabs(cvContourArea(c))< 200)         )
			//if (   (fabs(cvContourArea(c)) > 20) &&  (fabs(cvContourArea(c))< 250)         )
			if (   (fabs(cvContourArea(c)) > 20) )// &&  (fabs(cvContourArea(c))< 250)         )
				cvDrawContours(denoisedChangeMask,c, cvScalar(255,255,255), cvScalar(255), -1, CV_FILLED, 8);

	cvReleaseMemStorage(&storage);
	return (denoisedChangeMask);
#endif
}


