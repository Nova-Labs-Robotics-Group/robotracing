#include <Servo.h>

//
// Hummerbot - Main control loop for low level devices
//
Servo steering;  // servo instance
Servo throttle;  // servo instance
int pins = 9;          // steering pin
int pint = 10;         // throttle pin
int pina = 7;          // mode switch pin
int pinr = 6;          // record pin
int pinrcs = 2;        // steering RC command
int pinrct = 3;        // throttle RC command

int pulse;
int pulsemin = 1000;    // in microseconds -- 1.10 ms pulse -- 0 deg -- most clockwise
int pulsemax = 2000;   // in microseconds -- 1.90 ms pulse -- 180 deg -- most anticlockwise
int pulseto = 20000;    // in microseconds -- timeout at double maximum pulse width
int pulserange;
unsigned long duration;// pulse duration (RC)

int scmd = 0;    // steering command variables
int scmdrc = 0;
int scmdrc0 = 0;
int scmdrc1 = 0;
int scmdrc2 = 0;

int tcmd = 0;    // throttle command variables
int tcmdrc = 0;
int tcmdrc0 = 0;
int tcmdrc1 = 0;
int tcmdrc2 = 0;

int val = 0;           // variable for reading the vehicle mode
int mode = HIGH;       // variable for storing vehicle mode
int record = LOW;      // variable for storing record command

int incomingByte = 0;	// for incoming serial data
char buffer[40];
int bufmax = 30;
int bufferptr = 0;
int procptr = 0;
boolean complete = false;
boolean cmdvalid = false;
int nread = 0;
int i = 0;

int modeval = 11;
int scmdval = 22;
int tcmdval = 33;
int headcmp = 44;
int tframe = 55;

//
//   Time Management Variables
//
unsigned long tstart = 0;  // start of inner frame (servo commands)
unsigned long tused = 0;   // end of processing in inner frame (servo commands)
unsigned long tend = 0;    // end of inner frame (servo commands)
int frametim = 100;       // desired inner frame time (in ms)
int tdelay = 0;          // commanded delay to use remainder of frame
int tfusdmin = frametim; // min frame time used
int tfusdmax = 0;        // max frame time used
int tfusdtot = 0;        // total frame time used
int tinave = 25;           // average frame time used in inner frame
int nfover = 0;          // number of frame overruns
int ninout = 0;              // number of inner frames so far in current outer frame
int ninoutmax = 10;        // maximum number of inner frames per outer frame (timeout)
int touttot = 0;           // total time spent in outer frame (image processing)
int toutave = 25;           // average frame time used in outer frame

//
//   midval() - Determine middle value (used for command smoothing)
//
int midval(int val1, int val2, int val3)
{
   if (val1 > val2)
   {
      if (val1 > val3) 
      {
         if (val2 > val3) return val2;  // val3, val2, val1
         else return val3;  // val2, val3, val1
      }
      else return val1;  // val2, val1, val3
   }
   else
   {
      if (val2 > val3)
      {
         if (val1 > val3) return val1;  // val3, val1, val2
         else return val3;  // val1, val3, val2
      }
      else return val2; // val1, val2, val3  
   }
}

//
//   readRC() - reads PWM pulse and converts to 0 - 180 range
//
int readRC(int pin)
{
   duration = pulseIn(pin, HIGH, pulseto);
   if (duration == 0) duration = (pulsemax + pulsemin)/2;  // timeout - center servo
   return (min(max((duration - pulsemin) * 180 / pulserange,0),180));
}

void setup() 
{ 
  Serial.begin(9600); 
  
  // setup steering servo
  steering.attach(pins);
  //steering.attach(pins,pulsemin,pulsemax);
  
  // test steering
  delay(100);
  steering.write(90);    // center steering
  delay(500);            // wait 1/2 sec
  steering.write(180);   // full right
  delay(500);           // wait 2.5 sec
  steering.write(0);     // full left
  delay(500);           // wait 2.5 sec
  steering.write(90);    // recenter steering

  // setup throttle (ESC)
  throttle.attach(pint);
  
  // Setup mode and record indicator
  pinMode(pina, INPUT);     // declare picoswitch as input
  pinMode(pinr, INPUT);     // declare record switch as input
  
  // Initialize to Manual mode
  mode = HIGH;
  modeval = 0;
  
  // Initialize to Record on
  record = HIGH;

  // Setup RC contral signals
  pinMode(pinrcs, INPUT);     // declare RC Steering as input
  pinMode(pinrct, INPUT);     // declare RC Throttle as input
  pulserange = pulsemax - pulsemin;
  
  // initialize past steering and throttle commands
  tcmdrc1 = readRC(pinrct);
  tcmdrc0 = readRC(pinrct);
  scmdrc1 = readRC(pinrcs);
  scmdrc0 = readRC(pinrcs);
  
  // Initialize read of Asus to Arduino Data
  bufferptr = 0;
  complete = false;

  // Initialize time
  delay(100);
  tstart = millis();  // initialize frame time
}

