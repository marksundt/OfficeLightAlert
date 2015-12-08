#include <Wire.h>

#define TSL2572_I2CADDR     0x39

#define GAIN_1X 0
#define GAIN_8X 1
#define GAIN_16X 2
#define GAIN_120X 3

//only use this with 1x and 8x gain settings
#define GAIN_DIVIDE_6 true 

int gain_val = 0;
int too_dark = 0;
float AmbientLightLux = 0.0;
int light_segment = 0;

void setup()
{
  Serial.begin(9600);
  LedOn(0);//Pass a zero to turn all LEDs off

  TSL2572nit(GAIN_1X);

  Serial.println("Set Up Completed!");
}

void loop()
{
  AmbientLightLux = Tsl2572ReadAmbientLight();
  Serial.print("Lux: ");


  light_segment = (int)AmbientLightLux/20;

  Serial.print("Ambient: ");
  Serial.println(AmbientLightLux);
  Serial.print("Segment: ");
  Serial.println(light_segment);
  
  if(light_segment <= 1) 
    {
    too_dark++;
    }
  else
    {
    too_dark = 0;
    }

  if(too_dark > 5)
    {
      ToDark();
    }
  LedOn(light_segment);
  delay(100000);
 };


void ToDark()
{
  // Five min notification
 Serial.println("Turn On Lights!");
 for(int x=1; x< 10; x++)
 {
  for(int i=1;i<21;i++){
    LedOn(i);
    delay(50);
  };
  
  for(int i=21;i>1;i--){
    LedOn(i);
    delay(50);
  };
 };
   
}

void LedOn(int ledNum)
{
  for(int i=4;i<10;i++){
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  };
  if(ledNum<1 || ledNum>21) return;
  char highpin[21]={5,6,5,7,6,7,6,8,5,8,8,7,9,7,9,8,5,9,6,9,9};
  char lowpin[21]= {6,5,7,5,7,6,8,6,8,5,7,8,7,9,8,9,9,5,9,6,4};
  ledNum--;
  digitalWrite(highpin[ledNum],HIGH);
  digitalWrite(lowpin[ledNum],LOW);
  pinMode(highpin[ledNum],OUTPUT);
  pinMode(lowpin[ledNum],OUTPUT);
}

void TSL2572nit(uint8_t gain)
{
  Tsl2572RegisterWrite( 0x0F, gain );//set gain
  Tsl2572RegisterWrite( 0x01, 0xED );//51.87 ms
  Tsl2572RegisterWrite( 0x00, 0x03 );//turn on
  if(GAIN_DIVIDE_6)
    Tsl2572RegisterWrite( 0x0D, 0x04 );//scale gain by 0.16
  if(gain==GAIN_1X)gain_val=1;
  else if(gain==GAIN_8X)gain_val=8;
  else if(gain==GAIN_16X)gain_val=16;
  else if(gain==GAIN_120X)gain_val=120;
}


void Tsl2572RegisterWrite( byte regAddr, byte regData )
{
  Wire.beginTransmission(TSL2572_I2CADDR);
  Wire.write(0x80 | regAddr); 
  Wire.write(regData);
  Wire.endTransmission(); 
}


float Tsl2572ReadAmbientLight()
{     
  uint8_t data[4]; 
  int c0,c1;
  float lux1,lux2,cpl;

  Wire.beginTransmission(TSL2572_I2CADDR);
  Wire.write(0xA0 | 0x14);
  Wire.endTransmission();
  Wire.requestFrom(TSL2572_I2CADDR,4);
  for(uint8_t i=0;i<4;i++)
    data[i] = Wire.read();
     
  c0 = data[1]<<8 | data[0];
  c1 = data[3]<<8 | data[2];
  
  //see TSL2572 datasheet
  cpl = 51.87 * (float)gain_val / 60.0;
  if(GAIN_DIVIDE_6) cpl/=6.0;
  lux1 = ((float)c0 - (1.87 * (float)c1)) / cpl;
  lux2 = ((0.63 * (float)c0) - (float)c1) / cpl;
  cpl = max(lux1, lux2);
  return max(cpl, 0.0);
}

