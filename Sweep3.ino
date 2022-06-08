//1. (Hard Code) Map each midi note to two servo coordinates
//2. Calculate the delay time between notes and solenoid pressdown time
//3. Fire piston once both delays are finished
//4. Get ready for next note

//MIDI Servo
#include <Servo.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>


//OLED Code
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

//MIDI Code
struct MySettings : public midi::DefaultSettings {
  static const long BaudRate = 128000;
};
enum NoteStatus : bool {
  NOT_PLAYING = 0,
  PLAYING
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MySettings)

//Solenoid Code
int solenoid = 9;

//Servo Code
Servo servo1;  // create servo object to control a servo
Servo servo2;  // create servo object to control a servo

//Timing Code
//Adjust for my inconsistent trash robot
float timeScale = 0;
bool targetActive = false;

//Technical Junk
String junk[25] =
{
  "NAVIGATE",
  "ERROR 814",
  "help",
  "CONDUCTOR",
  "REVERSING",
  "regress",
  "W S P",
  "TRANSFER",
  "syntax jam",
  "SERVO2",
  "HI-POWER",
  "REVERSING",
  "M O V E",
  "WSP NULL",
  "indexing",
  "WSP LOCK",
  "Position",
  "horsepower",
  "donies",
  "Garfield Undetectable",
  "MOVING",
  "SERVO 1",
  "HIGH VOLT",
  "Establish",
  "FIRE"
};
int junkIndex = 0;

//All possible locations
int pos[64][2] = {
  {910, 1625},   //0 X
  {940, 1689},
  {995, 1759},
  {1055, 1825},
  {1125, 1885},   //4
  {1188, 1950},
  {1250, 2015},
  {1320, 2085},   //7 X
  {940, 1625},
  {990, 1670},   //9
  {1042, 1733},
  {1100, 1798},   //11
  {1162, 1856},
  {1218, 1921},
  {1288, 1990},   //14
  {1355, 2043},
  {1000, 1590},
  {1038, 1638},
  {1095, 1714},
  {1150, 1775},   //19
  {1198, 1817},
  {1253, 1876},
  {1315, 1945},
  {1371, 1977},
  {1068, 1545},   //24 X
  {1100, 1615},
  {1145, 1667},
  {1185, 1719},
  {1245, 1780},
  {1296, 1847},   //29
  {1356, 1878},
  {1399, 1924},   //31 X
  {1500, 1500},   //Only using half of the launchpad
  {1500, 1500},
  {1500, 1500},   //34
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},   //39
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},   //44
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},   //49
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},   //54
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},   //59
  {1500, 1500},
  {1500, 1500},
  {1500, 1500},
  {1050, 1930}   //63
};

void setup() {
  display.display();
  display.setRotation(2); //Flip the screen 180
  //Attach Servo
  servo1.attach(11);  // attaches the servo on pin 11 to the servo object
  servo2.attach(10);  // attaches the servo on pin 10 to the servo object

  //Attach Solenoid
  pinMode(9, OUTPUT);

  //LaunchMidi and listen
  //MIDI.begin(5);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  //8 Hits, 1 to each corner
  display.display();

  //Set timescale
  timeScale = 0.542; //174bpm
  // GoTo(13,172.41,0);

  // GoTo(63,600000000,0); //Count in

}

void loop()
{
  
  //GoTo(13, 200, -1);
  //PlayTheSong();
  
//  GoTo(63, 200, -1); GoTo(63, 200, 10); //Count in  
//  GoTo(63, 200, -1); GoTo(63, 200, 10);
//  GoTo(63, 200, -1); GoTo(63, 200, 10);
//  GoTo(63, 200, -1); GoTo(63, 200, 10);
  //OneOfEach(); //Test
}

void OneOfEach() {
  //Test each co-ord
  int index = 0;
  for (index = 0; index <= 31; index++) {
    GoTo(index, 1000, 0);
  }
}

