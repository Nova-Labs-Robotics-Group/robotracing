//*******************************************************************
//   CaptureInit
//
//     Developed by Robert Noteboom
//
//     Purpose: Initialize capture device and monitoring windows
//
//*******************************************************************

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "Hummerbot.h"

///////////////////////////////////////////////////
/* define a trackbar callback */
void on_trackbar(int h)
{
  ; /* do nothing */
}

void CaptureInit(char *filepath)
{

   // validate capture file or device

   if (strcmp(filepath,"camera")==0)
   {
      capture = cvCreateCameraCapture( CV_CAP_ANY );
   }
   else if (strcmp(filepath,"camera0")==0)
   {
      capture = cvCaptureFromCAM(0);
   } 
   else if (strcmp(filepath,"camera1")==0)
   {
      capture = cvCaptureFromCAM(1);
   } 
   else 
   {
      capture = cvCaptureFromAVI(filepath);
   }

   // verify capture device is valid
   if (!capture)
   {
      printf("Invalid capture device: %s\n",filepath);
      exit(1);
   }
   else
   {
      // get width and height of capture device */
      width = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH );
      height = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT );
      printf("Capture video from: %s : Width = %d : Height = %d\n",filepath,width,height);
      horizon = 0.6 * height;
   }

   //
   //  Create windows for analysis

   //     PREVIEW - captured video with annotations 
   cvNamedWindow( PREVIEW_WINDOW );
   cvMoveWindow( PREVIEW_WINDOW, 0, 50);

   //     MASK - binary image showing objects detected
   cvNamedWindow( MASK_WINDOW );
   cvMoveWindow( MASK_WINDOW, 0, 500);

   //     CONTROLS - controls used to adjust object det
   cvNamedWindow( CONTROLS_WINDOW );
    //  cvResizeWindow( CONTROLS_WINDOW, 300,380);
    cvCreateTrackbar( "Hue Tgt", CONTROLS_WINDOW, &hue_target, 180, on_trackbar );
    cvCreateTrackbar( "Hue Rng", CONTROLS_WINDOW, &hue_range, 90, on_trackbar );
    cvCreateTrackbar( "Min Sat", CONTROLS_WINDOW, &sat_min, 255, on_trackbar );
    cvCreateTrackbar( "Min Size", CONTROLS_WINDOW, &min_comp_size, 255, on_trackbar );
    cvCreateTrackbar( "Max Size", CONTROLS_WINDOW, &max_comp_size, 1000, on_trackbar );
    cvCreateTrackbar( "Min Rect", CONTROLS_WINDOW, &min_rect, 1000, on_trackbar );
    cvCreateTrackbar( "Max Rect", CONTROLS_WINDOW, &max_rect, 1000, on_trackbar );
    cvCreateTrackbar( "Horizon", CONTROLS_WINDOW, &horizon, height, on_trackbar );
    cvCreateTrackbar( "Top", CONTROLS_WINDOW, &top_prct, 100, on_trackbar );


    // Allocate memory for images
    hue_img = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    sat_img = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    hsv = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 3 );
    mask = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    hue_mask = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    hue_mask1 = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    hue_mask2 = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    sat_mask = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    sh_mask = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
    er_mask = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
}
