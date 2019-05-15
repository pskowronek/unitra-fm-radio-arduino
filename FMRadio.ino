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


#include <TEA5767N.h>  //https://github.com/mroger/TEA5767
#include <LCD5110_Graph.h> //http://www.rinkydinkelectronics.com/library.php?id=48

LCD5110 lcd(8,9,10,12,11);
TEA5767N radio = TEA5767N();

// Analog PIN number to read analog value from to set the frequency
const int FREQ_PIN = A0;
// Analog PIN number to read the modes: manual freq adjustments, or predefined ones 
const int MODE_PIN = A1;

// Digial PIN number to control LCD backlight
const int BACKLIGHT_PIN = 3;

// Freq range: 87.5 MHz to 108.00 MHz (x10 for sake of mapping)
const int FREQ_START = 875;
const int FREQ_END = 1080;
// Freq potentiometer range - if its physical rotation range is 100%, then use 0-1023,
// otherwise adjust accordingly max out to what is possible (due to mechanical constraints of old radio mechanism for instance)
const int FREQ_POTENTIOMETER_START = 0;
const int FREQ_POTENTIOMETER_END = 820;

// Freq init
const float FREQ_INIT = 87.5;
// delay between fresh of freq readings and LCD refresh (in ms)
const int REFRESH_DELAY = 10;
// startup delay / splash screen (in ms)
const int SPLASHSCREEN_TIME = 1000;
// Update receive details every n-th interation
const int UPDATE_SCREEN_EVERY = 5000 / (5*REFRESH_DELAY);
// A default backlight intensity
const int DEFAULT_BACKLIGHT_INTENSITY = 0;
// A number of readings of frequency potentiometer or mode pin-out to avarage from
const int NUMBER_OF_READINGS_FOR_AVG = 5;

extern unsigned char BigNumbers[];
extern unsigned char TinyFont[];

extern uint8_t splash[];
extern uint8_t signal5[];
extern uint8_t signal4[];
extern uint8_t signal3[];
extern uint8_t signal2[];
extern uint8_t signal1[];

// The list of most used frequencies (radio stations) to ideally tune-in + display its name
const float SUGGESTED_FREQS[] = { 99.4, 101.6, 102, 102.9 };
const String SUGGESTED_FREQS_NAMES[] = { "RADIO TROJKA", "RADIO KRAKOW", "RADIO DWOJKA", "RADIO TOK FM" };
// The threshold to say when to ideally adjust to one of suggested freqs
const float SUGGESTED_THRESHOLD = 0.2;

// The list of stations for quick access done by rotary switch (where first state is manual
// adjustment and 2nd, 3rd and 4th states tune to given indecies from SUGGESTED_FREQS above).
// To read the state a PIN FREQ_PIN - for each state the following resistor ladder readins are designated:
// 1st mode (manual): 0ohm value reading i.e. connected to negative
// 2nd mode - for 1*1k ohm - first station index
// 3nd mode - for 2*1k ohm - second station index
// 4th mode - for 3*1k ohm - third station index
const int MAX_PREDEFINED_FREQS = 3; // the number of states besides the manual one
const int PREDEFINED_FREQS_IDX[MAX_PREDEFINED_FREQS] = { 0, 2, 3 }; // idx of SUGGESTED_FREQS
// Ballpark values for each state mode
const int PREDEFINED_MODE_VALUES[MAX_PREDEFINED_FREQS + 1] = { 0, 430, 870, 1000 }; // readings values of resitors ladder
// The % of the values above to still match the given mode
const float PREDEFINED_MODE_APPROX = 100; // +/- this value

// the mode we are in (0 - manual, 1 - 1st station and so on)
int mode = 0;
// previous mode
int previousMode = -1;

float currentFrequency = 0;
int detailsUpdateCounter = UPDATE_SCREEN_EVERY;
int backlightIntensity = DEFAULT_BACKLIGHT_INTENSITY;


void setup() {
  pinMode(FREQ_PIN, INPUT);
  pinMode(MODE_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, backlightIntensity);
  
  radio.mute();
  radio.setSoftMuteOn();
  radio.setStereoNoiseCancellingOn();
  radio.setHighCutControlOn();
  radio.selectFrequency(FREQ_INIT);
  radio.setMonoReception(); // comment out for stereo
  
  initScreen();
  showSplashScreen();
  delay(SPLASHSCREEN_TIME);
  lcd.clrScr();
  lcd.update();
  currentFrequency = radio.readFrequencyInMHz();
  mode = readMode();
}
 
void loop() {
  if (mode == 0) {
      bool freqAdjustement = adjustFrequency();  
      updateScreen();
      if (!freqAdjustement) {
          delay(REFRESH_DELAY);
      }
  } else {
      setFrequencyByMode(mode);
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
       printSignalStrength();
       printStereo();
       detailsUpdateCounter = 0;
  } else {
       detailsUpdateCounter++;
  }
  printFrequency();
  lcd.update();
}

void updateBacklight() {
   if (backlightIntensity < 255) {
       backlightIntensity++;
   }
}