void GoTo(int index, int pause, int pressTime) {
  //Keep moving
  servo1.writeMicroseconds(pos[index][0]);
  servo2.writeMicroseconds(pos[index][1]);
  testscrolltext(index);
  delay((pause * timeScale) - (pressTime * timeScale)); //in ms

  if (pressTime != -1) {  //Dont fire solenoid
    FirePiston(pressTime * timeScale);
  }
  else {
    //Do Nothing
  }
}

void FirePiston(int downTime) {

  digitalWrite(solenoid, HIGH);

  int inversion = junkIndex % 7;
  //Inversion
  if (inversion > 5) {
    display.invertDisplay(true);
  }
  else {
    display.invertDisplay(false);
  }

  display.clearDisplay();
  display.setTextSize(3); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 0);
  display.println("FIRE");
  display.display();

  delay(downTime); //in ms
  digitalWrite(solenoid, LOW);

}

//Screen Text
void testscrolltext(int inputInt) {

  String myText = String(pos[inputInt][0]) + " " + String(pos[inputInt][1]);

  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(myText);
  display.display();      // Show initial text

  myText = junk[junkIndex];
  junkIndex++;
  if (junkIndex > 23) {
    junkIndex = 0;
  }

  //
  int dispMode = junkIndex % 4;
  int inversion = junkIndex % 7;

  //Scrolling type
  switch (dispMode) {
    case 0:
      // statements
      display.stopscroll();
      display.startscrollright(0x00, 0x0F);
      if (inversion > 4) {
        display.stopscroll();
      }
      break;
    case 1:
      // statements
      //display.stopscroll();
      display.startscrollleft(0x00, 0x0F);
      break;
      if (inversion > 4) {
        display.stopscroll();
      }
    case 2:
      // statements
      //display.stopscroll();
      display.startscrolldiagright(0x00, 0x07);
      if (inversion > 4) {
        display.startscrollright(0x00, 0x0F);
        display.stopscroll();
      }
      break;
    case 3:
      // statements
      //display.stopscroll();
      display.startscrolldiagleft(0x00, 0x07);
      if (inversion > 4) {
        display.stopscroll();
      }
      break;
  }

  //Inversion
  if (inversion > 5) {
    display.invertDisplay(true);
  }
  else {
    display.invertDisplay(false);
  }

  display.setCursor(0, 15);
  display.println(myText);
  display.display();      // Show initial text

}

