 /* 
 Various motor control algorithms and checks
 
 Copyright 2017
 Stanford University
 Author: Simon Kalouche
 */

 struct CMD motorControl(struct FBK fbk, struct CMD cmd) {
   /* INSTRUCTIONS FOR ALL MODES:
     1) Both potentiometers should start turned CCW all the way
     2) Both strings should have 0 twists upon start
    */ 
    
   if (mode == 0) {
     // unwind/wind individual strings in mode 0
     if (motorSelect == 1) {
       // control motor 1
       motor1_pwm = map(fbk.pot1,0, MAX_NUM_TWISTS, 0, 255)*sign(fbk.pot2 - MAX_NUM_TWISTS/2.0); 
       motor2_pwm = 0; }
     else if (motorSelect == 2) {
       // control motor 2
       motor1_pwm = 0; 
       motor2_pwm = map(fbk.pot1,0, MAX_NUM_TWISTS, 0, 255)*sign(fbk.pot2 - MAX_NUM_TWISTS/2.0); }}
   
   else if (mode == 1) {
     // individual potentiometer manual control
     cmd_pos1 = fbk.pot1;
     cmd_pos2 = fbk.pot2; }
     
   else if (mode == 2) {
     // potentiometer 1 output position control with tuneable stiffness
     PRELOAD = map(fbk.pot2, 0, MAX_NUM_TWISTS, 0, MAX_PRELOAD*100.0)/100.0;
     cmd_pos1 = MAX_NUM_TWISTS - fbk.pot1 + PRELOAD/2.0;
     cmd_pos2 = fbk.pot1 + PRELOAD/2.0; }
     
   else if (mode == 3) {
     // potentiometer 1 output stiffness control
     cmd_pos1 = fbk.pot1;
     cmd_pos2 = (MAX_NUM_TWISTS - fbk.pot1) + (fbk.pot2 - MAX_NUM_TWISTS/2.0)/Ks;}
     
   else if (mode == 4) {
     // commands motor positions from Python: ex) "c50.0,35.4,180.0,.3,.1"
     cmd_pos1 = cmd.m1Pos;
     cmd_pos2 = cmd.m2Pos; }
       
   else if (mode == 5) {
     // output position control mode
     // add twisted string FK model here
     float pos_kp = 1000.0;
     float pos_kd = 0;
     float rev_cmd = map(fbk.pot1,0, MAX_NUM_TWISTS, 0, 1000)/1000.0;
     Serial.print(rev_cmd);
     Serial.print("         ");
     motor1_pwm = pos_kp*(fbk.outputRev - rev_cmd) + pos_kd*(fbk.outputRev - fbk_output_pos_last)/dt;
     motor2_pwm = -pos_kp*(fbk.outputRev - rev_cmd) - pos_kd*(fbk.outputRev - fbk_output_pos_last)/dt;
       
     // saftey check that motors are only winding up in one direction
     motor1_pwm, motor2_pwm = motorCheck(fbk, motor1_pwm, motor2_pwm); }
     
   else if (mode == 6) {
     // force control mode
     float force_kp = 80.0;
     float coactivation = map(fbk.pot1,0, MAX_NUM_TWISTS, 0.5, 25);
     
     motor1_pwm = force_kp*(fbk.fsr1 - coactivation);   // unwinds string when force fbk > force command (i.e. coactivation) --> (output turns CCW)
     motor2_pwm = force_kp*(fbk.fsr2 - coactivation);   // unwinds string when force fbk > force command (i.e. coactivation) --> (output turns CW)
     
     // saftey check that motors are only winding up in one direction
     motor1_pwm, motor2_pwm = motorCheck(fbk, motor1_pwm, motor2_pwm);
     Serial.println(motor1_pwm);}
   
   else if (mode == 7) {
     // position control using force
     float force_kp = 80.0;
     float pos_kp = 15.0;
     float coactivation = map(fbk.pot2,0, MAX_NUM_TWISTS, 0.5, 25);
     float rev_cmd = map(fbk.pot1,0, MAX_NUM_TWISTS,0, 1000)/1000.0;
     
     // make position a function of string tension user inner position loop and outer force loop. 
     // To rotate output clockwise --> string1_tension > string2_tension and vice versa
     
     // to turn CCW using force  - (fbk.outputRev - rev_cmd) < 0
     motor1_pwm = force_kp*(fbk.fsr1 - (coactivation - pos_kp*(fbk.outputRev - rev_cmd)));  // unwinds string when force fbk > (...) --> (output turns CCW)
     
     // to turn CCW using force + (fbk.outputRev - rev_cmd) < 0
     motor2_pwm = force_kp*(fbk.fsr2 - (coactivation + pos_kp*(fbk.outputRev - rev_cmd))); 
     
//     motor1_pwm, motor2_pwm = motorCheck(fbk, motor1_pwm, motor2_pwm);  
     }
   
   else if (mode == 8) {
     // output position control mode with force
     // add twisted string FK model here
     // pot1 controls position, pot2 controls coactivation
     float pos_kp = 1000.0;
     float pos_kd = 0;
     float rev_cmd = map(fbk.pot1,0, MAX_NUM_TWISTS, 0, 1000)/1000.0;
        
     float force_kp = 80.0;
     float coactivation = map(fbk.pot2,0, MAX_NUM_TWISTS, 0.5, 25);
     
     int s1 = sign(fbk.m1Twists), s2 = sign(fbk.m2Twists);
     s1 = 1; s2 = 1;

     motor1_pwm = s1*(pos_kp*(fbk.outputRev - rev_cmd) + force_kp*(fbk.fsr1 - coactivation));
     motor2_pwm = -s2*(pos_kp*(fbk.outputRev - rev_cmd) + force_kp*(fbk.fsr2 - coactivation));
     Serial.println(rev_cmd);
       
     // saftey check that motors are only winding up in one direction
     motor1_pwm, motor2_pwm = motorCheck(fbk, motor1_pwm, motor2_pwm);
     
   }
   
   if (mode == 1 || mode == 2 || mode == 3 || mode == 4) {
     // get feedback for number of twists in each string  (modes 1-4)
     fbk_pos_1 = fbk.m1Twists;
     fbk_pos_2 = fbk.m2Twists;

     // calculate velocity (modes 1-4)
     fbk_vel_1 = (fbk_pos_1 - fbk_pos_1_last)/dt;
     fbk_vel_2 = (fbk_pos_2 - fbk_pos_2_last)/dt;
     
     // normal PID controller for modes 1-4
     motor1_pwm = cmd.motorKp*(fbk_pos_1 - cmd_pos1) + cmd.motorKd*(fbk_vel_1);
     motor2_pwm = cmd.motorKp*(fbk_pos_2 - cmd_pos2) + cmd.motorKd*(fbk_vel_2); }
     
   
   // send commands to motor
   motorDrive(1, motor1_pwm);
   motorDrive(2, motor2_pwm);
   
   // update parameters for next loop
   cmd.m1Pos = cmd_pos1;
   cmd.m2Pos = cmd_pos2;
   fbk_pos_1_last = fbk_pos_1;
   fbk_pos_2_last = fbk_pos_2;
   fbk_output_pos_last = fbk.outputRev;
   timeNow = millis();
   dt = timeNow - timeLast;
   timeLast = timeNow;
   
   delayMicroseconds(PID_DELAY); 
   return cmd;
 }

 
 
 
  float motorCheck(struct FBK fbk, float motor1pwm, float motor2pwm){
   
   if (fbk.m1Twists < 0.0) {
     // only want string to twist in positive direction
     motor1pwm = -150.0; }
   
   if (fbk.m2Twists < 0.0 ) {
     // only want string to twist in positive direction
     motor2pwm = -150.0; }
     
//   // handle case when string twists cross 0 
//   float total_twists = abs(fbk.m1Twists) + abs(fbk.m2Twists);
//   if (total_twists > MAX_NUM_TWISTS) {
//     motor1pwm = 0;
//     motor2pwm = 0; }
   
   return motor1pwm, motor2pwm;
 }


 void motorDrive (int motor_channel, int motor_pwm) {
  int pwm_out = min(abs((int)motor_pwm),255);
  if (abs(pwm_out) < POT_DEADZONE ){
    pwm_out = 0; }
   
  if (motor_channel == 1) {
    /* conditional operators for motor_channel 1 and 2 are reversed because 
       string windup direction should be different to match the direction the steel pulley
       cable is wound on each side of the pulley */
    if (motor_pwm > 0){
      analogWrite(motor1_A, pwm_out);
      analogWrite(motor1_B, LOW); }
    else if (motor_pwm < 0) {
      analogWrite(motor1_A, LOW);
      analogWrite(motor1_B, pwm_out); }
    else {
      analogWrite(motor1_A, LOW);
      analogWrite(motor1_B, LOW); } 
  }
  
  if (motor_channel == 2) {
    if (motor_pwm < 0){
      analogWrite(motor2_A, pwm_out);
      analogWrite(motor2_B, LOW); }
    else if (motor_pwm > 0) {
      analogWrite(motor2_A, LOW);
      analogWrite(motor2_B, pwm_out); }
    else {
      analogWrite(motor2_A, LOW);
      analogWrite(motor2_B, LOW); }
  }
 }
 
 
 void setPWMFrequency(){
  // Change PWM frequency to reduce motor noise (don't use pins running off timer0 on Mega b/c timer0 is used for micros() and millis() )
  //-- Set PWM frequency for D2, D3 & D5 -- 
  //TCCR3B = TCCR3B & B11111000 | B00000001;    // set timer 3 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR3B = TCCR3B & B11111000 | B00000010;    // set timer 3 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR3B = TCCR3B & B11111000 | B00000011;    // set timer 3 divisor to    64 for PWM frequency of   490.20 Hz
  
  //-- Set PWM frequency for D6, D7 & D8 --
  //TCCR4B = TCCR4B & B11111000 | B00000001;    // set timer 4 divisor to     1 for PWM frequency of 31372.55 Hz
  TCCR4B = TCCR4B & B11111000 | B00000010;    // set timer 4 divisor to     8 for PWM frequency of  3921.16 Hz
   
   
 }
