_Language versions:_\
[![EN](https://github.com/pskowronek/epaper-clock-and-more/raw/master/www/flags/lang-US.png)](https://github.com/pskowronek/unitra-fm-radio-arduino) 
[![PL](https://github.com/pskowronek/unitra-fm-radio-arduino/raw/master/www/flags/lang-PL.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=pl&u=https://github.com/pskowronek/unitra-fm-radio-arduino)
[![DE](https://github.com/pskowronek/unitra-fm-radio-arduino/raw/master/www/flags/lang-DE.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=de&u=https://github.com/pskowronek/unitra-fm-radio-arduino)
[![FR](https://github.com/pskowronek/unitra-fm-radio-arduino/raw/master/www/flags/lang-FR.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=fr&u=https://github.com/pskowronek/unitra-fm-radio-arduino)
[![ES](https://github.com/pskowronek/unitra-fm-radio-arduino/raw/master/www/flags/lang-ES.png)](https://translate.googleusercontent.com/translate_c?sl=en&tl=es&u=https://github.com/pskowronek/unitra-fm-radio-arduino)

# Unitra ANIA R-612 FM Radio powered by Arduino & TEA5767 w/ Nokia 5110 display 

## A background 
Let's start with a background information. [Unitra ANIA R-612](http://www.oldradio.pl/karta.php?numer=926) is kinda old FM 'tourist' Polish mono radio produced in communist times.
It was relatively of good quality both as it comes to materials and electronics circuits inside - as it received stations very good and
the sound quality was good. From visual aspect of the design was also nice. It could be powered with 220V (now we have 230V, but it is not a problem)
and with 5x AA. It could receive LF (D), MF (Ś), HF (K) and FM aka VHF but in [OIRT band](https://en.wikipedia.org/wiki/International_Radio_and_Television_Organisation) 65.8-74.0 MHz.
As it might be suspected FM / VHF was most popular, but in late 90s all the stations were forced to move to CCIR band - 87.5-108MHz.
Finally we moved away from eastern ideas of doing things different and joined the rest of the Europe. But, this required to re-tune the radios
to use so called upper band. Three years ago when there was a need to have a radio in a bathroom I came up with this Unitra piece
but I had to re-tune it and succeeded ... only partially - the radio covered only 2/3rds of the current band and the reception was not so
much ideal (remember, it is in the bathroom - no windows, waves have problems). Let's end up here.

## The idea

The idea was to replace the old electronics with something from 21st century - there were some voices of electronics guys
that new modules are much worse etc (no filtering of low gain stations) and I should try to re-tune one more time. But, I'm not really into analog electronics, so
I started the project to revamp the radio with Arduino + TEA5767 module + Nokia 5110 display. And here it is - the revamped Unitra ANIA R-612.

## The project

### The Aim
The aim was to keep the orginal look&feel of the radio as much as possible. So, I kept the volume potentiometer working also as power-on switch.
The analog feeling of tunning along with freq indicator moving had to be kept working (eventhough the scale is not relevant anymore due to OIRT -> CCIR transition).
I could use rotary encoder for this but I felt that co-ordination with analog indicator may get lost so I decided to use rotary potentiometer
which of course is not so accurate for fine tunning plus the influence of temperature and humidity (wire connecting
analog indicator) doesn't help. But, it gives the oldschool feedback, yo! To avoid madness in the household I decided to convert the band switch
to do something usefull - to tune into 3 preconfigured frequencies, only the first position allows for free tunning.

### The Feedback
To get any feedback what's the current frequency I decided to implant Nokia 5110 display, which was hidden under the scale.
Its illumination automatically fades out when tunning is over, so it doesn't destroy the look so much. The LCD may also display the name
of the radio station, but since the FM module I used doesn't provide RDS readings, the radio station name is taken from
the list of predefined stations, also this list is used to automatically fine-tune to exact frequency. The LCD display additionally
displays the signal level and mono/stereo status.

### The Power Source
The radio is powered by 5x AA batteries consuming around 50mAh. Sadly, I didn't measure what was the original power consumption :/
The circuit for 230V has been removed (you don't want that functionality in the bathroom, right?), eventhough it could be left as it was -
the LDO could have been be easily connected to graetz bridge after transformer.


## Photos

[![Assembled](https://pskowronek.github.io/unitra-fm-radio-arduino/www/assembled/07.jpg)](https://pskowronek.github.io/unitra-fm-radio-arduino/www/assembled/index.html "Photos of assembling the radio")


More photos of the assembled Radio running this project are [here](https://pskowronek.github.io/unitra-fm-radio-arduino/www/assembled/index.html "Photos of assembling the radio and final result").

## Video

Here is the video how this revamped radio works now:
[![Video of Unitra ANIA running Arduino](https://pskowronek.github.io/unitra-fm-radio-arduino/www/video/still.jpg)](https://youtu.be/vbuSLiFZ-MU)

## Hardware

- Arduino Nano or similar
- [TEA5767 FM module](https://www.aliexpress.com/item/TEA5767-FM-Stereo-Radio-Module-for-76-108MHZ-With-Free-Cable-Antenna/32735797434.html)
- [Nokia 5110 LCD display](https://www.aliexpress.com/item/High-Quality-8448-84x84-LCD-Module-blue-backlight-adapter-PCB-for-Nokia-5110-for-Arduino/32614334972.html)
- [PAM8403 amp (I use only one channel, since radio has only one speaker)](https://www.aliexpress.com/item/PAM8403-Super-Mini-Digital-Amplifier-Board-2-3W-Class-D-Digital-2-5V-To-5V-Power/1822706737.html)
- [Rotary switch 4 position](https://botland.com.pl/en/przelaczniik-obrotowe/6163-rotary-switch-4-positions-2-circuits-30mm.html)
- [Rotary potentiometer, anything from 1k to 10k ohm will do](https://botland.com.pl/en/potentiometers/2168-potencjometr-precyzyjny-wieloobrotowy-20k-10-obr.html)
- [LDO 5V LM1117](https://botland.com.pl/en/voltage-regulators/791-linear-voltage-regulator-ldo-5v-lm1117t-tht-to220.html)
- 4x resistors 220ohm

## Circuit

Of course to do it right I should have designed a custom circuit board, but since I wanted to play around and have some freedom to
add features I used prototyping circuit boards and everything was connected using jumpers (I guess I spent more on those
jumpers than on the arduino and modules <sic!>) and a bit of hot glue :) 

The connecting scheme is mostly the same as on [Nick's](http://educ8s.tv/arduino-fm-radio-project) project with some additional
wiring to:
- control LCD brightness - connect 8-LED of LCD to digital 3 pin
- to quickly adjust to predefined stations using rotary switch - connect A1 to main pin of rotary switch, between position pins solder
a resistor (220ohm) to build a voltage ladder, connect the first pin to negative, and the last one to positive.

To provide 5V power out of the 5xAA battery pack use LDO LM1117 in the simplest manner - take a look at datasheet and simply connect GND to negative, INPUT connect
thru the switch embeded into potentiometer, and finally use OUTPUT to power all the stuff.

## License

Since this project is based on this [project](http://educ8s.tv/arduino-fm-radio-project) the original licenses still apply.
The modifications and enhancements are being done under Apache 2 license unless the original license states otherwise.

## Authors

- [Piotr Skowronek](https://github.com/pskowronek)
- [Nick - the original author](http://educ8s.tv/arduino-fm-radio-project)
