//
//  Serial Test Program - 
//
//   Developed by Robert Noteboom to test communications between PC and Arduino
//
//   Purpose: Test communications between PC and Hummerbot Arduino
//
//   Usage: HBCommtest_full [device: optional]

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>  /* Standard C library */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

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
   char nextrec[255];  /* record buffer */
   int fd;
   int nr;
   int recnum;
   int recnmax = 255;
   int procptr = 0;
   char inBuffer[255];
   char outBuffer[255];
   int nin;          // number of bytes in input record
   int delay = 100;        // delay between frames (in ms)

   int scmd = 90;     // Steering Command (0-180)
   int scmdstp = 10; // Steering Command Step
   int tcmd = 90;     // Throttle Command (0-180)
   int tcmdstp = 10; // Throttle Command Step

   int mode = 0;     //Current mode (1 - Auto; 0 - Manual)
   int scmdrc = 90;     // Steering Command (0-180)
   int tcmdrc = 90;     // Throttle Command (0-180)
   int tinave = 0;   // average frame time used in inner frame
   int tfusdmin = 0; // min frame time used
   int tfusdmax = 0; // max frame time used
   int toutave = 0;  // average frame time used in outer frame
   int nfover = 0;   // number of frame overruns
   int ninout = 0;   // number of inner frames so far in current outer frame
   int nread = 0;    // number of bytes read

   char devname[255];  // device name for comm test
   char devdef[] = {"/dev/ttyUSB0"};  // default device
   char bufnl[] = {"\n"};

   // Print Hummerbot Intro
   printf("\nHummerbot Communications Test Program\n\n");

   // Process inputs arguments to deterime I/O device to use
   if ( argc == 1 ) 
   {
      strcpy(devname,devdef);
   }
   else if ( argc == 2 ) 
   {
      delay = atoi(argv[1]);
      if (delay <= 1) delay = 1;
      strcpy(devname,devdef);
    }
   else if ( argc == 3 ) 
   {
      delay = atoi(argv[1]);
      if (delay <= 1) delay = 1;
      strcpy(devname,argv[2]);
   }
   else
   { 
      printf("Usage: %s delay(in ms) [device name]\n\n", *argv);
      return(1);
   }

   //  Print user instructions
   printf( "\nCommands for Hummerbot Communications Test Program\n"
           "Hot keys: \n"
           "\tCtrl-C     - quit the program\n");

   printf("IN: nr,mode,scmdrc,tcmdrc,tinave,tfusdmin,tfusdmax,toutave,nfover,ninout,nread\n");
   printf("OUT: scmd,tcmd\n\n");

   //  Open port for test and write a byte
   fd = open_port(devname);
   if (fd == -1)
   {
      printf("Error opening device : %s\n",devname);
      return(1);
   }
   nr = write_port(fd,bufnl);

   // Flush old input from port
   // Synchronize by reading until end of record is found
   flush_port(fd);

   // Read and print records from port
   recnum = 0;
   while (1)
   {
      // Prepare and write data record to Arduino
      scmd = scmd + scmdstp;
      if ( (scmd > 160) || (scmd < 20) ) scmdstp = -scmdstp;
      tcmd = tcmd + tcmdstp;
      if ( (tcmd > 160) || (tcmd < 20) ) tcmdstp = -tcmdstp;
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
      }
      usleep(delay*1000);

   }

   // close port when complete
   close_port(fd);
}
