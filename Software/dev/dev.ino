#include <TimeLib.h>
#include <Encoder.h>
#include <TeensyThreads.h>
#include <Adafruit_ST7735.h>
#include <SoftwareSerial.h>
#include <SOMO_II.h>
#include <bitmap.h>

// UI Creation 
String apps[] = { " Clock", " Music", " Photos", " Games", " Settings" };

// EVENTS 
#define PUSH 2
Encoder Crown(0, 1);
volatile long pos = 0, newPos = 0;
volatile bool HOME = true, LOCKOUT_HOME = false;

// SD CARD
#define SD_CS 15
#define FILE_LEN 32
bool SDACCESS; // Must initialize before SD card
                                 // otherwise a failure to load slows SPI bus

// GRAPHICS
Adafruit_ST7735 tft = Adafruit_ST7735(10, 9, 8); // CS, DC, RST
Bitmap bmpLoader = Bitmap(&tft);
#define SW 128
#define SL 160

// AUDIO
//SoftwareSerial SOMO(4,3);
#define SOMO Serial1
SOMO_II mp3 = SOMO_II(SOMO);

// FUNCTION PROTOTYPES
void home(int);                          // Homescreen UI
void readCrown();                        // Manage rotating knob
void write(String, int, int, int, int);  // Merge all printing functionality
void iCal();                             // RTC and Calendar Application
void iTunes();                           // Audio Visualizer
void iPhoto();                           // Look at your (128x160) photos!
void games();                            // Slithery lil snek
void settings();
static void(*appPointers[])(void) = { iCal, iTunes, iPhoto, games, settings };

void setup() {

  srand(analogRead(A0));
  setTime(9, 23, 30, 30, 3, 2018); // do once
  Serial.begin(9600); //Debugging
  pinMode(PUSH, INPUT);

  // mp3 setup
  SOMO.begin(SOMO_BAUD);
  mp3.begin();
  mp3.feedback(true);

  // Set display, bootup sequence
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);
  delay(100);
  for (int i=0; i<15; i++) {
    write("][][][][][][][][][][][][][", 2, 3 + i * 8, ST7735_GREEN, 1);
    delay(50);
  }
  delay(1000);
  
  tft.fillScreen(ST7735_BLACK);
  threads.addThread(readCrown);
  home(0);

  SDACCESS = SD.begin(SD_CS);
}


void loop() {

  delay(100);
  int selected = abs(pos) % 5;

  if (!HOME) {
    tft.fillScreen(ST7735_BLACK);
    appPointers[selected]();
    tft.fillScreen(ST7735_BLACK);
    delay(100);
    home(selected);
  }
}


void home(int selected) {
  write("APPLE ][", 30, 4, ST7735_GREEN, 2);
  apps[selected] = apps[selected].replace(" ","]");
  for (int i = 0; i < 5; i++)
    write(apps[i], 0, 32 + i * 16, ST7735_GREEN, 2);
  apps[selected].replace("]"," ");
}


void readCrown() {

  bool turned = false; // Software-based debounce for turning

  while (1) {
    delay(50);
    newPos = Crown.read() / 4;

    // Turn
    if (newPos != pos) {
      if (turned) 
        pos = newPos;
      if (HOME)
        home(abs(pos) % 5);
      turned = !turned;
    }

    // Push
    if ((digitalRead(PUSH) == LOW) && !LOCKOUT_HOME) {
      HOME = !HOME;
      delay(100);
    }
  } // end while
} // end readCrown


void write(String txt, int x, int y, int clr, int sz) {

  tft.setTextSize(sz);
  tft.setTextWrap(false);
  tft.setCursor(x, y);
  tft.setTextColor(clr, ST7735_BLACK);
  tft.print(txt);
  
}


