//*******************************************************************
// Class Project
// CS682 - Spring 2009
// by Robert Noteboom
//
// Filename: Hummerbot.h
//
//*******************************************************************

#ifndef __HUMMERBOT_H
#define __HUMMERBOT_H

///////////////////////////////////////////
//CONSTANTS:
#define TRACK_HYSTER_FACT (0/16) //hysteresis factor (by trial-and-error) if we are in a curve or not
#define STEER_HYSTER_FACT (1/16) //hysteresis factor (by trial-and-error) if we are in a curve or not

#define FAST_SPEED 150  //arbitrary units (90 = stop : 180 = full throttle)
#define MED_SPEED 135
#define SLOW_SPEED 110
#define STOP_SPEED 90

#define SHARP_LEFT 135
#define SLIGHT_LEFT 110
#define STRAIGHT 90
#define SLIGHT_RIGHT 70
#define SHARP_RIGHT 45

#define RED CV_RGB(255,0,0)  //define the color "red"
#define GREEN CV_RGB(0,255,0)  //define the color "green"
#define BLUE CV_RGB(0,0,255)  //define the color "blue"

#define PREVIEW_WINDOW "Preview"
#define MASK_WINDOW "Mask"
#define CONTROLS_WINDOW "Controls"

///////////////////////////////////////////

extern CvCapture *capture;    // capture device
extern CvVideoWriter* vwlog;  // video log file

extern IplImage *img;		/* copy of camera image		*/
extern IplImage *capimg;	/* camera image		*/

extern IplImage *hsv;		/* hsv image		*/
extern IplImage *hue_img;	/* hue channel image*/
extern IplImage *sat_img;	/* sat channel image*/
extern IplImage *mask;		/* mask image		*/	
extern IplImage *hue_mask;		/* mask image		*/	
extern IplImage *hue_mask1;		/* mask image		*/	
extern IplImage *hue_mask2;		/* mask image		*/	
extern IplImage *sat_mask;		/* mask image		*/	
extern IplImage *sh_mask;		/* mask image		*/	
extern IplImage *er_mask;		/* mask image		*/	

extern int frame;           /* frame counter */

extern int width, height;			/* with and height	*/
extern bool draw_squares; 	/* draw squares		*/

extern int hue_min; /* hue min		*/
extern int hue_max; /* hue max		*/
extern int hue_range; /* trackbar hue min		*/
extern int hue_target; /* trackbar hue max		*/
extern int sat_min; /* trackbar sat min		*/
extern int min_comp_size;	/* side of connected components must be greater then this */
extern int max_comp_size;	/* side of connected components must be less then this */
extern int max_rect;	/* ratio of height/width of connected components must be less then this */
extern int min_rect;	/* ratio of height/width of connected components must be greater then this */
extern int horizon;     /* location of horizon (percent of image) */
extern int top_prct;    /* precent of bottom-horizon distance to top boundar */

extern int tcmd;   // Throttle Command - <90 = stop; 180 = Full Throttle
extern int scmd;   // Steerting Command - 0-90 left turn; 90-180 right turn
extern int lastscmd;   // Previous Steerting Command

void CaptureInit(char *filepath);
void ProcessImage(int mode);

#endif
