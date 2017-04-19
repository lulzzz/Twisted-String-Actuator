// serial communications functions for Python and Serial terminal

  void send2Python(struct FBK fbk, struct CMD cmd){
      Serial.print(fbk.m1Twists);
      Serial.print(',');
      Serial.print(fbk.m2Twists);
      Serial.print(',');
      Serial.print(fbk.outputRev);
      Serial.print(',');
      Serial.print(fbk.fsr1);
      Serial.print(',');
      Serial.println(fbk.fsr2);  
  }
  
  void debugging(struct FBK fbk, struct CMD cmd){
    Serial.print(fbk.fsr1);
    Serial.print("         ");
    Serial.print(fbk.fsr2);
    Serial.print("         ");
    Serial.print(fbk.m1Twists);
    Serial.print("         ");
    Serial.print(fbk.m2Twists);
    Serial.print("         ");
  //  Serial.print(cmd.m1Pos);
  //  Serial.print("         ");
  //  Serial.print(cmd.m2Pos);
  //  Serial.print("         ");
    Serial.println(fbk.outputRev*100.0); }



  


struct CMD readCommandsFromSerial(struct CMD cmdLast){
  struct CMD cmd = cmdLast;
  
  if(Serial.available())
  {
    String incoming_string = Serial.readString();
    String first_char = incoming_string.substring(0,1);  // get the first character in the string from serial
    
    Serial.println(first_char);
    
    // use "" for strings and '' for characters
    if((first_char == "z") || (first_char == "Z")) {
      // Ability to add software zero (by pressing z at any point)
      set_zero((current_reading/(MAX_MAG_ENC_VAL*1.0))*360.0); }
      
    if((first_char == "r") || (first_char == "R")) {
      // RESET: untwist both strings all the way 
      // example command: r0 =>(untwists all the way), r100 =>(winds both strings up to 100 turns)
      mode = 4; 
      String twistVal = incoming_string.substring(1);
      cmd.m1Pos = twistVal.toInt(); 
      cmd.m2Pos = twistVal.toInt(); 
    }
      
      
    else if((first_char == "c") || (first_char == "C")) {
      // examples of commands from Python or Serial: "<motor1_pos [turns]>, <motor2_pos [turns]>, <output angle [deg -180 to 180]>, <Kp gain>, <Kd gain>"  
      // ex) "c120,230,90,0.5,0.6"
      
      // parse the command from Serial/Python
      int commaIndex1 = incoming_string.indexOf(',');
      int commaIndex2 = incoming_string.indexOf(',', commaIndex1+1);
      int commaIndex3 = incoming_string.indexOf(',', commaIndex2+1);
      int commaIndex4 = incoming_string.indexOf(',', commaIndex3+1);
      
      String val1 = incoming_string.substring(1,commaIndex1);
      String val2 = incoming_string.substring(commaIndex1 + 1, commaIndex2);
      String val3 = incoming_string.substring(commaIndex2 + 1, commaIndex3);
      String val4 = incoming_string.substring(commaIndex3 + 1, commaIndex4); 
      String val5 = incoming_string.substring(commaIndex4); // to end of the string
      
      cmd.m1Pos = val1.toInt();
      cmd.m2Pos = val2.toInt();
      cmd.outputAngle = val3.toInt();
      cmd.motorKp = val4.toInt();
      cmd.motorKd = val5.toInt();
      }
      
    else if((first_char == "m") || (first_char == "M")) {
      // examples of input for changing mode: "M1" or "M2" or "M3"
      String modeVal = incoming_string.substring(1,2);
      mode = modeVal.toInt(); 
      if (mode == 0) {
        String motorVal = incoming_string.substring(2,3);
        motorSelect = motorVal.toInt(); }
        
      set_zero((current_reading/(MAX_MAG_ENC_VAL*1.0))*360.0);  // reset zero position on output encoder
    }
   
  }
  
  return cmd;
  
}


