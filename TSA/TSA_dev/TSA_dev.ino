 /* 
 Twisted String Actuator Controller
 
 Copyright 2017
 Stanford University
 Author: Simon Kalouche
 */
 
 
 #include <Encoder.h>
 #include <SPI.h>
 
 // Runtime Options
 bool DEBUGGER = true;    // if true python commands are NOT sent
 
 // Define Pin out
 #define encoder1PinA 21  
 #define encoder1PinB 22 // 22 
 #define encoder2PinA 20
 #define encoder2PinB 23 // 23
 #define motor1_A 2  // CW +
 #define motor1_B 3  
 #define motor2_A 5  // CCW +
 #define motor2_B 6  //   
 #define magEncPin 53
 #define pot1Pin 54   // analog pin 0
 #define pot2Pin 55   // analog pin 1
 #define fsr1Pin 56   // anolog pin 2
 #define fsr2Pin 57   // analog pin 3
 int CS_PIN = magEncPin;
 
 // Options and Parameters
 #define POT_MIN_VAL 10
 #define POT_MAX_VAL 1015
 #define BAUD_RATE 115200
 #define TIMEOUT 0  // timeout is for Serial.readString() timeout. If its large the control loop becomes very slow
 #define pauseTime 1  // (us microseconds) change this to increase feedback frequency on AMS5047 encoder
 #define PID_DELAY 500 // motor PD controller delay time in microseconds
 #define CPR 2000.0  // counts per turn for maxon motor encoders
 #define FSR_CALIBRATION_CONSTANT 2.0   //1.81
 #define MAX_FSR_FORCE 100 // max fsr force in N
 #define MAX_MAG_ENC_VAL 16383 //2^14 - 1
 #define MAX_NUM_TWISTS 150.0 // 144 (by hand), (115 @12V)
 #define MAX_PRELOAD 60.0
 #define Ks 10 // stiffness gain of twisted string actuator. stiffness of twisted string can be varied/tuned using potentiometer from: -(MAX_NUM_TWISTS)/(2*Ks) < 0 < (MAX_NUM_TWISTS)/(2*Ks). 
 // ...corresponds to the number of twists offset the actuator is from an ideal TSA where the antagonistic strings are perfectly inversed in length
 
 // Motor PD Gains
 #define kp 60.0    // at 12 V power supply to motor driver (40)
 #define kd -.10
 
 int POT_DEADZONE = 1;     // kp/2;
 float PRELOAD = 3.0;  // number of twists of preload
 
 // define motor encoder classes
 Encoder enc1(encoder1PinA, encoder1PinB);
 Encoder enc2(encoder2PinA, encoder2PinB);
 
 int pot1Val = 0, pot2Val = 0, enc1Val = 0, enc2Val = 0, m1_turns = 0, m2_turns = 0, mode = 1, motorSelect = 1, inbyte;
 float fbk_pos_1 = 0.0, fbk_pos_2 = 0.0, fbk_vel_1 = 0.0, fbk_vel_2 = 0.0, fbk_pos_1_last = 0.0, fbk_pos_2_last = 0.0, fbk_output_pos_last = 0.0;
 float cmd_pos1 = 0.0, cmd_pos2 = 0.0, motor1_pwm = 0.0, motor2_pwm = 0.0, dt = 0.01;
 long timeNow = 0, timeLast = 0;
 unsigned long serialdata;
 int16_t current_reading; //current magnetic encoder reading with compensation
 bool error_flag = false;

 struct FBK{
   float pot1;  // 0-MAX_NUM_TWISTS
   float pot2;  // 0-MAX_NUM_TWISTS
   float fsr1;  // 0-100 N
   float fsr2;  // 0-100 N
   float outputAngle;  // [deg]
   float outputRev;    // [revolutions]
   float enc1;  // raw num encoder counts, resets to 0 with every revolution
   float enc2;  // raw num encoder counts, resets to 0 with every revolution
   float m1Twists;  // number of twists or revolutions
   float m2Twists;  // number of twists or revolutions
  };
  
  struct CMD{
   float m1Pos;    // [revolutions]
   float m2Pos;    // [revolutions]
   float outputAngle;  // [deg]
   float motorKp;  
   float motorKd;
  };
  
  struct CMD cmdLast = {0,0,0,kp,kd};
  struct CMD cmd;


void setup() {
  // increase pwm frequency on motors to reduce audible noise 
  setPWMFrequency();
  
  // setup magnetic encoder
  error_flag = false;
  set_zero((1.0*12300)/MAX_MAG_ENC_VAL);  // instead of zero make this the analogvalue/(2^15 - 1) to set permanent zero
  
  // Setup motors, encoders, and potentiometer pins
  pinMode(magEncPin,OUTPUT);
  pinMode(pot1Pin, INPUT);
  pinMode(pot2Pin, INPUT);
  pinMode(fsr1Pin, INPUT);
  pinMode(fsr2Pin, INPUT);
  pinMode(encoder1PinA, INPUT_PULLUP);
  pinMode(encoder1PinB, INPUT_PULLUP);
  pinMode(encoder2PinA, INPUT_PULLUP);
  pinMode(encoder2PinB, INPUT_PULLUP);
  pinMode(motor1_A, OUTPUT);
  pinMode(motor1_B, OUTPUT);
  pinMode(motor2_A, OUTPUT);
  pinMode(motor2_B, OUTPUT);
  
  // SPI setup for AMS5047 encoder
  SPI.beginTransaction(SPISettings(9000000,MSBFIRST,SPI_MODE1));
  SPI.begin();
  
  // Serial setup
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(TIMEOUT);  // sets timeout delay time for Serial.readString(). If this is not set the default is 1000 ms which makes the control loop very slow
  delay(100);

}



void loop() {
  long tic = millis();
  delayMicroseconds(pauseTime);   
  
  // get feedback (sensors)
  struct FBK fbk = getAllFeedback();  

  // get commands from Python or Serial Window (communications)
  cmd = readCommandsFromSerial(cmdLast);
   
  // motor controller  
  cmd = motorControl(fbk, cmd);

  // publish feedback to Serial/Python (communications)
  send2Python(fbk, cmd);
  
  // publish for debugging  (communications)
//  debugging(fbk, cmd);
  
  cmdLast = cmd;
//  Serial.println(1000.0/(millis() - tic));  // frequency

}





