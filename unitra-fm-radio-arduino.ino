/*
 * skowro.net https://github.com/pskowronek/unitra-fm-radio-arduino
 * Any changes introduced are under License Apache 2.0 unless the original license states different.
 * The project evolved from:
 */

    /////////////////////////////////////////////////////////////////
   //                 Arduino FM Radio Project                    //
  //       Get the latest version of the code here:              //
 //     📥 http://educ8s.tv/arduino-fm-radio-project            //
/////////////////////////////////////////////////////////////////

// select either TEA or RDS - TEA stands for TEA5767N or RDA5807M in TEA mode (no RDS then), and RDS for RDA5807M (with RDS)
//#define TEA 1
#define RDS 1

#include <LCD5110_Graph.h> //http://www.rinkydinkelectronics.com/library.php?id=48

#include <radio.h>
#ifdef TEA
  #include <TEA5767.h>
  TEA5767 radio;
#elif RDS
  #include <RDA5807M.h>
  #include <RDSParser.h>
  RDA5807M radio;
  RDSParser rds;
#endif

const byte FIX_BAND = RADIO_BAND_FM;  // band, RADIO_BAND_FM - EU&US, RADIO_BAND_FMWORLD - Japan
const byte FIX_VOLUME = 14;   // 0..15 - RDA only, set it to almost max if you intent to use analog potentiometer

// Analog PIN number to read analog value from to set the frequency
const byte FREQ_PIN = A0;
// Analog PIN number to read the modes: manual freq adjustments, or predefined ones 
const byte MODE_PIN = A1;

// Digial PIN number to control LCD backlight
const byte BACKLIGHT_PIN = 3;

// Freq range: 87.5 MHz to 108.00 MHz (x10 for sake of mapping)
const RADIO_FREQ FREQ_START = 8750;
const RADIO_FREQ FREQ_END = 10800;
// Freq potentiometer range - if its physical rotation range is 100%, then use 0-1023,
// otherwise adjust accordingly to max out to what is possible (due to mechanical constraints of old radio mechanism for instance)
// The values might differ when you change power source.
const int FREQ_POTENTIOMETER_START = 85;
const int FREQ_POTENTIOMETER_END = 775;

// Freq to set when starting (will be overriden by freq potentiometer reading).
const RADIO_FREQ FREQ_INIT = 8750; // *100



// delay between refresh of freq readings and LCD refresh (in ms)
const byte REFRESH_DELAY = 10;
// startup delay / splash screen (in ms)
const int SPLASHSCREEN_TIME = 1000;
// Whether to peridically update signal level and stereo/mono on the screen 
const bool SHOULD_UPDATE_PERIODICALLY = false; // I needed to set it to false for RDA5807m (I2C communication to get status interrupted the reception) :/
// Update receive details every n-th interation
const int UPDATE_SCREEN_EVERY = 5000 / (5*REFRESH_DELAY);

#define LCD_FULL_BRIGHTNESS_AT_0 1  // comment it out if your display has 255 for full light intensity
#ifdef LCD_FULL_BRIGHTNESS_AT_0
  // A default backlight intensity
  const byte SCREEN_FULL_BACKLIGHT_INTENSITY = 0; 
#else
  const byte SCREEN_FULL_BACKLIGHT_INTENSITY = 255; 
#endif


// A number of readings of frequency potentiometer or mode pin-out to avarage from
const byte NUMBER_OF_READINGS_FOR_AVG = 5;

// The list of most used frequencies (radio stations) to ideally tune-in + display its name
const RADIO_FREQ SUGGESTED_FREQS[] = { 9940, 10160, 10200, 102900 };
const String SUGGESTED_FREQS_NAMES[] = { "RADIO TROJKA", "RADIO KRAKOW", "RADIO DWOJKA", "RADIO TOK FM" };
// The threshold to say when to ideally adjust to one of suggested freqs
const uint16_t SUGGESTED_THRESHOLD = 200;

// The list of stations for quick access done by rotary switch (where first state is manual
// adjustment and 2nd, 3rd and 4th states tune to given indecies from SUGGESTED_FREQS above).
// To read the state a PIN FREQ_PIN - for each state the following resistor ladder readins are designated:
// 1st mode (manual): 0ohm value reading i.e. connected to negative
// 2nd mode - for 1*1k ohm - first station index
// 3nd mode - for 2*1k ohm - second station index
// 4th mode - for 3*1k ohm - third station index
const byte MAX_PREDEFINED_FREQS = 3; // the number of states besides the manual one
const byte PREDEFINED_FREQS_IDX[MAX_PREDEFINED_FREQS] = { 0, 2, 3 }; // idx of SUGGESTED_FREQS
// Ballpark values for each state mode
const byte PREDEFINED_MODE_VALUES[MAX_PREDEFINED_FREQS + 1] = { 0, 430, 770, 1000 }; // readings values of resitor ladder
// The % of the values above to still match the given mode
const byte PREDEFINED_MODE_APPROX = 100; // +/- this value