void iCal() {
  
  tft.drawCircle(SL/2, SW/2, 50, ST7735_GREEN);
  float h, m, s;
  int hL = 25, mL = 35, sL = 40;
  
  while (!HOME) {
    
    // Convert time to angle of clockhand
    h = (hourFormat12() % 12 * 60.0 + minute()) / 720.0 * 6.28;
    m = (minute()*60.0 + second()) / 3600.0 * 6.28;
    s = second() / 60.0 * 6.28;
    
    // Update clock
    write(String(dayShortStr(weekday())) + ", " + String(day()) + " " + String(monthShortStr(month())), 50, 80, ST7735_GREEN, 1);
    tft.drawLine(SL/2, SW/2, uint16_t(SL/2 + hL * sin(h)), uint16_t(SW/2 - hL * cos(h)), ST7735_GREEN);
    tft.drawLine(SL/2, SW/2, uint16_t(SL/2 + mL * sin(m)), uint16_t(SW/2 - mL * cos(m)), ST7735_GREEN);
    tft.drawLine(SL/2, SW/2, uint16_t(SL/2 + sL * sin(s)), uint16_t(SW/2 - sL * cos(s)), ST7735_GREEN);
    delay(1000);
    
    // Prepare for screen redraw
    write(String(dayShortStr(weekday())) + ", " + String(day()) + " " + String(monthShortStr(month())), 50, 80, ST7735_BLACK, 1);
    tft.drawLine(SL/2, SW/2, uint16_t(SL/2 + hL * sin(h)), uint16_t(SW/2 - hL * cos(h)), ST7735_BLACK);
    tft.drawLine(SL/2, SW/2, uint16_t(SL/2 + mL * sin(m)), uint16_t(SW/2 - mL * cos(m)), ST7735_BLACK);
    tft.drawLine(SL/2, SW/2, uint16_t(SL/2 + sL * sin(s)), uint16_t(SW/2 - sL * cos(s)), ST7735_BLACK);
  }
}

void iTunes() {

  int n = 40, x0 = 0, y0 = 30;
  int w = round(160/n)-1;
  float bars[n], newBars[n], del[n];
  uint8_t vol = 0;
  int i;

  for (i=0; i<n; i++) { bars[i] = 0; newBars[i] = 0; }

  tft.setRotation(3);
  
  while (!HOME) {

    uint8_t v = mp3.queryVol();
    if (v != 65) vol = 8*v;
    
    for (i=0; i<n; i++) { // set newBars
      newBars[i] = round(vol*abs(sin(6.28*(rand()%100))));
      del[i] = (newBars[i] - bars[i])/4;
    }
    
    for (int s=0; s<4; s++) {
      for (i=0; i<n; i++) {
        bars[i]+=del[i];
        tft.fillRect(x0+(w+1)*i, y0, w, bars[i], ST7735_GREEN);
      }
      delay(50);
      for (i=0; i<n; i++)
        tft.fillRect(x0+(w+1)*i, y0, w, bars[i], ST7735_BLACK);
    }
    
  };

  tft.setRotation(1);
  
}


void iPhoto() {

  //if (!SDACCESS) return;

  Serial.println("HEY!");

  File pics = SD.open("PICS/");

  File entry;
  int fcount = 0;
  while (entry=pics.openNextFile()) {
    fcount++;
    entry.close();
  }
  
  pics.seek(0);
  char** files = new char*[fcount];
  for (int i=0; i<fcount; i++) {
    entry = pics.openNextFile();
    files[i] = new char[FILE_LEN];
    strcpy(files[i], entry.name());
    entry.close();
  }
  pics.close();
  
  int p=-1;
  while (!HOME) {
    if (p==-1) bmpLoader.drawSDBitmap(((String)"PICS/" + (String)files[abs(newPos%fcount)]).c_str());
    p = abs(newPos%fcount);
    delay(100);
    if (p != abs(newPos%fcount)) {
      p = abs(newPos%fcount);
      bmpLoader.drawSDBitmap(((String)"PICS/" + (String)files[p]).c_str());
    }
  }

  for (int i=0; i<fcount; i++) delete[] files[i];
  delete[] files;
}


void settings() {
  tft.fillScreen(ST7735_BLACK);
  delay(1000);
}

void games() {
  tft.fillScreen(ST7735_BLACK);
  delay(1000);
}

