 /* 
 Read and organize sensor data
 
 Copyright 2017
 Stanford University
 Author: Simon Kalouche
 */



float getOutputAngle() {
  int16_t val = get_angle_without_compensation();
  get_angle_with_compensation();
  return get_rotation();  
}

float getOutputRev() {
  int16_t val = get_angle_without_compensation();
  get_angle_with_compensation();
  return get_rotation_rev();  
}

float getFSRForce(int fsrPin) {
    float fsrVoltage;     // the analog reading converted to voltage
    float fsrResistance;  // The voltage converted to resistance, can be very big so make "long"
    float fsrConductance; 
    float fsrForce; 
    
    fsrVoltage = map(analogRead(fsrPin), 0, 1023, 0, 5000);
  
    // The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    // so FSR = ((Vcc - V) * R) / V        yay math!
    fsrResistance = 5000 - fsrVoltage;     // fsrVoltage is in millivolts so 5V = 5000mV
    fsrResistance *= 10000;                // 10K resistor
    fsrResistance /= fsrVoltage;
 
    fsrConductance = 1000000;           // we measure in micromhos so 
    fsrConductance /= fsrResistance;
    
    fsrForce = FSR_CALIBRATION_CONSTANT*fsrConductance / 80;  // in Newtons 
 
//    // Use the two FSR guide graphs to approximate the force
//    if (fsrConductance <= 1000) {
//      fsrForce = FSR_CALIBRATION_CONSTANT*fsrConductance / 80;  // in Newtons 
//    } else {
//      fsrForce = fsrConductance - 1000;
//      fsrForce /= 30; // in Newtons          
//    }
    
    return fsrForce; 
  }


struct FBK getAllFeedback() {
  
  struct FBK fbk;
  fbk.pot1 = map(analogRead(pot1Pin), POT_MIN_VAL, POT_MAX_VAL, 0., MAX_NUM_TWISTS*100)/100.0;
  fbk.pot2 = map(analogRead(pot2Pin), POT_MIN_VAL, POT_MAX_VAL, 0., MAX_NUM_TWISTS*100)/100.0;
  fbk.fsr1 = getFSRForce(fsr1Pin);
  fbk.fsr2 = getFSRForce(fsr2Pin);
  fbk.outputRev = getOutputRev();      // CW is positive rotation
  fbk.outputAngle = getOutputAngle();  // CW is positive rotation
//  fbk.enc1 = -enc1.read();   // CCW should be positive so that string rotation matches the twist direction of the steel cable
//  fbk.enc2 = enc2.read();    // CW should be positive so that string rotation matches the twist direction of the steel cable
  fbk.enc1 = -enc1Count;
  fbk.enc2 = enc2Count;  
  
  // keep track of number of full motor revolutions. Need to do this because Arduino runs out of memory when counting absolute rotation 
  // at approx. 2000 counts per revolution (CPR) and thousands of revolutions
  if (fbk.enc1 >= CPR) {
    m1_turns += 1;
    enc1Count = 0; 
    enc1.write(0); }
  else if (fbk.enc1 <= -CPR) {
    m1_turns -= 1;
    enc1Count = 0; 
    enc1.write(0);}
  
  if (fbk.enc2 >= CPR) {
    m2_turns += 1;
    enc2Count = 0; 
    enc2.write(0); }
  else if (fbk.enc2 <= -CPR) {
    m2_turns -= 1; 
    enc2Count = 0;
    enc2.write(0);} 
    
  fbk.m1Twists = m1_turns + fbk.enc1/CPR;  // units are number of turns. it can be any real number (i.e. not only whole numbers)
  fbk.m2Twists = m2_turns + fbk.enc2/CPR;  // units are number of turns. it can be any real number (i.e. not only whole numbers)
   
  
  return fbk;
}

void e1p() {
  enc1Count += incThresh;
}

void e1n() {
  enc1Count -= incThresh;
}

void e2p() {
  enc2Count += incThresh;
}

void e2n() {
  enc2Count -= incThresh;
}
  

void zeroEncoders() {
  enc1.write(0);
  enc2.write(0);
  enc1Count = 0;
  enc2Count = 0;
  m1_turns = 0;
  m2_turns = 0;  }