LCD5110 lcd(8,9,10,12,11);

extern unsigned char BigNumbers[];
extern unsigned char TinyFont[];

extern uint8_t splash[];
extern uint8_t signal5[];
extern uint8_t signal4[];
extern uint8_t signal3[];
extern uint8_t signal2[];
extern uint8_t signal1[];


// the mode we are in (0 - manual, 1 - 1st station and so on)
byte mode = 0;
// previous mode
byte previousMode = -1;

RADIO_FREQ currentFrequency = 0;
int detailsUpdateCounter = UPDATE_SCREEN_EVERY;
byte backlightIntensity = SCREEN_FULL_BACKLIGHT_INTENSITY;
String currentStationName = "";

void setup() {
  pinMode(FREQ_PIN, INPUT);
  pinMode(MODE_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, backlightIntensity);

  radio.init();
  radio.setMute(true);
  radio.setSoftMute(true);
  radio.setBandFrequency(FIX_BAND, FREQ_INIT);
  radio.setMono(true); // set to false for stereo
  #ifdef RDS
    //radio.setVolume(FIX_VOLUME);
    //radio.attachReceiveRDS(RDS_process);
    //rds.attachServicenNameCallback(DisplayServiceName);
  #endif
  
  initScreen();
  showSplashScreen();
  delay(SPLASHSCREEN_TIME);
  lcd.clrScr();
  lcd.update();
  lcdDebug("1");
  currentFrequency = radio.getFrequency();
  mode = readMode();
}
 
void loop() {
  if (mode == 0) {
      bool freqAdjustement = adjustFrequency();
      currentStationName = "";
      radio.checkRDS();
      updateScreen();
      if (!freqAdjustement) {
          delay(REFRESH_DELAY);
      }
  } else {
      setFrequencyByMode(mode);
      radio.checkRDS();
      updateScreen();
      delay(REFRESH_DELAY);
  }
  previousMode = mode;
  mode = readMode();
}

void initScreen() {
  lcd.InitLCD();
  lcd.setFont(BigNumbers);
  lcd.clrScr();
}

void updateScreen() {
  analogWrite(BACKLIGHT_PIN, backlightIntensity);
  updateBacklight();

  if (detailsUpdateCounter >= UPDATE_SCREEN_EVERY) {
       printRcvDetails();
       detailsUpdateCounter = 0;
  } else {
       if (SHOULD_UPDATE_PERIODICALLY) {
          detailsUpdateCounter++;
       }
  }
  printFrequency();
  lcd.update();
}

void updateBacklight() {
   #ifdef LCD_FULL_BRIGHTNESS_AT_0
     if (backlightIntensity < 255) {
         backlightIntensity++;
     }
   #else
     if (backlightIntensity > 0) {
         backlightIntensity--;
     }   
   #endif
}

void showSplashScreen() {
  lcd.drawBitmap(0, 0, splash, 84, 48);
  lcd.update();  
}

void setFrequencyByMode(int mode) {
  if (previousMode != mode) {
      lcd.clrScr();
      uint16_t frequencyToSet = tuneIn(SUGGESTED_FREQS[PREDEFINED_FREQS_IDX[mode - 1]]);
      radio.setFrequency(frequencyToSet);
      currentFrequency = frequencyToSet;
      radio.setMute(false);
      detailsUpdateCounter = UPDATE_SCREEN_EVERY;
      backlightIntensity = SCREEN_FULL_BACKLIGHT_INTENSITY;
  }
}

byte readMode() {
  int val = 0; 
  for (byte i = 0; i < NUMBER_OF_READINGS_FOR_AVG; i++) {
      val = val + analogRead(MODE_PIN); 
      delay(1);
  }
  val = val / NUMBER_OF_READINGS_FOR_AVG; // avg of readings

  int maxVal = val + PREDEFINED_MODE_APPROX;
  int minVal = val - PREDEFINED_MODE_APPROX;
  if (minVal < 0) {
      minVal = 0;
  }
  
  for (byte i = 0; i <= MAX_PREDEFINED_FREQS; i++) {
      if (PREDEFINED_MODE_VALUES[i] >= minVal && PREDEFINED_MODE_VALUES[i] <= maxVal) {
          return i;
      }
  }
  return 0;
}

