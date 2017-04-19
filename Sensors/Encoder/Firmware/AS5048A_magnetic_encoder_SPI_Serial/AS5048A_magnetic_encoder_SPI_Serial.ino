/* AS5048A Angle Sensor Firmware

   MOSI : pin 11
   MISO:  pin 12
   SCK:   pin 13
   CS:    pin 10
   Vcc :  3.3V
   
   * Use yrs-859 crimper for encoder wires
   
   created 1/14/15
   by Karthic Palaniappan 
*/

/* SPI int16_terface for AS5047P (from datasheet)
 *  Max rate = 10MHz
 *  SPI Mode 1
 *  MSB first transfer
 *  Parity error checking
 */
 
//  Ability to add software zero (by pressing z at any point)

#include <SPI.h>

/* These registers are declared volatile because they can change unexpectedly.
 *  If declared as just unsigned, then compiler may cache in memory. volatile
 *  turns off compiler optimizations
 */
#define ERRFL (volatile unsigned word)0x0001
#define PROG (volatile unsigned word)0x0003
#define DIAAGC (volatile unsigned word)0x3ffc
#define MAG (volatile unsigned word)0x3ffd
#define ANGLEUNC (volatile unsigned word)0x3ffe
#define ANGLECOM (volatile unsigned word)0x3fff

#define MAX_SENSOR_VAL 16383 //2^14 - 1
#define pauseTime 0  // change this to increase feedback frequency
#define BAUD_RATE 115200 

int CS_PIN = 10;

bool error_flag = false;
//current zero_angle got from the zero reading set. 0 by default
double zero_angle;
//current sensor reading with compensation
int16_t current_reading;

/* This sensor operates with even parity. This means that if the the parity bit
 *  in a frame will be set according in order to make the number of ones in the
 *  frame an even number. So if frame has an even number of ones before the 
 *  parity bit is set, then parity bit is 0 but if the frame contained an odd
 *  number of ones before the parity bit is set, then parity bit is 1 to make
 *  the whole frame have even parity
 */
int16_t calculate_parity(word addr)
{
  int16_t popcount = 0;
  for(int16_t i=0;i<16;i++)
  {
    popcount += ((addr>>i) & 0x1);
  }
  return ( (popcount % 2 == 0) ? 0 : 1);
}

/* A typical write cycle is as follows (all frames are of size word/16 bits)
 *  1. Hold chip select n low i.e. assert chip select
 *  2. While holding chip select n low, write the address to write to as the 
 *     first frame. Remember a frame is of the format 
 *     parity bit | read/write | address (13 bits)
 *  3. Release chip select n
 *  4. Assert chip select n
 *  5. Write the data to write. Remember data is of the form
 *     parity | 0 (always low) | data (13 bits)
 *  6. Deassert chip select n
 */
void write_register(word addr,word data)
{
  int16_t parity = calculate_parity(addr);
  //Bitwise and with 0x3fff is just taking the lower 13 bits
  word transferWord = (parity<<15) | (addr & 0x3fff);
  parity = calculate_parity(data);
  word transferData = (parity<<15) | (data & 0x3fff);
  
  digitalWrite(CS_PIN,LOW);
  SPI.transfer16(transferWord);
  digitalWrite(CS_PIN,HIGH);
  
  digitalWrite(CS_PIN,LOW);
  SPI.transfer16(transferData);
  digitalWrite(CS_PIN,HIGH);
  
  digitalWrite(CS_PIN,LOW);
  word recievedWord = SPI.transfer16(0x0);
  digitalWrite(CS_PIN,HIGH);
  if((recievedWord & 0x3fff) != data)
  {
    Serial.println("Inconsistent write!!!");
  }
}


