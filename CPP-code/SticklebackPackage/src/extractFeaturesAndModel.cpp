#include <cv.h>
#include <highgui.h>
#include <math.h>
#include "string.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <math.h>
#include "util.h"
#define pi 3.14159265
#define __USE_SMOOTHED_FILE 0
#define __SHOW_OUTPUT 0
#define __NOMASK 0
#define __RECORD_VIDEO_OUTPUT 1
#define __JUST_EXTRACT_CENTER 1
void sharpenAFrame(const IplImage* oriImg, IplImage * dstImg)
{

    IplImage* grayImg = cvCreateImage(cvGetSize(oriImg), 8, 1);
    cvCvtColor(oriImg, grayImg, CV_BGR2GRAY);

    cvSmooth(grayImg, grayImg, CV_GAUSSIAN, 3, 0, 0, 0);

    IplImage* laplaceImg = cvCreateImage(cvGetSize(oriImg), IPL_DEPTH_16S, 1);
    IplImage* abs_laplaceImg = cvCreateImage(cvGetSize(oriImg), IPL_DEPTH_8U, 1);
    cvLaplace(grayImg, laplaceImg, 3);
    cvConvertScaleAbs(laplaceImg, abs_laplaceImg, 1, 0);
    dstImg = cvCreateImage(cvGetSize(oriImg), IPL_DEPTH_8U, 1);
    cvAdd(abs_laplaceImg, grayImg, dstImg, NULL); 
    cvShowImage("abs_laplaceImg",abs_laplaceImg);
    cvShowImage("laplaceImg",laplaceImg);
    cvShowImage("grayImg",grayImg);
    cvWaitKey(-1);

}

//void returnHaarFeatures (IplImage* inImg, std::vector<int> &featureVec, int gridSize = 20)
void returnHaarFeatures (IplImage* gray, CvMat* sum, std::vector<int> &featureVec, int gridSize = 20)
{
    //    CvMat* sum = cvCreateMat(gray->height + 1, gray->width + 1, CV_32SC1);
    cvIntegral(gray,sum);

    //std::cout<<inImg->height + 1<<","<<inImg->width + 1<<std::endl;
    //cv::imshow("sum",sum);
    //cv::imshow("inimg",inImg);

    CvPoint topRight, butLeft; //hard-coded.. defining roi for the region that tank is..
    topRight.x = 200;butLeft.x = 700;
    topRight.y = 20; butLeft.y = 520;
    int xGridSize, yGridSize;
    xGridSize = yGridSize = gridSize;

    int currX = topRight.x;
    int currY = topRight.y;
    //sum = pt4-pt3-pt2+pt1 using integral image ... check http://en.wikipedia.org/wiki/Haar-like_features
    // this should be changed a little bit.. so we want to have area of {(x,y) and (x+grid/2,y+grid)}- area of {(x+grid/2,y+grid) and (x+grid,y+grid)} 
    for (currX = topRight.x;currX<=(butLeft.x-xGridSize);currX+=xGridSize)
        for (currY=topRight.y;currY<=(butLeft.y-yGridSize);currY+=yGridSize)
        {
            int areaSum = CV_MAT_ELEM(*sum,int,currY+yGridSize,currX+xGridSize) - CV_MAT_ELEM(*sum,int,currY+yGridSize,currX)- CV_MAT_ELEM(*sum,int,currY,currX+xGridSize) + CV_MAT_ELEM(*sum,int,currY,currX);
            featureVec.push_back(areaSum);

        }
        //cvReleaseMat(&sum);
}



