#include <Encoder.h>

/* Read Quadrature Encoder
* Connect Encoder to Pins encoder0PinA, encoder0PinB, and +5V.
*
* Sketch by max wolf / www.meso.net
* v. 0.1 - very basic functions - mw 20061220
*
*/  

// Encoder pinout 
//----red---2(5V)    1
//----------4        3(GND)
//----------6(A)     5
//----------8(B)     7
//----------10       9
 
 #define encoder0PinA 2  
 #define encoder0PinB 4  
 #define motor1_A 9
 #define motor1_B 10
 #define potPin 14
 #define CPT 2000.0
 #define kp .55    // at 13 V power supply to motor driver
 #define kd -.02
 
 Encoder enc0(encoder0PinA, encoder0PinB);
 
 int potVal = 0;
 int encVal = 0;
 int turns = 0;
 float fbk_pos = 0.0;
 float fbk_vel = 0.0;
 float fbk_pos_last = 0.0;
 float cmd_pos = 0.0;
 float motor_pwm = 0.0;
 float dt = 0.01;
 long timeNow = 0;
 long timeLast = 0;
 
 struct CMD_STRUCT{
   byte pos;
   byte vel;
   byte torq;
 };

 void setup() { 
   pinMode(potPin, INPUT);
   pinMode(encoder0PinA, INPUT);
   pinMode(encoder0PinB, INPUT);
   pinMode(motor1_A, OUTPUT);
   pinMode(motor1_B, OUTPUT);
//   attachInterrupt(0, readEncoder, CHANGE);  // interrupt 0 = Arduino's pin 2, interrupt 1 = Arduino pin 3
   Serial.begin(115200);
 } 
 
 

 void loop() {
   potVal = analogRead(potPin);
//   Serial.println(potVal);
   cmd_pos = potVal/1022.0 * 360;

   encVal = enc0.read();
   fbk_pos = encVal/CPT * 360.0;
   if (encVal > CPT) {
     turns += 1; 
     enc0.write(0); }
   else if (encVal < -CPT) {
     turns -= 1; 
     enc0.write(0);}
   
   fbk_vel = (fbk_pos - fbk_pos_last)/dt;
   motor_pwm = kp*(fbk_pos - cmd_pos) + kd*(fbk_vel);
   
     
//   Serial.print("cmd_pos: ");
//   Serial.print(cmd_pos);
//   Serial.print("        fbk_pos: ");
//   Serial.print(fbk_pos);
//   Serial.print("        pwm: ");
//   Serial.println(motor_pwm);

  fbk_pos_last = fbk_pos;
  timeNow = millis();
  dt = timeNow - timeLast;
  timeLast = timeNow;
  
  motorDrive(1, motor_pwm);
   
  delayMicroseconds(700);   
 }
 
 
 
 void motorDrive (int motor_channel, int motor_vel) {
  
  if (motor_channel == 1) {
    if (motor_vel > 0){
      analogWrite(motor1_A, min(abs((int)motor_vel),255));
      analogWrite(motor1_B, LOW); }
    else if (motor_vel < 0) {
      analogWrite(motor1_A, LOW);
      analogWrite(motor1_B, min(abs((int)motor_vel),255)); }
    else {
      analogWrite(motor1_A, LOW);
      analogWrite(motor1_B, LOW); }  
  }
 }
 
 
 
 void positionController (int motor_channel) {
  CMD_STRUCT cmd = {0, 0, 0};
//  cmd = (CMD_STRICT){0, 0, 0};

   
   
 }



