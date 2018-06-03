#include <TimeLib.h>
#include <Encoder.h>
#include <TeensyThreads.h>
#include <Adafruit_ST7735.h>
#include <SOMO_II.h>
#include <bitmap.h>

// UI Creation 
String apps[] = { " Clock", " Music", " Photos", " Settings", " Games" };

// EVENTS 
#define PUSH 16
Encoder Crown(17, 18);
volatile long pos = 0, newPos = 0;
volatile bool HOME = true, LOCKOUT_HOME = false;

// GRAPHICS
#define SD_CS 15
#define SW 128
#define SL 160
Adafruit_ST7735 tft = Adafruit_ST7735(10, 9, 8); // CS, DC, RST

// AUDIO
#define SOMO Serial1
SOMO_II mp3 = SOMO_II(SOMO);

// FUNCTION PROTOTYPES
void home(int); // Homescreen UI
void readCrown(); // Manage rotating knob
void write(String, int, int, int, int);
void iCal();
void iTunes();
void iPhoto();
void settings();
void games();
static void(*appPointers[])(void) = { iCal, iTunes, iPhoto, settings, games };

// OTHER
#define FILE_LEN 32


void setup() {

  setTime(23, 59, 30, 31, 12, 2017); // do once
  Serial.begin(9600); //Debugging
  pinMode(PUSH, INPUT);

  // mp3 setup
  SOMO.begin(SOMO_BAUD);
  mp3.begin();
  mp3.feedback(false);

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

  // Software debounce for turn and push
  bool turned = false;

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
  // Merge all printing functionality
  tft.setTextSize(sz);
  tft.setTextWrap(false);
  tft.setCursor(x, y);
  tft.setTextColor(clr, ST7735_BLACK);
  tft.print(txt);
}


void iCal() {
  
  // RTC and Calendar Application
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
/*
class Play {
private:
  uint16_t x, y, s, colorOn, colorOff;
  bool selected;
public:
  Play(uint16_t[3] dim) { // top left x,y; square border --> s
    x = dim[0]; y = dim[1]; s = dim[2];
    selected = false;
    colorOn = ST7735_GREEN;
    colorOff = ST7735_BLACK;
  }
  void set(bool state = true) {
    if (state) {
      tft.fillCircle(x+s/2, y+s/2, s/2, 1, colorOff);
      tft.fillTriangle(x+s/4, y+s/4, x+3*s/4
    }
  }
};
*/
void iTunes() {
  LOCKOUT_HOME = true;
  bool PLAY = false, EXIT = false;
  int vol = mp3.queryVol(), newVol;
  long t = 0;
  
  tft.fillScreen(ST7735_BLACK);
  tft.drawRoundRect(85,36,20,50,1,ST7735_GREEN);
  tft.drawRoundRect(115,36,20,50,1,ST7735_GREEN);
  tft.fillCircle(50,60,25,ST7735_GREEN);
  tft.fillTriangle(42,70,62,60,42,50,ST7735_BLACK);
  tft.drawRoundRect(25,100,110,5,0,ST7735_GREEN);
  
  while(1) {
    delay(5);
    
    newVol = mp3.queryVol();
    if (newVol != vol) { // Display volume
      vol = newVol;
      tft.fillRoundRect(25,100,110,5,0,ST7735_BLACK);
      tft.drawRoundRect(25,100,110,5,0,ST7735_GREEN);
      tft.fillRoundRect(25,100,110/30*vol,5,0,ST7735_GREEN);
    }
    
    if (newPos != pos) { // Redraw UI
      tft.fillScreen(ST7735_BLACK);
      PLAY = !PLAY;
      if (PLAY) { // highlight Play
        tft.drawRoundRect(85,36,20,50,1,ST7735_GREEN);
        tft.drawRoundRect(115,36,20,50,1,ST7735_GREEN);
        tft.fillCircle(50,60,25,ST7735_GREEN);
        tft.fillTriangle(42,70,62,60,42,50,ST7735_BLACK);
      }
      else { // highlight Pause
        tft.fillRoundRect(85,36,20,50,1,ST7735_GREEN);
        tft.fillRoundRect(115,36,20,50,1,ST7735_GREEN);
        tft.drawCircle(50,60,25,ST7735_GREEN);
        tft.fillTriangle(42,70,62,60,42,50,ST7735_GREEN);
      }
      tft.drawRoundRect(25,100,110,5,0,ST7735_GREEN);
      tft.fillRoundRect(25,100,110/30*vol,5,0,ST7735_GREEN);
    }

    if (digitalRead(PUSH) == LOW) { // EXIT and Play/Pause Control
      if (!EXIT) { t = millis(); EXIT = true; }
      if (EXIT && (millis()-t > 1000)) break;
      
      if (PLAY) mp3.play();
      else mp3.pause();
    }
    else
      if (EXIT) EXIT = false;

  } // End while

  LOCKOUT_HOME = false;
  delay(1000);
}


void iPhoto() {

  if (!(SD.begin(SD_CS))) return; 
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

  int p;
  while (!HOME) {
    p = abs(newPos%fcount);
    delay(100);
    if (p != abs(newPos%fcount)) {
      p = abs(newPos%fcount);
      tft.parseSDBitmap(((String)"PICS/" + (String)files[p]).c_str());
    }
  }

  for (int i=0; i<fcount; i++) delete[] files[i];
  delete[] files;
}


void settings() {
  // Do nothing
}

void games() {
  // Do nothing
}