void drawArrow(IplImage* inImg, float* p)
{
    double centerX = 473.8111;
    double centerY = 236.0689;
    CvPoint pp,qq;
    pp = cvPoint(cvRound(p[0] ),cvRound(p[1] ));

    double tanValue = (centerY-p[1])/(p[0]-centerX);
    double angle = atan(tanValue);
    double arrowLength = 40;
    if (abs(abs(angle*180/pi) - 90.0)>2.0)
    {
        if ((pp.x>cvRound(centerX)))
            qq = cvPoint(cvRound(p[0] - arrowLength*sin(angle) ),cvRound(p[1]- arrowLength*cos(angle) ));
        else
            qq = cvPoint(cvRound(p[0] + arrowLength*sin(angle) ),cvRound(p[1]+ arrowLength*cos(angle) ));
    }
    else
    {
        //printf("\nabs(abs(angle*180/pi) - 90.0)<1.0)");
        if ((pp.y<cvRound(centerY)))

        {
            printf("\nif part");
            qq = cvPoint(cvRound(p[0] - arrowLength*1.0 ),cvRound(p[1]- arrowLength*0.0 ));
        }
        else
        {
            printf("\nelse part");
            qq = cvPoint(cvRound(p[0] + arrowLength*1.0 ),cvRound(p[1]+ arrowLength*0.0));
        }
    }
    int line_thickness; line_thickness = 2;
    CvScalar line_color; line_color = CV_RGB(255,255,0);
    cvLine( inImg, pp, qq, line_color, line_thickness, CV_AA, 0 );
    /* Now draw the tips of the arrow. I do some scaling so that the
    * tips look proportional to the main line of the arrow.
    */
    if (pp.x>cvRound(centerX))
    {
        pp.x = ( int) (qq.x + 9 * sin(pi / 4 + angle));
        pp.y = ( int) (qq.y + 9 * cos(pi / 4 + angle));
        cvLine( inImg, pp, qq, line_color, line_thickness, CV_AA, 0 );
        pp.x = ( int) (qq.x - 9 * sin(-angle + pi / 4));
        pp.y = ( int) (qq.y + 9 * cos(-angle + pi / 4));
        cvLine( inImg, pp, qq, line_color, line_thickness, CV_AA, 0 );
    }

    else
    {
        //cvWaitKey(-1);
        pp.x = ( int) (qq.x + 9 * cos(pi / 4 + angle));
        pp.y = ( int) (qq.y - 9 * sin(pi / 4 + angle));
        cvLine( inImg, pp, qq, line_color, line_thickness, CV_AA, 0 );

        pp.x = ( int) (qq.x - 9 * cos(-angle + pi / 4));
        pp.y = ( int) (qq.y - 9 * sin(-angle + pi / 4));
        cvLine( inImg, pp, qq, line_color, line_thickness, CV_AA, 0 );
    }
}


