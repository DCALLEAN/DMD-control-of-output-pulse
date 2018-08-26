/*
	This sample is provided as-is, without any warranty.

	Please always consult the ALP-4 high-speed API description when
	customizing this program. It contains a detailled specification
	of all Alp... functions.

	(c) 2007 ViALUX GmbH. All rights reserved.
*/

// BlackWhiteSample.cpp : Defines the entry point for the console application.
//



//#include "stdafx.h"
#include <TCHAR.h>
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include<vector>
#include<iostream>
#include<io.h>
#include "alp.h"
#include <cv.h>
#include<opencv2\opencv.hpp>
#include <opencv2\contrib\contrib.hpp>  
//#include <highgui.h>

using namespace std ;
using namespace cv;


BOOL EnableSeqSynch( ALP_ID const nDevId, long &nPulseWidth /* micro seconds */, long nPolarity = ALP_LEVEL_HIGH /* or ALP_LEVEL_LOW */ );

//int _tmain(int /*argc*/, _TCHAR** /* *argv[]*/)
int _tmain(int argc, _TCHAR*  argv[])  // 1: image path,  2: CCD frame rate; 3: picture number  
{
	ALP_ID nDevId, nSeqId;
	long nDmdType, nSizeX, nSizeY;
	long nReturn;
	PCHAR imagefolder = argv[1];         // image folder path;
	//PCHAR imagefolder="D:\\dc\\randomPat\\randomPat_bin8";
	//PCHAR imagefolder="C:\\Users\\bbnc\\Desktop\\新建文件夹 (3)";
	char filename[100];
	long FrameRate = atol(argv[2]);    
	long Picnum=atol(argv[3]);             // picture number
	//long Picnum=100; 

	long nPictureTime=1000000/FrameRate/Picnum;// exposure  time 
	//long nPictureTime = 50000;	// 20 ms, i.e. 50 Hz frame rate
	
	const long nPicTimeMin=50, nPicTimeMax=100000;
	UCHAR *pImageData = NULL;

	// Allocate the ALP high-speed device
	if (ALP_OK != AlpDevAlloc( ALP_DEFAULT, ALP_DEFAULT, &nDevId )) return 1;

	// Inquire DMD type
	if (ALP_OK != AlpDevInquire( nDevId, ALP_DEV_DMDTYPE, &nDmdType )) return 1;
	switch (nDmdType) {
	case ALP_DMDTYPE_XGA_055A :
	case ALP_DMDTYPE_XGA_055X :
	case ALP_DMDTYPE_XGA_07A :
		nSizeX = 1024; nSizeY = 768;
		break;
	case ALP_DMDTYPE_DISCONNECT :
	case ALP_DMDTYPE_1080P_095A :
		nSizeX = 1920; nSizeY = 1080;
		break;
	case ALP_DMDTYPE_WUXGA_096A :
		nSizeX = 1920; nSizeY = 1200;
		break;
	default:
		// unsupported DMD type
		return 1;
	}

	// Allocate a sequence of two binary frames
	if (ALP_OK != AlpSeqAlloc( nDevId, 1, Picnum, &nSeqId )) return 1;

	 // Create images
	/*
	pImageData = (UCHAR*) malloc( 2*nSizeX*nSizeY );
	if (NULL == pImageData) return 1;
	FillMemory( pImageData, nSizeX*nSizeY, 0x80 );				// white
	FillMemory( pImageData+nSizeX*nSizeY, nSizeX*nSizeY, 0x00 );		// black
	*/

	// load the image to the buffer ----dc
	//vector<string> image_list;
	//*
	cout<<"Start loading the projection images"<<endl;


	Directory dir;  
    vector<string> fileNames = dir.GetListFiles(imagefolder, "*.png", true);
	pImageData = (UCHAR*) malloc( Picnum*nSizeX*nSizeY ); // allocate the memory
	vector<Mat> ImageData;

	for (int i=0; i<Picnum; i++)
	{
		string fileName = fileNames[i];
		string fileFullName = imagefolder+fileName;
		cout<<"File Name:"<<fileName<<endl;
		cout<<"Full path:"<<fileFullName<<endl;
        
		// load image
		Mat image=imread(fileName,IMREAD_UNCHANGED);
		//imshow("image",image);
		//(1000000);
		//imshow("image",image);
		if (!image.data)
		{
			cout << "Something wrong with the iamge !" << endl;
			return -1;
		}

		
		//ImageData.push_back(image);
		//fread(pImageData + i* nSizeX*nSizeY, nSizeX*nSizeY, 1, image);
		memcpy( pImageData + i* nSizeX*nSizeY,  image.data, nSizeX*nSizeY );
		//FillMemory( pImageData + i* nSizeX*nSizeY,  nSizeX*nSizeY, &image );

	}
	//*/

	// Transmit images into ALP memory
	nReturn = AlpSeqPut( nDevId, nSeqId, 0, Picnum, pImageData );
	
	free( pImageData );
	if (ALP_OK != nReturn) return 1;

	long Pwidth = nPictureTime/10 ;
	long nSeqSynchPulseWidthUs(Pwidth);	// 5 milli seconds
	if (EnableSeqSynch(nDevId, nSeqSynchPulseWidthUs)) {
		_tprintf( _T("Enabled SeqSynch output, pulse width: %i 祍\r\n"), nSeqSynchPulseWidthUs );
	} else {
		_tprintf( _T("Could not enabled SeqSynch output\r\n"));
	}

	for (;;) {
		// Set up image timing
		// For highest frame rate, first switch to binary uninterrupted
		// mode (ALP_BIN_UNINTERRUPTED) by using AlpDevControl.
		_tprintf( _T("\nSet up timing: f=%i fps\r\n"), 1000000/nPictureTime );
		if (ALP_OK != AlpSeqTiming( nDevId, nSeqId, ALP_DEFAULT, nPictureTime,
			ALP_DEFAULT, ALP_DEFAULT, ALP_DEFAULT ) ) return 1;

		// Start sequence
		if (ALP_OK != AlpProjStartCont( nDevId, nSeqId )) return 1;

		// Wait for key stroke
		_tprintf( _T("+: Increase frequency\r\n-: Decrease frequency\r\nESC: quit") );
		switch (_gettch()) {
		case _T('+'):
			nPictureTime -= 5000;
			if (nPictureTime<nPicTimeMin) nPictureTime = nPicTimeMin;
			break;
		case _T('-'):
			nPictureTime += 5000;
			if (nPictureTime>nPicTimeMax) nPictureTime = nPicTimeMax;
			break;
		case VK_ESCAPE:
			AlpDevHalt( nDevId );	// asynchronously stop sequence
			AlpDevFree( nDevId );
			return 0;
		}
	}
}



BOOL EnableSeqSynch( ALP_ID const nDevId, long &nPulseWidth /* micro seconds */, long nPolarity )
{
	if (ALP_OK != AlpDevControl( nDevId, 2062, 0 )	// output steady state on GPIO5 in order to suppress glitches due to internal changes
		|| ALP_OK != AlpDevControl( nDevId, 2061, 1 )
		|| ALP_OK != AlpDevControl( nDevId, 2059, 1 )
		|| ALP_OK != AlpDevControl( nDevId, 2060, 2006 ))
		return FALSE;
	Sleep(1);
	if (ALP_OK != AlpDevControl( nDevId, 2061, nPulseWidth*100 )	// monoflop time [10ns]; not very accurate, but reproducable
		|| ALP_OK != AlpDevInquire( nDevId, 2061, &nPulseWidth )
		|| ALP_OK != AlpDevControl( nDevId, 2062, nPolarity==ALP_LEVEL_HIGH?4:5 ))	// enable monoflop output on Pin GPIO5
		return FALSE;
	nPulseWidth /= 100;
	return TRUE;
}


