/* File: I2C_senor_rfboard 
 * Date: 10/23/19
 * Description: This program sets the ST25DV04K breakout board to energy harvesting mode. After running this program,
 * the eval board should be able to harvest energy and read/write to an NFC device/smartphone without 
 * autonomously without external power/microcontroller. 
 * 
 * After setting the EH mode of the device, this program also writes to the ST25DV04K mailbox.
 */

#include <i2c_t3.h>
#define EH_CTRL_DYN 0x2002    // dynamic energy harvesting I2C address. For use with DEV_SEL_ADDR_DYN.
#define EH_MODE_SYS 0x0002    // system energy harvesting I2C address. For use with DEV_SEL_ADDR_SYS.
#define I2C_SSO_DYN 0x2004    // dynamic I2C security session address. 
#define MAILBOX_I2C_START_ADDR 0x2008 // start address of the mailbox for I2C writing  
#define DEV_SEL_ADDR_DYN 0b1010011   // device select code to access user memory
#define DEV_SEL_ADDR_SYS 0b1010111   // device select code to access system area

bool EH_mode_set = false; 
  
// ----- 

byte read_dyn_register(uint16_t address) {

  Wire.beginTransmission(DEV_SEL_ADDR_DYN);    // select the device memory area
  Wire.write((address >> 8) & 0xff);    // address byte 1 MSB first  
  Wire.write(address & 0xff);    // address byte 2 LSB
  Wire.endTransmission(1);    // send bytes and keep connection active
  Wire.requestFrom(DEV_SEL_ADDR_DYN, 1);    // (2nd dev sel) request a byte from the address
  byte c = Wire.read();
  //Serial.println(c, BIN);

  if (c & 0x04) {    // if b2 = 1, then RF field is detected and communication is possible 
    digitalWrite(LED_BUILTIN, HIGH);
  } else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  return c;
}

byte read_sys_register(uint16_t address) {

  Wire.beginTransmission(DEV_SEL_ADDR_SYS);    // select the device memory area
  Wire.write((address >> 8) & 0xff);    // address byte 1 MSB first  
  Wire.write(address & 0xff);    // address byte 2 LSB
  Wire.endTransmission(1);    // send bytes and keep connection active
  Wire.requestFrom(DEV_SEL_ADDR_SYS, 1);    // (2nd dev sel) request a byte from the address
  byte c = Wire.read();
  //Serial.println(c, BIN);

  if (c & 0x04) {    // if b2 = 1, then RF field is detected and communication is possible 
    digitalWrite(LED_BUILTIN, HIGH);
  } else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  return c;
}

int16_t write_sys_register(uint16_t address, uint8_t value){
  
  Wire.beginTransmission(DEV_SEL_ADDR_SYS);    // dev select 
  Wire.write((address >> 8) & 0xff);    // address byte 1 MSB first  
  Wire.write(address & 0xff);    // address byte 2 LSB  
  Wire.write(value);    // write the value to the register
  Wire.endTransmission(1);    // stop

  return 0;
}

int16_t write_dyn_register(uint16_t address, uint8_t value){
  
  Wire.beginTransmission(DEV_SEL_ADDR_DYN);    // dev select 
  Wire.write((address >> 8) & 0xff);    // address byte 1 MSB first  
  Wire.write(address & 0xff);    // address byte 2 LSB  
  Wire.write(value);    // write the value to the register
  Wire.endTransmission(1);    // stop

  return 0;
}

// presents the default I2C password to open an I2C security session
int16_t open_I2C_security_session(){
  
  Serial.println("Initiating password present sequence.");
  
  Wire.beginTransmission(DEV_SEL_ADDR_SYS);    // dev select 
  Wire.write(0x09);    // Password address byte 1
  Wire.write(0x00);    // Password address byte 2

  Serial.println("transmitting password 1x");
  // send the 8 password bytes (factory value all 0x00)
  for(int i = 0; i < 8; i++) { 
    Wire.write(0x00); 
  }

  Serial.println("sending validation code");
  Wire.write(0x09);    // validation code

  Serial.println("transmitting password 2x");
  // resend the 8 password bytes (factory value all 0x00)
  for(int i = 0; i < 8; i++) { 
    Wire.write(0x00); 
  }

  Wire.endTransmission(1);    // stop
 
  // check the I2C_SSO_DYN register to confirm open security session
  byte c = read_dyn_register(I2C_SSO_DYN); 
  Serial.print("I2C_SSO_DYN: "); 
  Serial.println(c, BIN);

  return 0;
}

