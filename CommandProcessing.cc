//*******************************************************************
//   Hummerbot Command Processing
// 
//   Developed by Robert Noteboom
//
//   Purpose: Use OpenCV to process images from Hummerbot and issue throttle and steering commands
//

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "Hummerbot.h"

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

//
//  Data structures holding detected cone info
//
struct cone
{
  int x;
  int w;
  int y;
  int h;
};
cone leftCone, rightCone;
cone farleftCone, farrightCone;
int compcount;
int compsize;
int compinview;
int compcones;
int compleftcones;
int comprightcones;

///////////////////////////

/////////////////////////////////////////////////////////////////////////
// perform connected components on mask image and draw on color_img
/////////////////////////////////////////////////////////////////////////
void FindCones( IplImage *color_img, IplImage *mask )
{  
  IplImage *src = cvCloneImage(mask); /* make copy of mask image */
  CvMemStorage* storage = cvCreateMemStorage(0);
  CvSeq* component = 0;
  int top_boundary;
  int middle_x;
  int bottom_boundary;
  int track_hyster;
  int steer_hyster;

  top_boundary = horizon * top_prct / 100;
  bottom_boundary = 0;
  middle_x = width/2;
  track_hyster = TRACK_HYSTER_FACT * width;
  steer_hyster = STEER_HYSTER_FACT * width;

  //////////////////////////////////////////////////////
  //Draw area-of-interest boundaries:
  // horizon
  cvLine( color_img, cvPoint(0, horizon), cvPoint(width, horizon), GREEN, 1, CV_AA, 0 );
  // upper boundary
  cvLine( color_img, cvPoint(0, top_boundary), cvPoint(width, top_boundary), BLUE, 1, CV_AA, 0 );
  // lower boundary
  cvLine( color_img, cvPoint(0, bottom_boundary), cvPoint(width, bottom_boundary), BLUE, 1, CV_AA, 0 );
  // vertical mid point
  cvLine( color_img, cvPoint(middle_x, top_boundary), cvPoint(middle_x, bottom_boundary), BLUE, 1, CV_AA, 0 );
  // left track boundary
  cvLine( color_img, cvPoint(middle_x - steer_hyster, top_boundary), cvPoint(middle_x - steer_hyster, bottom_boundary), BLUE, 1, CV_AA, 0 );
  // right track boundary
  cvLine( color_img, cvPoint(middle_x + steer_hyster, top_boundary), cvPoint(middle_x + steer_hyster, bottom_boundary), BLUE, 1, CV_AA, 0 );

  // draw max component size at top_boundary
  int wadj1  = max_comp_size * (horizon - top_boundary) / horizon;
  int hadj1  = max_comp_size * (horizon - top_boundary) / horizon;
  cvRectangle( color_img, cvPoint(middle_x+wadj1/2, top_boundary), cvPoint(middle_x-wadj1/2, top_boundary+hadj1), BLUE );	

  // draw min component size at top_boundary
  wadj1  = min_comp_size * (horizon - top_boundary) / horizon;
  hadj1  = min_comp_size * (horizon - top_boundary) / horizon;
  cvRectangle( color_img, cvPoint(middle_x+wadj1/2, top_boundary), cvPoint(middle_x-wadj1/2, top_boundary+hadj1), GREEN );	

  // draw max component size at bottom_boundary
  wadj1  = max_comp_size * (horizon - bottom_boundary) / horizon;
  hadj1  = max_comp_size * (horizon - bottom_boundary) / horizon;
  cvRectangle( color_img, cvPoint(middle_x+wadj1/2, bottom_boundary), cvPoint(middle_x-wadj1/2, bottom_boundary+hadj1), BLUE );	

  // draw min component size at bottom_boundary
  wadj1  = min_comp_size * (horizon - bottom_boundary) / horizon;
  hadj1  = min_comp_size * (horizon - bottom_boundary) / horizon;
  cvRectangle( color_img, cvPoint(middle_x+wadj1/2, bottom_boundary), cvPoint(middle_x-wadj1/2, bottom_boundary+hadj1), GREEN );	
  
  /////////////////////////////////////////////
  // Initialize cone statistics data
  leftCone.x = 0;
  leftCone.w = 0;
  leftCone.y = height;
  leftCone.h = 0;

  rightCone.x = 0;
  rightCone.w = 0;
  rightCone.y = height;
  rightCone.h = 0;

  farleftCone.x = 0;
  farleftCone.w = 0;
  farleftCone.y = 0;
  farleftCone.h = 0;

  farrightCone.x = 0;
  farrightCone.w = 0;
  farrightCone.y = 0;
  farrightCone.h = 0;

  compcount = 0;
  compinview = 0;
  compsize = 0;
  compcones = 0;
  compleftcones = 0;
  comprightcones = 0;

  /////////////////////////////////////////////
  // find all connected components
  cvFindContours( src, storage, &component, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

  // Check all components to see if they are cones
  for( ; component != 0; component = component->h_next )
  {
      CvRect comp_rect;  
      comp_rect = cvBoundingRect(component);
      compcount++;  // count components processed
 
      // ignore components outside of area-of-interest:
      if( (comp_rect.y+comp_rect.height/2 > top_boundary) || (comp_rect.y < bottom_boundary) ) continue;

      compinview++;  // count components in the area of interest

      // draw rectangle around component
      cvRectangle( color_img, cvPoint(comp_rect.x, comp_rect.y), cvPoint(comp_rect.x+comp_rect.width, comp_rect.y+comp_rect.height), BLUE );	

      // ignore components that are too small, large, or the wrong shape
      int wadj  = max(1,comp_rect.width * horizon / (horizon - comp_rect.y));
      int hadj  = max(1,comp_rect.height * horizon / (horizon - comp_rect.y));
      if ( (wadj < min_comp_size) || ( hadj < min_comp_size ) ) continue;  // too small
      if ( (wadj > max_comp_size) || ( hadj > max_comp_size ) ) continue;  // too large
      if ( (100*hadj/wadj < min_rect) || (100*hadj/wadj > max_rect ) ) continue;  // too rectangular

      compsize++;  // count components that are the right size and shape

      // draw rectangle around component
      cvRectangle( color_img, cvPoint(comp_rect.x, comp_rect.y), cvPoint(comp_rect.x+comp_rect.width, comp_rect.y+comp_rect.height), RED );	

      if(comp_rect.x+comp_rect.width/2 < middle_x - track_hyster)  // Cone is left of center
      {
         if(comp_rect.y < leftCone.y)  // Cone is closer than current left cone
         {
            leftCone.x = comp_rect.x;
            leftCone.w = comp_rect.width;
            leftCone.y = comp_rect.y;
            leftCone.h = comp_rect.height;
         }
         if(comp_rect.y > farleftCone.y)  // Cone is farther away than current far left cone
         {
            farleftCone.x = comp_rect.x;
            farleftCone.w = comp_rect.width;
            farleftCone.y = comp_rect.y;
            farleftCone.h = comp_rect.height;
         }
         compcones++;  // count cones found
         compleftcones++;
      }
      else if(comp_rect.x+comp_rect.width/2 >= (middle_x + track_hyster))  // Cone is right of center
      {
         if (comp_rect.y < rightCone.y)  // Cone is closer than current right cone
         {
            rightCone.x = comp_rect.x;
            rightCone.w = comp_rect.width;
            rightCone.y = comp_rect.y;
            rightCone.h = comp_rect.height;
         }
         if (comp_rect.y > farrightCone.y)  // Cone is farther away than current far right cone
         {
            farrightCone.x = comp_rect.x;
            farrightCone.w = comp_rect.width;
            farrightCone.y = comp_rect.y;
            farrightCone.h = comp_rect.height;
         }
         compcones++;  // count cones found
         comprightcones++;
      }
   }
   printf("N = %d: F = %d : V : %d : S = %d : C : %d : ",frame,compcount,compinview,compsize,compcones);
   // Cone detection processing complete
   /////////////////////////////////////

   /////////////////////////////////////
   // Determine steering commands

   // If inadeuate number of cones found -- Lost
   if ( (compcones < 2) ||  ((leftCone.y == height) && (rightCone.y == height)) )
   {
      tcmd = SLOW_SPEED;
      scmd = lastscmd;
      printf("Lost : continue with previous steering\n");
   }
   // only right cones found -- use direction of line from closest to farthest cone
   else if (leftCone.y == height)
   {
      if (rightCone.x > farrightCone.x)
      {
         scmd = SHARP_LEFT;
         tcmd = SLOW_SPEED;
         printf("No left cones : farthest cone more left : turn left\n");
      }
      else
      {
         scmd = SHARP_RIGHT;
         tcmd = SLOW_SPEED;
         printf("No left cones : farthest cone more right : turn right\n");
      }
   }
   // only left cones found -- use direction of line from closest to farthest cone
   else if (rightCone.y == height)
   {
      if (leftCone.x > farleftCone.x)
      {
         scmd = SHARP_LEFT;
         tcmd = SLOW_SPEED;
         printf("No right cones : farthest cone more left : turn left\n");
      }
      else
      {
         scmd = SHARP_RIGHT;
         tcmd = SLOW_SPEED;
         printf("No right cones : farthest cone more right : turn right\n");
      }
   }
   // check for left curve -- closest left cone is farther than farthest right cone (because it's really a right cone)
   else if ( leftCone.y > farrightCone.y )
   {
      scmd = SHARP_LEFT;
      tcmd = SLOW_SPEED;
      printf("In a left curve : Turn left\n");
   }
   // check for right curve -- closest right cone is farther than farthest left cone (because it's really a left cone)
   else if ( rightCone.y > farleftCone.y )
   {
      scmd = SHARP_RIGHT;
      tcmd = SLOW_SPEED;
      printf("In a right curve : Turn right\n");
   }
   // two good cones in center -- use left and right cones to determine steering commands
   else if ( (leftCone.y < height) && (rightCone.y < height) ) // found cones on both sides
   {
      // perspective adjustments
      int leftxp  = (leftCone.x+leftCone.w - middle_x) * horizon / (horizon - leftCone.y);
      int rightxp = (rightCone.x - middle_x) * horizon / (horizon - rightCone.y);
      int aimpoint = min(leftCone.y,rightCone.y);
      int midpoint = middle_x + ((leftxp + rightxp)/2) * (horizon - aimpoint)/horizon;
      printf("(%d,%d),(%d,%d) : ",leftxp,rightxp,midpoint,aimpoint);

      // draw selected cones
      cvRectangle( color_img, cvPoint(leftCone.x, leftCone.y), cvPoint(leftCone.x+leftCone.w, leftCone.y+leftCone.h), GREEN );	
      cvRectangle( color_img, cvPoint(rightCone.x, rightCone.y), cvPoint(rightCone.x+rightCone.w, rightCone.y+rightCone.h), GREEN );	

      // draw steering midpoint
      cvLine( color_img, cvPoint(midpoint, min(aimpoint+20,height)), cvPoint(midpoint, max(0,aimpoint-20)), GREEN, 1, CV_AA, 0 );

      if (midpoint > middle_x + steer_hyster) // center of track to right, steer right
      {
         scmd = SLIGHT_RIGHT;
         tcmd = SLOW_SPEED;
         printf("Adjust right\n");
      }
      else if (midpoint < middle_x - steer_hyster) // center of track to left, steer left
      {
         scmd = SLIGHT_LEFT;
         tcmd = SLOW_SPEED;
         printf("Adjust left\n");
      }
      else  // aligned with center of track go straigt fast
      {
         scmd = STRAIGHT;
         tcmd = SLOW_SPEED;
         printf("Go straight\n");
      }
   }
   else
   {
      printf("Error : should not get here\n");
   }

   lastscmd = scmd;
   cvReleaseImage(&src);

}  //END void FindCones( IplImage *color_img, IplImage *mask )
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// 
//    ProcessImage - Performs Image Processing
//
void ProcessImage(int mode)
{  

   // grab frame from camera
   if (capture==NULL) printf("no capture file\n");
   capimg = cvQueryFrame( capture );
   if ( capimg == NULL )
   {
      printf("No Image Captured\n");
      return;
   }
   else
   {
      if (vwlog && (mode !=0) ) cvWriteFrame( vwlog, capimg ); // Write the frame to video file if available
      img = cvCloneImage(capimg);  // clone image
      frame++;
   }
    
   //  Flip coordinate system and convert to HSV before processing
   cvFlip(img,img);
   cvCvtColor( img, hsv, CV_BGR2HSV );  // convert to HSV

   // Process Hue Channel
   cvSplit( hsv, hue_img, NULL, NULL, NULL ); // get hue channel
   // threshold hue image
   if (hue_target - hue_range < 0)
   {
      hue_min = 180 + (hue_target - hue_range);
      hue_max = hue_target + hue_range;
      cvInRangeS(hue_img, cvScalar( hue_min ), cvScalar( 180 ), hue_mask1 );
      cvInRangeS(hue_img, cvScalar( 0 ), cvScalar( hue_max ), hue_mask2 );
      cvOr(hue_mask1, hue_mask2, hue_mask);
   }
   else if (hue_target + hue_range > 180)
   {
      hue_min = hue_target - hue_range;
      hue_max = hue_target + hue_range - 180;
      cvInRangeS(hue_img, cvScalar( hue_min ), cvScalar( 180 ), hue_mask1 );
      cvInRangeS(hue_img, cvScalar( 0 ), cvScalar( hue_max ), hue_mask2 );
      cvOr(hue_mask1, hue_mask2, hue_mask);
   }
   else
   {
      hue_min = hue_target - hue_range;
      hue_max = hue_target + hue_range;
      cvInRangeS(hue_img, cvScalar( hue_min ), cvScalar( hue_max ), hue_mask );
   }

   // Process Sat Channel
   cvSplit( hsv, NULL, sat_img, NULL, NULL ); // get sat channel

   // threshold sat image
   cvInRangeS(sat_img, cvScalar( sat_min ), cvScalar( 255 ), sat_mask );

   // create mask from selected sat and hue data
   cvAnd(sat_mask, hue_mask, sh_mask);

   // Peform open function (erode and then dilate)
   cvErode(sh_mask, er_mask);
   cvDilate(er_mask, mask);

   // perform connected componenets to find cones and create steering commands
   FindCones(img, mask);		

   //  display image and mask
   cvFlip(img,img);
   cvShowImage( PREVIEW_WINDOW, img );  // image
   cvFlip(mask,mask);
   cvShowImage( MASK_WINDOW, mask );  // mask

   // release image memory
   cvReleaseImage( &img );

}  //END ProcessImage()