bool adjustFrequency() {
  lcdDebug("2");
  bool result = false;
  int val = 0; 
  
  for (byte i = 0; i < NUMBER_OF_READINGS_FOR_AVG; i++) {
      val = val + analogRead(FREQ_PIN); 
      delay(1);
  }
  val = val / NUMBER_OF_READINGS_FOR_AVG; // avg of readings  
  RADIO_FREQ frequencyToSet = map(val, FREQ_POTENTIOMETER_START, FREQ_POTENTIOMETER_END, FREQ_START, FREQ_END); // map analog value to freq range

  if (abs(frequencyToSet - currentFrequency) > 9) {
      RADIO_FREQ frequencyTunedIn = tuneIn(frequencyToSet);
      // get radio's frequency (to ensure we work correctly with artificially tuned-in suggested radios
      lcdDebug("3");
      RADIO_FREQ realFreq = radio.getFrequency();
      // only refresh sceen or light LCD up if necessary (if outside of suggested freq threshold)
      lcdDebug("4");
      if (abs(frequencyTunedIn != realFreq) > 9) {
          lcdDebug("5");
          if (frequencyToSet >= 10000 && currentFrequency < 10000 ||
              currentFrequency >= 10000 && frequencyToSet < 10000 || 
              previousMode != mode) {

              lcd.clrScr(); // since this is kinda slow, do this only if a number of digits changed,
                            // 'cuz normally any updated digit clears its background so full clrscr isn't necessary
          }
          radio.setFrequency(frequencyTunedIn);
          radio.setMute(false);
          detailsUpdateCounter = UPDATE_SCREEN_EVERY;
          backlightIntensity = SCREEN_FULL_BACKLIGHT_INTENSITY;
          currentFrequency = frequencyToSet;
          lcdDebug("6");
      }
      result = true;
  }
  lcdDebug("7");
  return result;
}

RADIO_FREQ tuneIn(RADIO_FREQ freq) {
  byte arraySize = sizeof(SUGGESTED_FREQS) / sizeof(SUGGESTED_FREQS[0]);
  lcd.setFont(TinyFont);
  for (byte i = 0; i < arraySize; i++) {
      RADIO_FREQ suggestedFreq = SUGGESTED_FREQS[i];
      
      if (abs(suggestedFreq - freq) <= SUGGESTED_THRESHOLD) {
          lcd.print("><", 30, 2);
          printStationName(SUGGESTED_FREQS_NAMES[i]);
          lcd.update();
          return suggestedFreq;
      }
  }
  lcd.print("  ", 30, 2);           // reset
  lcd.print("                      ", 10, 37); // reset
  lcd.update();
  return freq;  
}

void printStationName(String name) {
  lcd.setFont(TinyFont);
  lcd.print(name, 10, 37);
}

void printFrequency() {
  String frequencyString = "AA"; //String(float(currentFrequency) / 100, 1);
  lcd.setFont(BigNumbers);
  lcd.print(frequencyString, frequencyString.length() <= 4 ? 14 : 0, 12);
}

void printRcvDetails() {
  return;
  RADIO_INFO info;
  radio.getRadioInfo(&info);
  boolean isStereo = info.stereo;
  uint8_t signalStrength = info.rssi;

  if (info.rds && currentStationName.length() > 0) {
      printStationName(currentStationName);
  }

  if (signalStrength >= 15) {
      lcd.drawBitmap(1, 1, signal5, 17, 6);
  } else if (signalStrength >= 11) {
      lcd.drawBitmap(1, 1, signal4, 17, 6);
  } else if (signalStrength >= 9) {
      lcd.drawBitmap(1, 1, signal3, 17, 6);
  } else if (signalStrength >= 7) {
      lcd.drawBitmap(1, 1, signal2, 17, 6);
  } else {
      lcd.drawBitmap(1, 1, signal1, 17, 6);
  }
  lcd.setFont(TinyFont);
  lcd.print(isStereo ? "STEREO" : "      ", 55, 2);
}

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}

void DisplayServiceName(char *name) {
  currentStationName = String(name);
}

void lcdDebug(String text) {
  lcd.setFont(TinyFont);
  lcd.print(text, 55, 2);
  lcd.update();
}
