    /////////////////////////////////////////////////////////////////
   //                 Arduino FM Radio Project                    //
  //       Get the latest version of the code here:              //
 //     📥 http://educ8s.tv/arduino-fm-radio-project            //
/////////////////////////////////////////////////////////////////

#include <TEA5767N.h>  //https://github.com/mroger/TEA5767
#include <LCD5110_Graph.h> //http://www.rinkydinkelectronics.com/library.php?id=48

LCD5110 lcd(8,9,10,12,11);
TEA5767N radio = TEA5767N();

extern unsigned char BigNumbers[];
extern unsigned char TinyFont[];

extern uint8_t splash[];
extern uint8_t signal5[];
extern uint8_t signal4[];
extern uint8_t signal3[];
extern uint8_t signal2[];
extern uint8_t signal1[];

int analogPin = 0;
int val = 0; 
int frequencyInt = 0;
float frequency = 0;
float previousFrequency = 0;
int signalStrength = 0;

void setup() 
{
  radio.setMonoReception();
  radio.setStereoNoiseCancellingOn();
  initScreen();
  showSplashScreen();
  Serial.begin(9600);
}
 
void loop() {
  
  for(int i;i<30;i++)
  {
     val = val + analogRead(analogPin); 
     delay(1);
  }
  
  val = val/30;
  frequencyInt = map(val, 2, 1014, 8700, 10700); //Analog value to frequency from 87.0 MHz to 107.00 MHz 
  float frequency = frequencyInt/100.0f;

  if(frequency - previousFrequency >= 0.1f || previousFrequency - frequency >= 0.1f)
  {
    lcd.clrScr();
    radio.selectFrequency(frequency);
    printSignalStrength();
    printStereo();
    printFrequency(frequency);
    previousFrequency = frequency;    
  }
  
  lcd.clrScr();
  printSignalStrength();
  printStereo();
  printFrequency(frequency);
  delay(50); 
  val = 0;  
}

void initScreen()
{
  lcd.InitLCD();
  lcd.setFont(BigNumbers);
  lcd.clrScr();
}

void showSplashScreen()
{
  lcd.drawBitmap(0, 0, splash, 84, 48);
  lcd.update();  
  delay(3000);
  lcd.clrScr();
  lcd.update();
}

void printFrequency(float frequency)
{
  String frequencyString = String(frequency,1);
  if(frequencyString.length() == 4)
  {
    lcd.setFont(BigNumbers);
    lcd.print(frequencyString,14,12);
    lcd.update();
  }
  else
  {
    lcd.setFont(BigNumbers);
    lcd.print(frequencyString,0,12);
    lcd.update();
  }
}
void printStereo()
{
    boolean isStereo = radio.isStereo();
     if(isStereo)
    {
      lcd.setFont(TinyFont);
      lcd.print("STEREO",55,2);
    }
}

void printSignalStrength()
{
  signalStrength = radio.getSignalLevel();
  String signalStrenthString = String(signalStrength);
  if(signalStrength >=15)
  {
    lcd.drawBitmap(1, 1, signal5, 17 , 6);
  }else if(signalStrength >=11 && signalStrength <15)
  {
    lcd.drawBitmap(1, 1, signal4, 17 , 6);
  }
  else if(signalStrength >=9 && signalStrength <11)
  {
    lcd.drawBitmap(1, 1, signal3, 17 , 6);
  }
   else if(signalStrength >=7 && signalStrength <9)
  {
    lcd.drawBitmap(1, 1, signal2, 17 , 6);
  }
   else if(signalStrength <7)
  {
    lcd.drawBitmap(1, 1, signal1, 17 , 6);
  }
}

