//*******************************************************************
// Global Data for Hummerbot
//
//   Developed by Robert Noteboom
//
//*******************************************************************
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "Hummerbot.h"

 CvCapture *capture = NULL;	// capture device
 CvVideoWriter* vwlog = NULL;      // video log file

 IplImage *img = NULL;		// copy camera image
 IplImage *capimg = NULL;	/// camera image

 IplImage *hsv = NULL;		// hsv image
 IplImage *hue_img = NULL;	// hue channel image
 IplImage *sat_img = NULL;	// sat channel image
 IplImage *mask = NULL;		// mask image	
 IplImage *hue_mask = NULL;	// hue mask image	
 IplImage *hue_mask1 = NULL;	// hue mask image	
 IplImage *hue_mask2 = NULL;	// hue mask image	
 IplImage *sat_mask = NULL;	// saturation mask image	
 IplImage *sh_mask = NULL;	// saturation mask image	
 IplImage *er_mask = NULL;	// saturation mask image	

 int frame = 0;  // frame counter

 int width, height; // width and height	of captured image
 bool draw_squares = true; //  indicator to draw squares around objects

 int hue_target = 10; // orange blob (by trial-and-error)
 int hue_range = 20; // orange blob (by trial-and-error)
 int hue_min = 2; // orange blob (by trial-and-error)
 int hue_max = 20; // orange blob (by trial-and-error)
 int sat_min = 120; // orange blob (by trial-and-error)
 int min_comp_size = 40;  // area of connected components must be greater then this
 int max_comp_size = 160; // area of connected components must be less then this
 int min_rect = 105;      // ratio of height/width connected components must greater than this in %
 int max_rect = 400;      // ratio of height/width connected components must less than this in %
 int horizon = 200;       // location of horizon
 int top_prct = 95;       // precent of bottom-horizon distance to top boundary

 int tcmd = STOP_SPEED;   // Throttle Command - <90 = stop; 180 = Full Throttle
 int scmd = STRAIGHT;     // Steerting Command - 0-90 left turn; 90-180 right turn
 int lastscmd = STRAIGHT; // store previous steering command