int main(int argc, char** argv)
{
    CvCapture *videoCapture;

    IplImage *capturedFrame,*gray, *myMask,*maskedFrame;//, *myTemplate;
    capturedFrame = gray = myMask = NULL;
    std::string inputVideoFileName,inputMaskFileName;
#if 0


    if (argc<3)
    {
        //inputMaskFileName = "C:\\Users\\reza\\beforeSeattleTrip\\1.1_GG-clip-2011-04-21 16;05;32_mask.bmp";
        //inputVideoFileName = "C:\\Users\\reza\\beforeSeattleTrip\\1.1_GG-clip-2011-04-21 16;05;32.avi";
        inputVideoFileName = "D:\\Peichel\\PJxP_Family1\\school_only\\1.1_GG-clip-2011-04-21 16;05;32.avi";
        inputMaskFileName = "D:\\Peichel\\PJxP_Family1\\school_only\\1.1_GG-clip-2011-04-21 16;05;32_mask.bmp";

    }
    else
    {
        inputVideoFileName = argv[1];
        inputMaskFileName = argv[2]; 
    }

#else
    std::string listOfFiles;
    if (argc<2)
    {
        std::cout<<"no input file list is provided.. it is gonna use the default"<<std::endl;
        listOfFiles = "D:\\Peichel\\PJxP_Family1\\videoinfo4of4_fullPath_schoolOnly1_50.csv";

        return (-1);
    }
    else
    {

    listOfFiles = argv[1];
    }
    std::ifstream myInputFile(listOfFiles.c_str());
    std::stringstream ss;
    char buff[2000];
    if(!myInputFile.is_open())
    {
        std::cout<<"\nproblem with the input list of file: "<<listOfFiles<<std::endl;

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
        //change the extension from .mp4 to .avi
        inputVideoFileName = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
        inputVideoFileName+=".avi";
        std::cout<<"in: "<<inputVideoFileName<<std::endl;
        inputMaskFileName = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
        inputMaskFileName+="_Mask.bmp";
        std::cout<<"msk: "<<inputMaskFileName<<std::endl; 

        std::string outputInfoFilename = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
        outputInfoFilename+="_centerOfCircle.txt";

#if __RECORD_VIDEO_OUTPUT
        std::string videoOutfn=inputVideoFileName.substr(0,inputVideoFileName.length()-4);
        videoOutfn+="_output_hough.avi";
#endif

        std::ofstream outputFeatures;
        std::string outFeatureName = inputVideoFileName.substr(0,inputVideoFileName.length()-4);
        int gridSize = 20;
        char str[100];
        sprintf(str,"%d",gridSize);
        outFeatureName +="_allFeatures_";
        outFeatureName +=str;
        outFeatureName +=".csv";


        myMask = cvLoadImage(inputMaskFileName.c_str(),0);
#if __NOMASK
        cvZero(myMask);
        cvNot(myMask,myMask);
#endif
        if (myMask == NULL)
        {
            printf("\nCan not open the maskfile:%s",inputMaskFileName.c_str());
            //std::cout<<inputMaskFileName<<std::endl;
            if (argc<2)
                getchar();
            else
            {
                continue; //going for the next file
            }
            printf("\nexiting..");
            return -1;
        }

        videoCapture = cvCreateFileCapture(inputVideoFileName.c_str());
        if (videoCapture == NULL)
        {
            printf("\nCan not open the video: %s",inputVideoFileName.c_str());
            //std::cout<<"vid:  "<<inputVideoFileName<<std::endl;
            if (argc<2)
                getchar();
            else
            {
                continue; //going for the next file
            }

            printf("\nexiting..");

            return -1;
        }
        //if every thing is fine create these files..
        outputFeatures.open(outFeatureName.c_str());

        FILE* outputTxtFile = fopen(outputInfoFilename.c_str(),"w");

        fprintf(outputTxtFile,"fn,x,y,r\n");

        int totalFrames = (int)cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_COUNT);
        int h = (int)cvGetCaptureProperty(videoCapture,CV_CAP_PROP_FRAME_HEIGHT);
        int w = (int)cvGetCaptureProperty(videoCapture,CV_CAP_PROP_FRAME_WIDTH);

        std::cout<<" h :" <<h << " w :"<< w << " total: " << totalFrames <<std::endl;

        //Allocate all needed memories..
        maskedFrame =  cvCreateImage(cvSize(w,h),8,3);
        cvZero(maskedFrame);
        gray = cvCreateImage(cvSize(w,h), 8, 1);
        CvMemStorage* storage = cvCreateMemStorage(0);
        //storage = NULL;
        CvMat* sum = cvCreateMat(h + 1, w + 1, CV_32SC1);

        clock_t startTime = clock();

        cvSetCaptureProperty(videoCapture,CV_CAP_PROP_POS_FRAMES,(double)startFrame);
        
        for(int fn = startFrame;fn<=stopFrame;fn++)
        {
            if (fn%100 == 0)
                printf("\nfn = %d",fn);
            capturedFrame = cvQueryFrame(videoCapture);
            if (capturedFrame == NULL)
            {
                printf("\n Can not read the next frame");
                break;
            }

            //cvShowImage("frame",capturedFrame);
            //cvWaitKey(1);
            cvZero(maskedFrame);
            cvCopy(capturedFrame,maskedFrame,myMask);
            //cvShowImage("maskedFrame",maskedFrame);

            cvCvtColor(maskedFrame, gray, CV_BGR2GRAY);
            //////---------------------first take out the features info---------------------------
            std::vector<int> featureVec;
            //returnHaarFeatures(maskedFrame,featureVec,gridSize);
#if !__JUST_EXTRACT_CENTER
            returnHaarFeatures(gray,sum,featureVec,gridSize);
            outputFeatures<<"\n"<<fn;
            for (int ii = 0;ii<featureVec.size();ii++)
                outputFeatures<<","<<featureVec[ii];
#endif
            //--------------------------------------------------------------------
            //if (storage!=NULL) //throws an error.. 
            //    cvReleaseMemStorage(&storage);
            CvSeq* circles = cvHoughCircles(gray, storage, 
                CV_HOUGH_GRADIENT, 2, gray->height, 10,10, 50, 70); //    CV_HOUGH_GRADIENT, 2, gray->height, 50,40, 50, 70); //worksss


            for (int crclCount = 0; crclCount < circles->total; crclCount++) 
            {
                float* p = (float*)cvGetSeqElem( circles, crclCount );
                fprintf(outputTxtFile,"%d,%.2lf,%.2lf,%.2lf\n",fn,p[0],p[1],p[2]);
#if (__SHOW_OUTPUT || __RECORD_VIDEO_OUTPUT)
                cvCircle( maskedFrame, cvPoint(cvRound(p[0]),cvRound(p[1])), 
                    3, CV_RGB(0,255,0), -1, 8, 0 );
                //cvCircle( img, cvPoint(cvRound(p[0]),cvRound(p[1])), 
                //    cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );
                cvCircle( maskedFrame, cvPoint(cvRound(p[0]),cvRound(p[1])), 
                    60, CV_RGB(255,0,0), 3, 8, 0 );
                drawArrow(maskedFrame,p);
#endif

            }

#if __SHOW_OUTPUT
            cvNamedWindow( "circles", 1 );
            cvShowImage( "circles", maskedFrame );
            cvWaitKey(1);
#endif
        }

        if (gray != NULL)
            cvReleaseImage(&gray);

        if (maskedFrame != NULL)
            cvReleaseImage(&maskedFrame);

        if (myMask != NULL)
            cvReleaseImage(&myMask);

        if (videoCapture != NULL)
            cvReleaseCapture(&videoCapture);

        fclose(outputTxtFile);
        outputFeatures.close();
        printf("\ntime Taken is %.2lf seconds", double(clock()-startTime)/CLOCKS_PER_SEC);

    }
    if (argc<2)
        getchar();

    return 0;
}
#endif