void showSplashScreen() {
  lcd.drawBitmap(0, 0, splash, 84, 48);
  lcd.update();  
}

void setFrequencyByMode(int mode) {
  if (previousMode != mode) {
      lcd.clrScr();
      float frequencyToSet = tuneIn(SUGGESTED_FREQS[PREDEFINED_FREQS_IDX[mode - 1]]);
      radio.selectFrequency(frequencyToSet);
      currentFrequency = frequencyToSet;
      radio.turnTheSoundBackOn();
      detailsUpdateCounter = UPDATE_SCREEN_EVERY;
      backlightIntensity = DEFAULT_BACKLIGHT_INTENSITY;
  }
}

int readMode() {
  int val = 0; 
  for (int i = 0; i < NUMBER_OF_READINGS_FOR_AVG; i++) {
      val = val + analogRead(MODE_PIN); 
      delay(1);
  }
  val = val / NUMBER_OF_READINGS_FOR_AVG; // avg of readings


  float maxVal = val + PREDEFINED_MODE_APPROX;
  float minVal = val - PREDEFINED_MODE_APPROX;
  if (minVal < 0) {
      minVal = 0;
  }
  
  for (int i = 0; i <= MAX_PREDEFINED_FREQS; i++) {
      if (PREDEFINED_MODE_VALUES[i] >= minVal && PREDEFINED_MODE_VALUES[i] <= maxVal) {
          return i;
      }
  }
  return 0;
}

bool adjustFrequency() {  
  bool result = false;
  int val = 0; 
  
  for (int i = 0; i < NUMBER_OF_READINGS_FOR_AVG; i++) {
      val = val + analogRead(FREQ_PIN); 
      delay(1);
  }
  val = val / NUMBER_OF_READINGS_FOR_AVG; // avg of readings  
  int frequencyToSetInt = map(val, FREQ_POTENTIOMETER_START, FREQ_POTENTIOMETER_END, FREQ_START, FREQ_END); // map analog value to freq range
  float frequencyToSet = frequencyToSetInt / 10.0f;

  if (abs(frequencyToSet - currentFrequency) > 0.1f) {
      float frequencyTunedIn = tuneIn(frequencyToSet);
      // get radio's frequency (to ensure we work correctly with artificially tuned-in suggested radios
      float realFreq = radio.readFrequencyInMHz();
      // only refresh sceen or light LCD up if necessary (if outside of suggested freq threshold)
      if (abs(frequencyTunedIn != realFreq) > 0.01f) {
          if (frequencyToSet >= 100 && currentFrequency < 100 ||
              currentFrequency >= 100 && frequencyToSet < 100 || 
              previousMode != mode) {

              lcd.clrScr(); // since this is kinda slow, do this only if a number of digits changed,
                            // 'cuz normally any updated digit clears its background so full clrscr isn't necessary
          }
          radio.selectFrequency(frequencyTunedIn);
          radio.turnTheSoundBackOn();
          detailsUpdateCounter = UPDATE_SCREEN_EVERY;
          backlightIntensity = DEFAULT_BACKLIGHT_INTENSITY;
          currentFrequency = frequencyToSet;
      }
      result = true;
  }
  return result;
}

float tuneIn(float freq) {
  int arraySize = sizeof(SUGGESTED_FREQS) / sizeof(SUGGESTED_FREQS[0]);
  lcd.setFont(TinyFont);
  for (int i = 0; i < arraySize; i++) {
      float suggestedFreq = SUGGESTED_FREQS[i];
      
      if (abs(suggestedFreq - freq) <= SUGGESTED_THRESHOLD) {
          lcd.print("><", 30, 2);
          lcd.print(SUGGESTED_FREQS_NAMES[i], 10, 37);
          lcd.update();
          return suggestedFreq;
      }
  }
  lcd.print("  ", 30, 2);           // reset
  lcd.print("                      ", 10, 37); // reset
  lcd.update();
  return freq;  
}

void printFrequency() {
  String frequencyString = String(currentFrequency, 1);
  lcd.setFont(BigNumbers);
  lcd.print(frequencyString, frequencyString.length() <= 4 ? 14 : 0, 12);
}

void printStereo() {
  boolean isStereo = radio.isStereo();
  lcd.setFont(TinyFont);
  lcd.print(isStereo ? "STEREO" : "      ", 55, 2);
}

void printSignalStrength() {
  int signalStrength = radio.getSignalLevel();
  String signalStrenthString = String(signalStrength);
  if (signalStrength >= 15) {
      lcd.drawBitmap(1, 1, signal5, 17, 6);
  } else if (signalStrength >= 11 && signalStrength < 15) {
      lcd.drawBitmap(1, 1, signal4, 17, 6);
  } else if (signalStrength >= 9 && signalStrength < 11) {
      lcd.drawBitmap(1, 1, signal3, 17, 6);
  } else if (signalStrength >= 7 && signalStrength < 9) {
      lcd.drawBitmap(1, 1, signal2, 17, 6);
  } else if (signalStrength < 7) {
      lcd.drawBitmap(1, 1, signal1, 17, 6);
  }
}