// write to mailbox for RF to scan
int16_t write_to_mailbox(char *msg){

  if(read_dyn_register(I2C_SSO_DYN) == 0) {    // if security session closed, reopen
    open_I2C_security_session(); 
  }
  
  // enable mailbox mode 
  Serial.println("Enabling Mailbox Mode.");
  write_sys_register(0x000D, 0x01);       //write_dyn_register(0x000D, 0x01);    // enable MB_MODE 
  while((read_sys_register(0x000D) & 1) != 1){
    Serial.println("MB_MODE = 0. Retrying to enable MB_MODE (1).");
    write_dyn_register(0x000D, 0x01);    // enable MB_MODE
    delay(100); 
  } 

  Serial.println("Clearing mailbox.");
  while(read_dyn_register(0x2006) != 0){
    Serial.print("MB_CTRL_DYN (will retry): ");
    Serial.println(read_dyn_register(0x2006), BIN);
    write_dyn_register(0x2006, 0x00);    // write 0x0000 to clear reset mailbox 
    delay(100);
  } 
  Serial.print("Success! MB_CTLR_DYN: "); 
  Serial.println(read_dyn_register(0x2006), BIN); 

  // enable fast transfer mode 
  Serial.println("Enabling Fast Transfer Mode.");
  write_dyn_register(0x2006, 0x01);    // enable FTM 
  while((read_dyn_register(0x2006) & 1) != 1){
    Serial.print("MB_CTRL_DYN (will retry): ");
    Serial.println(read_dyn_register(0x2006), BIN);
    write_dyn_register(0x2006, 0x01);    // enable FTM 
    delay(100);
  }
  
  Serial.print("MB_MODE: ");
  Serial.println(read_sys_register(0x000D), BIN); 
  Serial.print("MB_CTRL_DYN: ");
  Serial.println(read_dyn_register(0x2006), BIN); 
  Serial.print("MB_LEN_DYN: ");
  Serial.println(read_dyn_register(0x2007), DEC); 
  
  Serial.println("Writing to mailbox.");
  
  Wire.beginTransmission(DEV_SEL_ADDR_DYN);    // dev select
  Wire.write((MAILBOX_I2C_START_ADDR >> 8) & 0xff);    // address byte 1 MSB first  
  Wire.write(MAILBOX_I2C_START_ADDR & 0xff);    // address byte 2 LSB
  for(int i = 0; i < strlen(msg); i++){  
    Wire.write(msg[i]);    // write the value to the register
    Serial.println(msg[i]);
  }
  Wire.endTransmission(1);
  
  Serial.println("Reading mailbox.");
  for(int i = 0; i < strlen(msg); i++){
    Serial.println((char)read_dyn_register(0x2008 + i));
  }
  
  return 0;
}

// ------------

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  Wire.begin();
  Wire.setDefaultTimeout(250000);
  delay(2000);

  Serial.println("starting");
  
  // open an I2C security session
  open_I2C_security_session();
  while(read_dyn_register(I2C_SSO_DYN) != 0x01) {
    Serial.println("I2C_SSO_DYN = 0. Retrying to open security session");
    open_I2C_security_session();
  }

  Serial.print("Security session open. I2C_SSO_DYN: ");
  Serial.println(read_dyn_register(I2C_SSO_DYN), BIN); 
  
  // set EH forced after boot 
  write_sys_register(EH_MODE_SYS, 0x00);
  while(read_sys_register(EH_MODE_SYS) != 0x00) {
    Serial.println("EH_MODE_SYS = 1. Retrying to force EH mode.");
    write_sys_register(EH_MODE_SYS, 0x00);
  }
  
  Serial.print("Success! EH_MODE_SYS: "); 
  Serial.println(read_sys_register(EH_MODE_SYS), BIN); 
  
  // write to the user memory block 
  char *msg = "hello world!";
  write_to_mailbox(msg); 
} 

void loop() {
} 
