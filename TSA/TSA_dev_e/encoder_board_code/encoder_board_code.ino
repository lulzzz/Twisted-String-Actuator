#include <Encoder.h>

// Define Pin out
#define encoder1PinA 2  
#define encoder1PinB 7 
#define encoder2PinA 3
#define encoder2PinB 4 
#define enc1SendPos 9
#define enc1SendNeg 10
#define enc2SendPos 11
#define enc2SendNeg 12
 
#define BAUD_RATE 115200
#define TIMEOUT 0  
#define incThresh 500    // make sure this number is the same in MAIN (on the MEGA)


// define motor encoder classes
Encoder enc1(encoder1PinA, encoder1PinB);
Encoder enc2(encoder2PinA, encoder2PinB);

// initialize variables
int enc1val = 0, enc2val = 0;
 
void setup() {
  pinMode(encoder1PinA, INPUT);
  pinMode(encoder1PinB, INPUT);
  pinMode(encoder2PinA, INPUT);
  pinMode(encoder2PinB, INPUT);
  pinMode(enc1SendPos, OUTPUT);
  pinMode(enc2SendPos, OUTPUT);
  pinMode(enc1SendNeg, OUTPUT);
  pinMode(enc2SendNeg, OUTPUT);
  
  enc1.write(0);
  enc2.write(0);
  
  // Serial setup
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(TIMEOUT);  // sets timeout delay time for Serial.readString(). If this is not set the default is 1000 ms which makes the control loop very slow
  delay(100);
}

void loop() {
  enc1val = enc1.read();
  enc2val = enc2.read(); 
  
//  Serial.print(enc1val);
//  Serial.print("          ");
//  Serial.println(enc2val);
  
  
  // send pulse for encoder 1
  if (enc1val >= incThresh){
    digitalWrite(enc1SendPos, HIGH);
    digitalWrite(enc1SendNeg, LOW);
    enc1.write(0);}
  else if (enc1val <= -incThresh) {
    digitalWrite(enc1SendPos, LOW);
    digitalWrite(enc1SendNeg, HIGH);
    enc1.write(0);}
  else {
    digitalWrite(enc1SendPos, LOW);
    digitalWrite(enc1SendNeg, LOW);}
    
  
  // send pulse for encoder 2
  if (enc2val >= incThresh){
    digitalWrite(enc2SendPos, HIGH);
    digitalWrite(enc2SendNeg, LOW);
    enc2.write(0);}
  else if (enc2val <= -incThresh) {
    digitalWrite(enc2SendPos, LOW);
    digitalWrite(enc2SendNeg, HIGH);
    enc2.write(0);}
  else {
    digitalWrite(enc2SendPos, LOW);
    digitalWrite(enc2SendNeg, LOW);}
    
    
    
  

  
}
