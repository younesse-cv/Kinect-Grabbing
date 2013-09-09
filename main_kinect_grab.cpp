/*Author: Younesse ANDAM 2011-2012*/
/* This is a simple project to grab the content (depth and RGB).*/
/*It may be useful for people who want to figure out how to display the data, and apply some 
operation on them*/ 
/*Enjoy*/

#include "XnCppWrapper.h"
#include <cv.h>
#include <highgui.h>
#include "stdio.h"
using namespace xn;

void main()
{
	//--------------------------------------------------- 
	bool bShouldRun = true;
	int c ;

	XnStatus nRetVal = XN_STATUS_OK;
	Context context;

	// Initialize context object
	nRetVal = context.Init();

	// check error code
	if(nRetVal)
		printf("Error : %s", xnGetStatusString(nRetVal));

	context.SetGlobalMirror(true); //mirror image

	// Create a DepthGenerator node
	DepthGenerator depth;
	nRetVal = depth.Create(context);
	// check error code
	if(nRetVal)
		printf("Failed to create depth generator: %s\n", xnGetStatusString(nRetVal));

	/// Create an ImageGenerator node
	ImageGenerator image;
	nRetVal = image.Create(context);
	if(nRetVal)
		printf("Failed to create image generator: %s\n", xnGetStatusString(nRetVal));

	if(nRetVal)
		printf("Failed to match Depth and RGB points of view: %s\n",xnGetStatusString(nRetVal));

	// Set it to VGA maps at 30 FPS
	XnMapOutputMode mapMode;
	mapMode.nXRes = XN_VGA_X_RES;
	mapMode.nYRes = XN_VGA_Y_RES;
	mapMode.nFPS = 30;
	nRetVal = depth.SetMapOutputMode(mapMode);

	// Make it start generating data
	nRetVal = context.StartGeneratingAll();

	//  check error code
	if(nRetVal)
		printf("Error : %s", xnGetStatusString(nRetVal));

	//create a OpenCv matrix
	CvMat* depthMetersMat = cvCreateMat(480, 640, CV_16UC1);
	IplImage *kinectDepthImage;
	kinectDepthImage = cvCreateImage( cvSize(640,480),16,1);
	IplImage *kinectDepthImage_raw= cvCreateImage( cvSize(640,480),16,1);

	IplImage rgbimg;

	XnPoint3D* pDepthPointSet = new XnPoint3D[ 640*480 ];


	// Main loop

	while (bShouldRun)
	{
		// Wait for new data to be available
		nRetVal = context.WaitOneUpdateAll(depth);

		if (nRetVal != XN_STATUS_OK)
		{
			printf("Failed updating data: %s\n", xnGetStatusString(nRetVal));
			continue;
		}

		// Take current depth map
		const XnDepthPixel* pDepthMap = depth.GetDepthMap();
		xn::DepthGenerator rDepth;

		//Copy the depth values
		for (int y=0; y<XN_VGA_Y_RES; y++)
			for(int x=0;x<XN_VGA_X_RES;x++)
			{
				depthMetersMat->data.s[y * XN_VGA_X_RES + x ] = 20*pDepthMap[y * XN_VGA_X_RES + x];

				// Convert the coordinates in the camera coordinate system
				pDepthPointSet[y * XN_VGA_X_RES + x].X = (XnFloat) x;
				pDepthPointSet[y * XN_VGA_X_RES + x].Y = (XnFloat) y;
				pDepthPointSet[y * XN_VGA_X_RES + x].Z = pDepthMap[y * XN_VGA_X_RES + x];

			}
		
			cvGetImage(depthMetersMat,kinectDepthImage_raw);
			cvShowImage("Depth stream", kinectDepthImage_raw);

			unsigned char* picture_RGB = new unsigned char[XN_VGA_X_RES * XN_VGA_Y_RES * 3];
			//initialization with the retrieved data
			memcpy(picture_RGB, (unsigned char*)image.GetRGB24ImageMap(),XN_VGA_Y_RES * XN_VGA_X_RES * 3);

			//From BGR to RGB
			for(int i = 0 ; i < XN_VGA_X_RES * XN_VGA_Y_RES ; i++)
			{   
				unsigned char temp = picture_RGB[i*3];
				picture_RGB[i*3] = picture_RGB[i*3+2];
				picture_RGB[i*3+2] = temp;
			}
			cv::Mat colorMatRes(XN_VGA_Y_RES, XN_VGA_X_RES, CV_8UC3, picture_RGB);
			rgbimg=colorMatRes; //Conversion from cv::mat to IplImage format
			cvShowImage("Color stream",&rgbimg); //Display the RGB stream

			// free memory 
			delete picture_RGB;
			c = cvWaitKey(1);
			if (c == 27)
				bShouldRun = false; //exit main loop

	}

	// Clean-up
	cvDestroyWindow("Color stream"); 
	cvDestroyWindow("Depth stream"); 
	cvReleaseImageHeader(&kinectDepthImage);
	delete pDepthPointSet;
	
}

