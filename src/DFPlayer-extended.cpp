#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "pin.h"
#include <Bounce2.h>

void (*resetFunc)(void) = 0;
long mapWithHysteresis(long x, long in_min, long in_max, long out_min, long out_max, byte threshold = 0);
void updateVolume();

DFRobotDFPlayerMini myDFPlayer;
Bounce *switches = new Bounce[NB_SWITCHES];
void printDetail(uint8_t type, int value);

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_DFP_BUSY, INPUT);
  pinMode(PIN_SOUND_VOL, INPUT);

  for (int i = 0; i < NB_SWITCHES; i++)
  {
    switches[i].attach(PIN_SWITCHES[i], INPUT_PULLUP); //setup the bounce instance for the current button
    switches[i].interval(25);                          // interval in ms
  }

  delay(1000);

  Serial.begin(9600);

  if (!myDFPlayer.begin(Serial))
  {
    for (int i = 0; i < 10; ++i)
    {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      delay(100);
    }
    resetFunc();
  }

  // Player init
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  updateVolume();
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
}

void loop()
{
  static unsigned long timer = millis();

  for (int i = 0; i < NB_SWITCHES; i++)
  {
    switches[i].update();
    if (switches[i].fell())
    {
      myDFPlayer.playMp3Folder(i + 1);
    }
  }

  if (millis() - timer > 100)
  {
    timer = millis();
    updateVolume();
  }
}

long mapWithHysteresis(long x, long in_min, long in_max, long out_min, long out_max, byte threshold)
{
  static long lastX = x;

  if (abs(lastX - x) > threshold)
  {
    lastX = x;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  else
    return (lastX - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void updateVolume()
{
  static int volume = -1;
  int vol = analogRead(PIN_SOUND_VOL);
  vol = mapWithHysteresis(vol, 0, 1023, 0, 30, 15);
  if (vol != volume)
  {
    digitalWrite(PIN_LED, vol & 0x01);
    volume = vol;
    myDFPlayer.volume(vol);
  }
}