/* A typical read cycle is as follows (all frames are of size word/16 bits)
 *  1. Hold chip select n low i.e. assert chip select
 *  2. While holding chip select n low, write the address to read from as the 
 *     first frame. Remember a frame is of the format 
 *     parity bit | read/write (since read this is 1) | address (13 bits)
 *  3. Release chip select n
 *  4. Assert chip select n
 *  5. Write some junk data (typically nop (read from address 0)). This is done
 *     because the data from the register is available only in the next chip
 *     select n cycle
 *  6. Deassert chip select n
 */
word read_register(word addr)
{
  addr |= (1<<14);
  word transferWord = (calculate_parity(addr)<<15) | addr;
    
  digitalWrite(CS_PIN,LOW);
  SPI.transfer16(transferWord);
  digitalWrite(CS_PIN,HIGH);
  
  digitalWrite(CS_PIN,LOW);
  word receivedWord = SPI.transfer16(0x0);
  digitalWrite(CS_PIN,HIGH);
  
  return (receivedWord & 0x3fff) ;
}

double get_rotation()
{
  //magnetic field is too low so whatever we read is useless
  int16_t too_low = (read_register(DIAAGC)>>11) & 0x1;
  if(too_low)
    return 0;
  double raw_rotation =  ((current_reading/(MAX_SENSOR_VAL*1.0)) * 360.0);
  
  raw_rotation = raw_rotation - zero_angle;
  
  if (raw_rotation < 0.0) {
    raw_rotation += 360.0;
  }
    
  return raw_rotation;
  
}

//set the current_reading variable rather than return a value
void get_angle_with_compensation()
{
   current_reading = (int16_t)read_register(ANGLECOM);
}

int16_t get_angle_without_compensation()
{
   return (int16_t)read_register(ANGLEUNC);
}

word get_state()
{
  return read_register(DIAAGC);
}

void print_state()
{
  word data = read_register(ERRFL);
  //ERRFL register will set either bit 0/1/2 if error is found
  //Bitwise and with 3 will reveal if any of these bits have been set
  if (data & 0x3)
  {
    Serial.println("Error detected");
    error_flag = true;
  }
  else
  {
    error_flag = false;
  }
  Serial.print("State : ");
  Serial.print(read_register(DIAAGC),BIN);
  Serial.print("   ");
}

void set_zero(double zero)
{
  Serial.print("Set zero as ");
  Serial.println(zero);
  zero_angle = zero;
}

void setup() {
  error_flag = false;
  set_zero((1.0*12300)/MAX_SENSOR_VAL);  // instead of zero make this the analogvalue/(2^15 - 1) to set permanent zero
  pinMode(CS_PIN,OUTPUT);
  pinMode(CS_PIN-1,OUTPUT);
  pinMode(CS_PIN-2,OUTPUT);
  SPI.beginTransaction(SPISettings(9000000,MSBFIRST,SPI_MODE1));
  SPI.begin();
  Serial.begin(BAUD_RATE);
  delay(100);
}

void loop() 
{
  long tic = millis();
  delay(pauseTime); 
  int16_t val = get_angle_without_compensation();
  get_angle_with_compensation();
  
//  Serial.print("Without compensation = ");
//  Serial.print(val);
//  Serial.print("  With compensation = ");
//  Serial.print(current_reading);
//  Serial.print("  Feedback Frequency (Hz) = ");
//  Serial.print(1/((millis() - tic)/1000.0));
  
  double val3 = get_rotation();

  Serial.println(val3);

  if(Serial.available())
  {
    char incoming_byte = Serial.read();
    if((incoming_byte == 'z') || (incoming_byte == 'Z')) {
      // Ability to add software zero (by pressing z at any point)
      set_zero((current_reading/(MAX_SENSOR_VAL*1.0))*360.0); }
     
    else if(incoming_byte == '0') {
      // change chip select pin
      CS_PIN = 10; }
     
    else if(incoming_byte == '1') {
      // change chip select pin
      CS_PIN = 9; }
    
    else if(incoming_byte == '2') {
      // change chip select pin
      CS_PIN = 8; }
  }
}
