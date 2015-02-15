// spi for clock segment display via max7219
#include <SPI.h>

// these are for the clock module to read the real time
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
tmElements_t tm;

// dioder/led stuff
int redPin = 5;
int greenPin = 6;
int bluePin = 9;
int redValue = 0;
int blueValue = 0;
int greenValue = 0;


// data pin to send to max7219
const int slaveSelectPin = 10;

// define max7219 registers
byte max7219_reg_noop        = 0x00;
byte max7219_reg_digit0      = 0x01;
byte max7219_reg_digit1      = 0x02;
byte max7219_reg_digit2      = 0x03;
byte max7219_reg_digit3      = 0x04;
byte max7219_reg_digit4      = 0x05;
byte max7219_reg_digit5      = 0x06;
byte max7219_reg_digit6      = 0x07;
byte max7219_reg_digit7      = 0x08;
byte max7219_reg_decodeMode  = 0x09;
byte max7219_reg_intensity   = 0x0a;
byte max7219_reg_scanLimit   = 0x0b;
byte max7219_reg_shutdown    = 0x0c;
byte max7219_reg_displayTest = 0x0f;

const int max_segment_bar = 1;
const int max_segment_tl = 2;
const int max_segment_bl = 4;
const int max_segment_b = 8;
const int max_segment_br = 16;
const int max_segment_tr = 32;
const int max_segment_t = 64;
const int max_segment_dot = 128;
const int max_segment_all = 255;

// array of numbers 0 - 9
const int max_numbers[] = {
  max_segment_t | max_segment_tr | max_segment_br | max_segment_b | max_segment_bl | max_segment_tl, // 0
  max_segment_tr | max_segment_br, // 1
  max_segment_t | max_segment_tr | max_segment_bar | max_segment_bl | max_segment_b, // 2
  max_segment_all - max_segment_dot - max_segment_tl - max_segment_bl, // 3
  max_segment_tl | max_segment_bar | max_segment_tr | max_segment_br, // 4
  max_segment_t | max_segment_tl | max_segment_bar | max_segment_br | max_segment_b, // 5
  max_segment_t | max_segment_tl | max_segment_bl | max_segment_bar | max_segment_br | max_segment_b, // 6
  max_segment_tr | max_segment_br | max_segment_t, // 7
  max_segment_all - max_segment_dot, // 8
  max_segment_all - max_segment_dot - max_segment_bl // 9
};


void setupMaxClock() {
  pinMode (slaveSelectPin, OUTPUT);
  SPI.begin();
  writeDataToClock(max7219_reg_scanLimit, 0x07);      
  writeDataToClock(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
  writeDataToClock(max7219_reg_shutdown, 0x01);    // not in shutdown mode
  writeDataToClock(max7219_reg_displayTest, 0x00); // no display test
  setIntensity(0x0f);

}

void setupTime() {
  RTC.read(tm);
  setTime(tm.Hour,tm.Minute,tm.Second,tm.Day,tm.Month,tmYearToCalendar(tm.Year));
}

void setupLedPins() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  setupTime();
  setupMaxClock();
}



void setAllClockSegments(int to = 0xff) {
  for (int e=1; e<9; e++) {
    writeDataToClock(e,to);
  } 
}

void clearAllClockSegments() {
  setAllClockSegments(0);
}

void setIntensity(int value) {
  writeDataToClock(max7219_reg_intensity, value & 0x0f);
}

void setClockNum(int location, int num) {
  if (num < 0 || num > 9) { 
    return; 
  }
  writeDataToClock(location,max_numbers[num]);
}


void writeDataToClock(int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}

void displayTime(int hours, int mins, int secs) {
  // hours
  setClockNum(8,hours / 10);
  setClockNum(7,hours % 10);
  // mins
  setClockNum(5,mins / 10);
  setClockNum(4,mins % 10);
  // seconds
  setClockNum(2,secs / 10);
  setClockNum(1,secs % 10);
}

void setAllLeds(int value) {
  redValue = blueValue = greenValue = value; 
}

void renderLeds() {
  analogWrite(redPin,redValue);
  analogWrite(greenPin,greenValue);
  analogWrite(bluePin,blueValue);
}

void flashLeds(int colour, int times = 1) {
  for (int i = 0; i < times; i++) {
    analogWrite(colour, 255);   // turn the LED on (HIGH is the voltage level)
    delay(1000);               // wait for a second
    analogWrite(colour, 0);    // turn the LED off by making the voltage LOW
    delay(1000);               // wait for a second 
  }
}


void updateLight(int weekday,int hour,int minute,int second) {

  // monday to friday
  if (weekday >= 2 && weekday <= 6) {
    if (hour == 8) {  

      if (minute <= 20) {
        int brightness = 255 / 20 * minute;
        if (brightness > 255) { brightness = 255; }
        setAllLeds(brightness);
        renderLeds();
      } else if (minute <= 22) {
        setAllLeds(0);
        renderLeds();
        flashLeds(redPin,5);
        flashLeds(greenPin,5);
        flashLeds(bluePin,5);
        setAllLeds(255);
        renderLeds();
      } else if (minute <= 30) {
        setAllLeds(255);
        renderLeds();
      } else if (minute > 30) {
        setAllLeds(200);
        renderLeds();
      }
    
    }
  }
}

void loop() {

  clearAllClockSegments();

  int thehour = hour();
  int theminute = minute();
  int thesecond = second();
  int theweekday = weekday();
  
	Serial.println("Date and time to follow:");
	Serial.println(dayStr(theweekday));
	Serial.println(thehour);
	Serial.println(theminute);
	Serial.println(thesecond);
  
  displayTime(thehour,theminute,thesecond);

  updateLight(theweekday,thehour,theminute,thesecond);  
  
  delay(1000);

}