void loop()
{
//
// Compute time used in last frame
//
  tused = millis();
  if (tused > tstart) tframe = (int)(tused - tstart);
  else tframe = tinave;
//
// Compute inner frame statistics (servo command processing)
//
  tfusdtot = tfusdtot + tframe;
  tfusdmin = min(tframe,tfusdmin);
  tfusdmax = max(tframe,tfusdmax);
//
// Computer frame delay and detect overruns
//
  tdelay = frametim - tframe;
  if (tdelay < 5)
  {
    tdelay = 5;
    nfover = nfover+1;
  }
  delay(tdelay);
//
// Compute total time of last frame
//
  tend = tstart;
  tstart = millis();   // start of this frame
  if (tend > tstart) tframe = (int)(tend - tstart);
  else tframe = frametim;
//
// Compute outer frame statistics (image processing)
//
  ninout = ninout + 1;
  touttot = touttot + tframe;
//
// Read Asus to Arduino Data
//
   while (Serial.available() > 0 && complete==false) {
	// read the incoming byte:
	incomingByte = Serial.read();
        nread++;
	// store in buffer
	buffer[bufferptr] = (char)incomingByte;
        if ((buffer[bufferptr] == '\n') || (buffer[bufferptr] == '!')) complete = true;
	else bufferptr = min(bufferptr+1,bufmax);
   }
//
// Process Asus to Arduino Data
//
   if (complete == true)
   {
        scmdval = 0;
        procptr = 0;
        while ((procptr < bufferptr) && (buffer[procptr] != ':'))
        {
           scmdval = scmdval*10 + (int)(buffer[procptr] - '0');
           procptr++;
        }
        procptr++;
        tcmdval = 0;
        while ((procptr < bufferptr) && (buffer[procptr] != ':'))
        {
           tcmdval = tcmdval*10 + (int)(buffer[procptr] - '0');
           procptr++;
        }
        if (procptr == bufferptr) cmdvalid = false;  // data read is not valid
        else cmdvalid = true;  // data read from Asus is valid
        bufferptr = 0;
   }
//
// Read data from RC Receiver
//
    //    steering command
    scmdrc2 = scmdrc1;
    scmdrc1 = scmdrc0;
    scmdrc0 = readRC(pinrcs);
    scmdrc = midval(scmdrc0,scmdrc1,scmdrc2); // pulseIn tends to be noisy - smooth data read from pulseIn
    // throttle command
    tcmdrc2 = tcmdrc1;
    tcmdrc1 = tcmdrc0;
    tcmdrc0 = readRC(pinrct);
    tcmdrc = midval(tcmdrc0,tcmdrc1,tcmdrc2);     // pulseIn tends to be noisy - smooth data read from pulseIn
//
// Read vehicle mode (HIGH = manual; LOW = autonomous)
//
  val = digitalRead(pina);      // read input value
  if (val == HIGH)              // check if the input is HIGH
  {
    if (mode != HIGH) modeval = 0; // Serial.println("MANUAL"); 
  } 
  else 
  {
    if (mode == HIGH) modeval = 1; // Serial.println("AUTO"); 
  }
  mode = val;
  record = digitalRead(pinr);   // read record indicator
  if ((record == HIGH)&&(mode == LOW)) modeval = 2;  // Record
//
//   Manual Mode
//
  if ((mode == HIGH)||(record == HIGH)) 
  {
    scmd = scmdrc;
    tcmd = tcmdrc;
  }
//
//    Autonomous Mode
//
  if ((mode == LOW)&&(record == LOW))
  {
//
//  Read commands from master processor
//     Speed (0-180), Turn (0-180)
//
      // only update if data is valid; don't change command otherwise
      if (cmdvalid)
      {
         tcmd = tcmdval;   // throttle command from Asus
         scmd = scmdval;   // steerting command from Asus
      }
  }
//
//  Write Steering and Throttle Commands to steering servo and motor ESC
//
   steering.write(scmd);    // write the pulse to steering servo
   throttle.write(tcmd);    // write the pulse to motor ESC   
//
//   Read digital compass
//
   headcmp = 0;
//
//   Detect end of outer frame
//
   if ((ninout >= ninoutmax) || cmdvalid)
   {
      // Compute time statistics
      if (ninout > 0) 
      {
         tinave = tfusdtot / ninout;
         toutave = touttot / ninout;
      }
      
      //   Write Arduino to Asus Data at end of outer frames (image processing)
      Serial.print(modeval);
      Serial.print(":");
      Serial.print(scmdrc);
      Serial.print(":");
      Serial.print(tcmdrc);
      Serial.print(":");
      Serial.print(tinave);
      Serial.print(":");
      Serial.print(tfusdmin);
      Serial.print(":");
      Serial.print(tfusdmax);
      Serial.print(":");
      Serial.print(toutave);
      Serial.print(":");
      Serial.print(nfover);
      Serial.print(":");
      Serial.print(ninout);
      Serial.print(":");
      Serial.print(nread);
      Serial.print(":");
      Serial.print("\n");
      
      // reset time statistics
      tfusdtot = 0;
      tfusdmin = frametim;
      tfusdmax = 0;
      nfover = 0;
      ninout = 0;
      touttot = 0;
      complete = false;
      cmdvalid = false;
   }
//
//  End of frame
//
}
