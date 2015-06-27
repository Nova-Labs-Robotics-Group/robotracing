//*******************************************************************
// 
// Hummerbot Analysis Program
// 
//   Developed by Robert Noteboom
//
//   Purpose: Analyze video captured by Hummerbot to refine object
//            detection and control algorithms
//
//   Usage: HBAnalysis [filename|camera0|camera1]
//
//*******************************************************************

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "Hummerbot.h"

int main(int argc, char *argv[])
{
   char key = 0;        // key pressed
   bool active = true;  // active or paused
   int delayopts[] = {10, 25, 50, 100, 250, 500, 1000, 2500};  // options for delay between frames
   int delay = 2500;    // delay between frames (in ms) 
   bool next = false;   // process one frame


   // Print Hummerbot Intro
   printf("\nHummerbot Analysis Program\n\n");

   // Process inputs arguments to deterime if processing from file or camera
   if ( argc == 2 ) 
   {
      CaptureInit(argv[1]);
   }
   else
   { 
      printf("Usage: %s [filename|camera0|camera1]\n\n", *argv);
      exit(1);
   }

   //  Print user instructions
   printf( "\nCommands for Hummerbot Analysis\n"
           "Hot keys: \n"
           "\t<space> - toggle video processing\n"
	   "\t 1-8    - video speed\n"
	   "\t n      - next frame only\n"
           "\tESC     - quit the program\n");

   //
   //   Main Loop
   //      wait for key and process accordingly
   //        <space> - toggle between active processing and paused
   //        1-8     - select delay time between frames
   //        n       - process next frame only
   //        ESC     - exit program
   //      

   // loop until ESC key is pressed, ESC = 27
   while ( key != 27 )
   {
      key = cvWaitKey( delay );
      if ((key > '0') && (key < '9')) delay = delayopts[key-'1'];
      if (key == 'n') next = true;
      if (key == ' ') active = !active;
      if (active||next) ProcessImage(0);
      next = false;
   }  

  // Clean up before exiting
  cvReleaseCapture( &capture );
  cvReleaseImage( &hsv );
  cvReleaseImage( &hue_img );
  cvReleaseImage( &sat_img );
  cvReleaseImage( &hue_mask );
  cvReleaseImage( &sat_mask );
  cvReleaseImage( &mask );

}

