#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"
#include "HughesyShiftBrite.h"


SdReader card;        // This object holds the information for the card
FatVolume vol;        // This holds the information for the partition on the card
FatReader root;       // This holds the information for the filesystem on the card
int numFiles = 0;     // The number of files in the root directory
dir_t dirBuf;         // buffer for directory reads
WaveHC wave;          // This is the only wave (audio) object, since we will only play one at a time

HughesyShiftBrite sb;    // Global ShiftBrite guy
int numShiftBrites = 4;  // The number of ShiftBrites hooked up
int fadeDelay = 25;      // How many ms to delay while fading

int purple_r = 767;      // The color purple
int purple_g = 0;
int purple_b = 100;

int orange_r = 1023;     // The color orange
int orange_g = 766;
int orange_b = 0;


void setup() {
  Serial.begin(9600); //set up Serial library at 9600 bps for debugging
  
  putstring_nl("\nIt's either you or them");
 
  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  if (!card.init()) {
    putstring_nl("Wave Shield card init failed");
    while(1); //then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
  
  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Figure out how many files we have to choose from
  getNumFiles();
  
  // Init the randomizer
  randomSeed(analogRead(0));
  
  //ShiftBrite!
  sb = HughesyShiftBrite(22, 23, 24, 25);
}

//////////////////////////////////// LOOP
void loop() {
  //Fade 5-10 times
  fader(random(5, 11));

  //Scare them
  sendColor(1023, 0, 0); // Flash red
  playRandom();          // Play a WAV
}

/////////////////////////////////// WAVE SHIELD HELPERS

/*
 * Fetches the number of files in the root directory
 */
void getNumFiles() {
  Serial.println("Scanning root directory");
  numFiles = 0;
  while (root.readDir(dirBuf) > 0) {
    if(DIR_IS_FILE(dirBuf)) {
      Serial.print("Found file "); Serial.print(numFiles+1, DEC); Serial.print(": "); printEntryName(dirBuf); Serial.println("");
      numFiles++;
    }
  }
}

/*
 * Sets dirBuf to the num file in the root directory - skips subdirectories
 */
void getFileNum(int num) {
  int currFileNum = 0;

  while (root.readDir(dirBuf) > 0) {
    if(DIR_IS_FILE(dirBuf) && currFileNum == num) {
      return;
    } else {
      currFileNum++;
    }
  }
}

/*
 * Play a random file in the root directory
 */
void playRandom() {
  root.rewind();
  //Set dirBuf to the random fileNum
  int fileNum = random(numFiles);
  getFileNum(fileNum);
  Serial.print("Playing file "); Serial.print(fileNum+1, DEC); Serial.print(" of "); Serial.print(numFiles, DEC); Serial.print(": "); printEntryName(dirBuf);

  //Open it
  FatReader file;
  file.open(vol, dirBuf);

  //Play it
  if (wave.create(file)) {
    wave.play();
    while (wave.isplaying) {
      putstring(".");
      delay(100);
    }
    Serial.println("");
  } else {
    Serial.println(" - Not a valid WAV!");
  }
}

/////////////////////////////////// SHIFTBRITE HELPERS
/*
 * Fades between purple and orange numTimes
 */
void fader(int numTimes) {
  //Start at purple
  int r = purple_r;
  int g = purple_g;
  int b = purple_b;
  sendColor(r, g, b);
  
  for(int i=0; i<numTimes; i++) {
    Serial.print("Fade #"); Serial.print(i+1, DEC); Serial.print(" of "); Serial.println(numTimes, DEC);
    Serial.println("Fading to orange");
    while(r != orange_r && g != orange_g && b != orange_b) {
      delay(fadeDelay);
      r = moveTowards(r, orange_r);
      g = moveTowards(g, orange_g);
      b = moveTowards(b, orange_b);
      sendColor(r, g, b);
    }
    
    Serial.println("Fading to purple");
    while(r != purple_r && g != purple_g && b != purple_b) {
      delay(fadeDelay);
      r = moveTowards(r, purple_r);
      g = moveTowards(g, purple_g);
      b = moveTowards(b, purple_b);
      sendColor(r, g, b);
    }
  }
}

int moveTowards(int startValue, int targetValue) {
  if(targetValue == startValue) {
    return startValue;
  }

  if(targetValue > startValue) {
    return ++startValue;
  }
  
  return --startValue;
}

void sendColor(int r, int g, int b) {
  for(int i=0; i<numShiftBrites; i++) {
    sb.sendColour(r, g, b);
  }
}
