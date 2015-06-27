//
//  HBComm - HB Communications Package 
//
//   Developed by Robert Noteboom for communications between PC and Arduino
//

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

int trace = 0;  // Set >0 to trace I/O

//
//  open_port() - Opens specified serial port
//
//    Input: name - device name of serial port to open   
//    Returns: the file descriptor on success or -1 on error
//
int open_port(char *name)
{
   int fd; /* File descriptor for the port */

   // Try to open device with following options
   fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1)
   {
    //  Could not open the port.
      perror("open_port: Unable to open device");
   }
   else
   {
      fcntl(fd, F_SETFL, 0);
   }
   return (fd);
}

//
//  close_port() - closes specified serial port
//
//    Input: the file descriptor of port to close
//
void close_port(int fd)
{
       close(fd);
}

//
//  write_port() - writes string to specified serial port
//
//    Inputs: 
//      fd - the file descriptor of port to close
//      string - null terminated string to write to port
//
int write_port(int fd, char *string)
{
   return write(fd, string, strlen(string));
}

//
//  read_port() - reads input from specified serial port until CR, NL, or end of input
//
//    Inputs: 
//      fd - the file descriptor of port to close
//      string - buffer to hold null terminated string read from port
//      nmax - maximum length of string
//
//    Returns:
//      number of bytes read
//
int read_port(int fd, char *buffer, int nmax)
{
   int bufidx;        /* Buffer index */
   int  nbytes;       /* Number of bytes read */

   // Ensure buffer is long enough to hold input
   if (nmax <=1)
   {
       printf("read_port error: buffer length of %d is too short to hold input\n",nmax);
       return (-1);
   }
   /* read characters into our string buffer until we get a CR or NL */
   bufidx = 0;
   while (1)
   {
      nbytes = read(fd, &buffer[bufidx], 1);
      if (nbytes > 0)
      {
         if (trace > 0) printf("(%c-%d)",buffer[bufidx],buffer[bufidx]);
         bufidx++;
         buffer[bufidx]='\0'; // null terminate the string
         if (buffer[bufidx-1] == '\n' || buffer[bufidx-1] == '\r') break;  // CR or NL
         if (bufidx >= (nmax-1)) break; // buffer full
      }
      else if (nbytes < 0)
      {
        printf("read_port error: error on read\n");
        return (-1);  // error on read
      }
      else
      {
         break;  // end of input
      }
   }
   return (bufidx);
}

//
//  flush_port() - flush port by reading until empty and then reading again
//                 until end of first record is found
//
//    Inputs: 
//      fd - the file descriptor of port to close
//
void flush_port(int fd)
{
   char buf;      /* Current char in buffer */
   int  nbytes;       /* Number of bytes read */

   printf("flushing buffer : ");

   /* read characters until none left */
   while (1)
   {
      nbytes = read(fd, &buf, 1);
      if (nbytes <= 0) break;
      else
      {
         printf("%c:",buf);
      }
   }
   printf("\nflushing complete\n");

   /* read characters into our string buffer until we get a CR or NL */
   printf("synchronizing buffer : ");
   while (1)
   {
      nbytes = read(fd, &buf, 1);
      if (nbytes > 0)
      {
         printf("%c",buf);
         if (buf == '\n' || buf == '\r') break;
      }
   }
   printf("\nsynchronization complete\n");
}

//
//   read_rec - read record from port
//
//    Inputs: 
//      fd - the file descriptor of port to close
//      string - buffer to hold null terminated string read from port
//      nmax - maximum length of string
//
//    Returns:
//      number of records read (all but last discarded)
//
int read_rec(int fd, char *nextrec, int nmax)
{
   int recnum = 0;
   int recnmax = 255;
   static char recbuf[255] = {0};  /* record buffer */
   static int recidx = 0;
   char buffer[255];         /* Input buffer */
   int nr;                   /* Number of bytes read */

   // Ensure buffer is long enough to hold input
   if (nmax <=1)
   {
       printf("read_rec error: record buffer length of %d is too short to hold input\n",nmax);
       return (-1);
   }
   recnum = 0;
   // Read input from port
   while (1) 
   {
      while (1)
      {
         buffer[0] = '\0'; // Initialize buffer
         nr = read_port(fd,buffer,255);
         if (nr > 0) 
         {
            // check for buffer overflow
            if ((recidx+nr) > (recnmax-1))
            {
               printf("buffer overflow\n");
               break;
            }
            // copy latest input to record buffer
            strncpy(&recbuf[recidx],buffer,nr);
            recidx = recidx + nr;
            recbuf[recidx] = '\0'; // null terminate the record
            if (trace > 0) printf("[%d: %s]\n",nr,buffer);
            // exit loop of end of record detected
            if ((recbuf[recidx-1] == '\n') || (recbuf[recidx-1] == '\r'))
            {
               recbuf[recidx-1] = '\0';
               if (recnum > 0) printf("Discarding buffer %d: %s\n",recnum,nextrec);
               strcpy(nextrec,recbuf);
               recnum++;
               recidx = 0;
               recbuf[0] = '\0';
               break;
            }
         }
         else if (nr < 0)
         {
            perror("main: read_port error\n");
         }
         else break; // no input read
      }
      if (nr <= 0) break; // error or no input
   }
   return recnum;
}

