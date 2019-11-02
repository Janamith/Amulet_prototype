//---------------------------------------------------------------------------------------------------
// I2C protocol for AK IR Sensor
// Human Proximity Sensing 
//---------------------------------------------------------------------------------------------------

#include <i2c_t3.h>
#define EH_ADDR 0x0002
#define READ_LENGTH 8
#define DEV_SEL_ADDR 0b1010011

void mode_setup(int address);
int16_t read_register(int address);
static int reset_counter = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(9600);

  Wire.begin();
  Wire.setDefaultTimeout(250000);
  digitalWrite(LED_BUILTIN, HIGH);

  delay(2000);
  /*
  Wire.beginTransmission(DEV_SEL_ADDR);
  Wire.write(0x20);
  Wire.write(0x02);
  Wire.write(9);
  Wire.endTransmission(1);
  */
  delay(2000);
} 

void loop() {
  int EH_reg_val = read_register(EH_ADDR);
  delay(1000);
} 

int16_t read_register(int address) {
  uint8_t data[READ_LENGTH];

  Wire.beginTransmission(DEV_SEL_ADDR);
  Wire.write(0x00);
  Wire.write(0x02);
  Wire.endTransmission(0);
  Wire.requestFrom(DEV_SEL_ADDR, 1);
  byte c = Wire.read();
  Serial.println(c);

  Wire.beginTransmission(DEV_SEL_ADDR);
  Wire.write(0x20);
  Wire.write(0x02);
  Wire.endTransmission(0);
  Wire.requestFrom(DEV_SEL_ADDR, 1);
  c = Wire.read();
  Serial.println(c);
  
  return 0;
}





