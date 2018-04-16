//High Precision MilliVolt meter from 1mv to 50v
//we advice to use LTC2400 and 24bit ADC converter you are able to use any other ADC converter
//more than standard ADC of Arduino or atmega328
//we used Crystal LCD, if you have some thing faster than this, go ahead with small changes.
//Vitapour Gmbh is international Trading Comapany in Austria working in highTech Equipments
//Program Changes and REsource, and written by: Seyed Roohollah Marashi (c) 2018
//Austria, Linz i am waiting for my residence renewal in month and for this duration manage this


#include <EEPROM.h>                        
#include <SPI.h>                              // this library need to drive LTC2400
#include <Wire.h>
#include <LiquidCrystal.h>                    // this library need to drive LCD crystal if you have i2c check address and change LIB


const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;  //Crystal LCD addressing to const variables
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                 //Allocate const variable to LCD numbering


#define averageNUM           15         // Number of samples to average (was 5)
#define reference           2.998 // Change this Number when you have exact value of your reference Source #####
#define LTC2400_CSPIN  10   //Spi library need this pin to Start Driver for SPI ADC Runnig over 3pins
uint32_t CALB = 0;  // when you have calibration machine add value for this
uint32_t arrayofReads[averageNUM] = {0};  //Matrix of Reads, when we don't have idea about number of matrix use {0} for UnKnow
uint32_t RunnigRead = 0;  // last and running Read ADC
uint8_t digitPrecision = 4;   // change this when your need more or less digits after dot .



void SPIrunner(void) {
  pinMode(LTC2400_CSPIN, OUTPUT);                // call pin for Output usage
  digitalWrite(LTC2400_CSPIN, HIGH);             // set pin ON, voltage going to CS pin and wakeup LTC2400
}


void SPIconfig(void) {
  SPI.begin();                                // this procedure is standard SPI commands take from other sources
  SPI.setBitOrder(MSBFIRST);                  // number of bits that SPI lib should know
  SPI.setDataMode(SPI_MODE0);                 // type of DataTransfer mode
  SPI.setClockDivider(SPI_CLOCK_DIV16);       // config CLock bus
}

unsigned long readADC(void) {
  digitalWrite(LTC2400_CSPIN, LOW);                //LTC2400 SLEEP
  delayMicroseconds(10);                       //short Delay for give a few millisecond to mach IC and Micro

  while ((PINB & (1 << 4)))   { }               // this loop will finish when one round of sample finished

  uint32_t reading = 0;
  for (int i = 0; i < 4; ++i) {               // 32 bits come from ADC
    reading <<= 8;                            // samples will count Down
    reading |= SPI.transfer(0xFF);            // addressing SPI from first bit to next memory bit
    if (i == 0)
      reading &= 0x0F;                        // hex numbering for last Reading
  }

  reading >>= 4;                              // no need last 4 reads and need to bypass

  digitalWrite(LTC2400_CSPIN, HIGH);               //Call the CS pin and wakeup again it mean one round is completed


  arrayofReads[RunnigRead++] = reading;    // keep reading on matrix to calculate avdrage later
  if (RunnigRead == averageNUM)  // this is swap current reading and previus reading for last read before ending limited number of reads
    RunnigRead = 0;
}


uint32_t calculateAVG(void) {          // this function is average of reads
  uint32_t sum = 0;
  for (int i = 0; i < averageNUM; i++)
    sum += arrayofReads[i];                        // value of sum is matrix reads number divide by const limited variable *averageNumber

  sum = sum / averageNUM;              // replace last number of sum into the current
  sum = sum + CALB;             // when we have calibration machine need to fix this don't touch if you dont have like me :)))))))
  return sum;
}

float convertToVoltage(uint32_t reading) {
  return reading * 10 * reference / 16777216;   //this is fixed value i take from one great code named SCULLCOM hobby special Thanks
}


void showReading(void) {
  //CALB = calculateAVG();                           // Read new average value
  //CALB = 2048000 - CALB;

  uint32_t reading = calculateAVG();

  float volt = convertToVoltage(reading);
  int  barVoltage = volt;                          // added for bar graph option

  char prefix = 0;
  if (volt < 0.001) {
    volt = volt * 1000000;
    prefix = 'u';

  } else if (volt < 1) {
    volt = volt * 1000;
    prefix = 'm';

  }
  lcd.setCursor(0, 0);
  lcd.print(prefix);
  lcd.print(volt);
  Serial.print(prefix);
  Serial.print("V : ");
  Serial.println(volt, digitPrecision);

  //
  // Print voltage as floating number with the right number of decimal places

}




void setup() {

  Serial.begin(9600);
  lcd.begin(16, 2);
  SPIrunner();
  SPIconfig();
}

// the loop function runs over and over again forever
void loop() {
  readADC();
  delay(30);                                  // read voltage from ADC
  showReading();
}
