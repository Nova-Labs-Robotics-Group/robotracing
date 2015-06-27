//
//  Hummberbot Control Program Program - 
//
//   Developed by Robert Noteboom to drive Hummerbot
//
//   Usage: HBControl [video_device_name: optional] [serial_device_name: optional] [video_log: optional]
//
//   Note: must specify video device (file, camera0, camera1) to specify serial device
//

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "Hummerbot.h"

#define ESC 27

extern int open_port(char *name);
extern void close_port(int fd);
extern int write_port(int fd, char *string);
extern int read_port(int fd, char *buffer, int nmax);
extern void flush_port(int fd);
extern int read_rec(int fd, char *nextrec, int nmax);

//
//   Program to test communications between PC and Arduino
//
int main(int argc, char *argv[])
{
   char key = '\0';    // input from keyboard
   char nextrec[255];  // record buffer
   int fd;
   int nr;
   int recnum;
   int recnmax = 255;
   int procptr = 0;
   char inBuffer[255];
   char outBuffer[255];
   int nin;          // number of bytes in input record

   int mode = 0;     //Current mode (1 - Auto; 0 - Manual)
   int scmdrc = 90;  // Steering Command (0-180)
   int tcmdrc = 90;  // Throttle Command (0-180)
   int tinave = 0;   // average frame time used in inner frame
   int tfusdmin = 0; // min frame time used
   int tfusdmax = 0; // max frame time used
   int toutave = 0;  // average frame time used in outer frame
   int nfover = 0;   // number of frame overruns
   int ninout = 0;   // number of inner frames so far in current outer frame
   int nread = 0;    // number of bytes read

   char viddevname[255];
   char viddevdef[] = {"camera0"};
   char serdevname[255];
   char serdevdef[] = {"/dev/ttyUSB0"};
   char vidlogname[255];
   char bufnl[] = {"\n"};
   int fourcc = CV_FOURCC('X','V','I','D');
   int fps = 30;
   CvSize frame_size;

   // Allow Windows
   cvStartWindowThread();

   // Print Hummerbot Intro
   printf("\nHummerbot Control Program\n\n");

   //  Create video log file default name
   sprintf(vidlogname,"video%li.avi",time(NULL));

   // Process inputs arguments to deterime camera and I/O device to use
   if ( argc == 1 ) 
   {
       strcpy(viddevname,viddevdef);
       strcpy(serdevname,serdevdef);
   }
   else if ( argc == 2 ) 
   {
       strcpy(viddevname,argv[1]);
       strcpy(serdevname,serdevdef);
   }
   else if ( argc == 3 ) 
   {
       strcpy(viddevname,argv[1]);
       strcpy(serdevname,argv[2]);
   }
   else if ( argc == 4 ) 
   {
       strcpy(viddevname,argv[1]);
       strcpy(serdevname,argv[2]);
       strcpy(vidlogname,argv[3]);
   }
   else
   { 
      printf("Usage: %s [video_dev] [serial_dev] [video_log]\n\n", *argv);
      return(1);
   }

   // Initialize camera
   CaptureInit(viddevname);

   //  Open port for test and write a byte
   fd = open_port(serdevname);
   if (fd == -1)
   {
      printf("Error opening device : %s\n",serdevname);
      return(1);
   }
   nr = write_port(fd,bufnl);

   // Create video log file if approporate
   if ( (strcmp(viddevname,"camera")==0) || (strcmp(viddevname,"camera0")==0) || (strcmp(viddevname,"camera1")==0) )
   {
      printf("Creating video log: %s\n",vidlogname);
      frame_size = cvSize( width, height );
      vwlog = cvCreateVideoWriter(vidlogname, fourcc, fps, frame_size, 1);
      if (!vwlog) printf("Error creating video log file : %s : continue without log\n",vidlogname);
   }

   // Flush old input from port
   // Synchronize by reading until end of record is found
   flush_port(fd);

   // Wait Display frames until 's' is pressed */
	
   printf("Hit <space> to initiate active mode; ESC to exit\n");
   while ( key != ' ' )
   {
      ProcessImage(0);
      key = cvWaitKey( 200 );
      tcmd = STOP_SPEED;
      scmd = STRAIGHT;
      if (key == ESC) exit(1);
   }

   // Initialize steering and throttle commands
   tcmd = STOP_SPEED;
   scmd = STRAIGHT;

   // Main Loop for Control Program
   recnum = 0;
   while (key != ESC)
   {
      // Prepare and write data record to Arduino
      sprintf(outBuffer,"%d:%d:\n",scmd,tcmd);
      printf("OUT: %s",outBuffer);
      nr = write_port(fd,outBuffer);

      nextrec[0] = '\0'; // Initialize record buffer

      // Read and print record from port
      nr = read_rec(fd,inBuffer,recnmax);
      if (nr > 0) 
      {
         recnum++;
         nin = strlen(inBuffer);
         printf("IN Record %d: %d : {%s}\n",recnum,nin,inBuffer);

         // Process record read from Arduino
         procptr = 0; // buffer pointer
         //  Current mode (1 - Auto; 0 - Manual)
         mode = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) mode = mode*10 + (int)(inBuffer[procptr-1] - '0');
         //  Steering Command (0-180)
         scmdrc = 0;   
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) scmdrc = scmdrc*10 + (int)(inBuffer[procptr-1] - '0');
         //  Throttle Command (0-180)
         tcmdrc = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) tcmdrc = tcmdrc*10 + (int)(inBuffer[procptr-1] - '0');
         //  Average frame time used in inner frame (in ms)
         tinave = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) tinave = tinave*10 + (int)(inBuffer[procptr-1] - '0');
         // Min frame time used (in ms)
         tfusdmin = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) tfusdmin = tfusdmin*10 + (int)(inBuffer[procptr-1] - '0');
         // Max frame time used (in ms)
         tfusdmax = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) tfusdmax = tfusdmax*10 + (int)(inBuffer[procptr-1] - '0');
         // Average frame time used in outer frame (in ms)
         toutave = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) toutave = toutave*10 + (int)(inBuffer[procptr-1] - '0');
         // Number of frame overruns
         nfover = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) nfover = nfover*10 + (int)(inBuffer[procptr-1] - '0');
         // Number of inner frames so far in current outer frame
         ninout = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) ninout = ninout*10 + (int)(inBuffer[procptr-1] - '0');
         // Number of bytes read
         nread = 0;
         while ((procptr < nin) && (inBuffer[procptr++] != ':')) nread = nread*10 + (int)(inBuffer[procptr-1] - '0');
         // Print inputs
         printf("Processed: (%d) %d : %d : %d : %d : %d : %d : %d : %d : %d : %d \n\n",nr,mode,scmdrc,tcmdrc,tinave,tfusdmin,tfusdmax,toutave,nfover,ninout,nread);

         // Process Image From Camera
         ProcessImage(mode);
      }
      key = cvWaitKey( 100 );
   }

   // close port when complete
   close_port(fd);
}