void PlayTheSong() {
  
  GoTo(63, 1000, -1);
  
  GoTo(63, 200, -1); GoTo(63, 200, 0); //Count in  
  GoTo(63, 200, -1); GoTo(63, 200, 0);
  GoTo(63, 200, -1); GoTo(63, 200, 0);
  GoTo(63, 200, -1); GoTo(63, 200, 0);
  GoTo(63, 200, -1); GoTo(63, 200, 0); //Count in
  GoTo(63, 200, -1); GoTo(63, 200, 0);
  GoTo(63, 200, -1); GoTo(63, 200, 0);
  GoTo(63, 200, -1); GoTo(63, 200, 0);
  
  GoTo(13, 35, -1); //0
  GoTo(13, 200, 10); 
  GoTo(7, 235, -1); //
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 225, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 225, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(5, 225, -1); 
  GoTo(5, 200, -1); 
  GoTo(5, 200, 10);
  GoTo(14, 215, -1); 
  GoTo(14, 200, 10);
  GoTo(6, 205, -1); 
  GoTo(6, 200, -1); 
  GoTo(6, 200, 10);
  GoTo(23, 215, -1); 
  GoTo(23, 200, -1); 
  GoTo(23, 175, 10);
  GoTo(23, 200, 10);
  GoTo(15, 200, 10);
  GoTo(18, 200, -1); 
  GoTo(18, 200, 20); 
  GoTo(19, 200, 10); 
  GoTo(9, 190, -1); 
  GoTo(9, 200, 10);
  GoTo(10, 190, 10);
  GoTo(11, 200, 10);
  GoTo(13, 145, 30);
  GoTo(7, 250, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 220, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 210, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(14, 280, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, 10);
  GoTo(21, 200, 20);
  GoTo(29, 200, -1); 
  GoTo(29, 200, 10);
  GoTo(22, 175, 10);
  GoTo(29, 175, 10);
  GoTo(0, 200, -1); //
  GoTo(0, 200, 10);
  GoTo(1, 175, 30);
  GoTo(2, 175, -1); 
  GoTo(2, 200, 10);
  GoTo(3, 175, 10);
  GoTo(4, 210, -1); 
  GoTo(4, 200, 10);
  GoTo(12, 210, 30);
  GoTo(20, 180, 30);
  GoTo(13, 200, 50); //32
  GoTo(7, 250, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 225, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 210, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(5, 225, -1); 
  GoTo(5, 200, -1); 
  GoTo(5, 200, 10);
  GoTo(14, 200, -1); 
  GoTo(14, 200, 10);
  GoTo(6, 200, -1); 
  GoTo(6, 200, -1); 
  GoTo(6, 200, 10);
  GoTo(23, 225, -1); 
  GoTo(23, 200, -1); 
  GoTo(23, 200, 10);
  GoTo(23, 200, 10);
  GoTo(15, 190, 10); //44
  //GoTo(17, 200, 10);
  GoTo(18, 170, -1); 
  GoTo(18, 200, 20); 
  GoTo(19, 200, 10); 
  GoTo(9, 200, -1); 
  GoTo(9, 200, 10);
  GoTo(10, 190, 10);
  GoTo(11, 190, 10);
  GoTo(13, 190, 10); //48
  GoTo(7, 230, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 210, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 210, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(14, 210, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, 10); //56
  GoTo(29, 225, -1); 
  GoTo(29, 200, -1); 
  GoTo(29, 200, 10);
  GoTo(22, 210, 10);
  GoTo(29, 190, 30);
  GoTo(23, 200, -1); 
  GoTo(23, 200, 10);
  GoTo(13, 100, -1); //64
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, 10);
  GoTo(7, 250, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 210, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 210, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(3, 250, -1); 
  GoTo(3, 200, -1); 
  GoTo(3, 200, -1); 
  GoTo(3, 200, -1); 
  GoTo(3, 200, 10);
  GoTo(10, 200, -1); 
  GoTo(10, 200, 20);
  GoTo(17, 200, 10);
  GoTo(13, 400, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, -1); 
  GoTo(13, 200, 10);
  GoTo(7, 250, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(7, 210, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 210, 10);
  GoTo(7, 200, -1); 
  GoTo(7, 200, -1); 
  GoTo(7, 200, 10);
  GoTo(14, 240, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, -1); 
  GoTo(14, 200, 10);
  GoTo(29, 220, -1); 
  GoTo(29, 200, -1); 
  GoTo(29, 200, 10);
  GoTo(22, 200, 10);
  GoTo(29, 190, 30);

  //Song End
  GoTo(29, 200, -1); 
  GoTo(29, 200, -1); 
  GoTo(29, 200, -1); 
  
  GoTo(63, 10000, 0); //Count out

  
  //  GoTo(63,344.83,0); //Count out
  //  GoTo(63,344.83,0);
  //  GoTo(63,344.83,0);
  //  GoTo(63,344.83,0);
}

void ScreenTesting() {
  testscrolltext(0);
  delay(2000);
  testscrolltext(7);
  delay(2000);
  testscrolltext(24);
  delay(2000);
  testscrolltext(31);
  delay(2000);

}

void Calibrate() {
  GoTo(0, 700, 10);
  delay(40);
  GoTo(7, 700, 10);
  delay(40);
  GoTo(24, 700, 10);
  delay(40);
  GoTo(31, 700, 10);
  delay(40);

  GoTo(0, 400, 10);
  //FirePiston(40);
  GoTo(7, 400, 10);
  //FirePiston(40);
  GoTo(24, 400, 10);
  //FirePiston(40);
  GoTo(31, 400, 10);
  //FirePiston(40);
}
