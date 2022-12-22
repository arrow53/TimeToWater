#include <M5StickC.h>
#include <esp_bt_main.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

RTC_TimeTypeDef RTC_TimeStruct;

int MinAnalog;
int Levels[7];
int SecondsPrev;
int i1;
int Blink = 0;
int Orientation = 3;
float Threshold = 50.0;

void setup() 
{
  M5.begin();
  M5.IMU.Init();
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(1);
  pinMode(37, INPUT);
  pinMode(39, INPUT);
  // red LED
  pinMode(10, OUTPUT);
  MinAnalog = 4095;

  for (i1 = 0; i1 < 7; i1++)
  {
    Levels[i1] = 0;
  }

  RTC_TimeTypeDef TimeStruct;
  TimeStruct.Hours = 12;
  TimeStruct.Minutes = 0;
  TimeStruct.Seconds = 0;
  M5.Rtc.SetTime(&TimeStruct);

  Serial.begin(9600);
  esp_wifi_stop();
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
}

void loop() 
{
  // read stuff
  M5.Rtc.GetTime(&RTC_TimeStruct);
  int Hour = RTC_TimeStruct.Hours;
  int Min = RTC_TimeStruct.Minutes;
  int Sec = RTC_TimeStruct.Seconds;
  int Analog = analogRead(33);
  int TopBtn = digitalRead(37);
  int BotBtn = digitalRead(39);

  // set stuff
  if (Analog < MinAnalog)
  {
    MinAnalog = Analog;
  }
  if (TopBtn == 0)
  {
    MinAnalog = 4095;
  }
  if (BotBtn == 0)
  {
    Threshold -= 10.0;
    if (Threshold < 10.0)
    {
      Threshold = 90.0;
    }
  }

  float CurDiff = (float)(4095 - Analog);
  float MaxDiff = (float)(4095 - MinAnalog);
  // shouldn't hit but error check
  if (CurDiff > MaxDiff)
  {
    CurDiff = MaxDiff;
  }
  if (MaxDiff == 0)
  {
    MaxDiff = 1;
  }
  float Percent = CurDiff / MaxDiff * 100.0;
  int PercentInt = (int)round(Percent);

  if (PercentInt < Threshold)
  {
    if (Blink == 0)
    {
      Blink = 1;
    }
    else
    {
      Blink = 0;
    }
    digitalWrite(10, Blink);
  }
  else
  {
    digitalWrite(10, 1);
  }

  Levels[0] = PercentInt;
  if (Hour == 12 && Min == 0 && SecondsPrev == 59)
  {
    for (i1 = 1; i1 < 7; i1++)
    {
      // push right
      Levels[7 - i1] = Levels[7 - i1 - 1];
      MinAnalog = 4095;
    }
  }
  SecondsPrev = Min;

  // display stuff
  // x is first then y
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.printf("%04d %04d %03d%% %02d:%02d:%02d\n", Analog, MinAnalog, PercentInt, Hour, Min, Sec);

  // 80 x 160. First is x and second is y
  // 18 * 7 = 126. 160 - 126 = 34. 8 * 4 = 32
  // we are going to run height from 20 to 75 for a total height of 55
//  M5.Lcd.fillRect(4, 20, 18, 55, GREEN); 
//  M5.Lcd.fillRect(26, 75 - 1, 18, 1, GREEN); 
  for (i1 = 0; i1 < 7; i1++)
  {
    float LevelsFloat = (float)Levels[i1];
    float Height = 55.0 * LevelsFloat / 100.0;
    int HeightInt = (int)Height;
    if (HeightInt > 55)
    {
      HeightInt = 55;
    }
    if (HeightInt < 1)
    {
      HeightInt = 1;
    }
    //Serial.print(i1);
    //Serial.print(" ");
    int Offset = 4 + i1 * 22;
    M5.Lcd.fillRect(Offset, 75 - HeightInt, 18, HeightInt, GREEN); 
    // 20 + 55 - 1 is 74 
    // 75 -1 to 75
    M5.Lcd.fillRect(Offset, 20, 18, 55 - HeightInt - 1, BLACK); 
  }
  // goes from 20 to 75 so delta is 55
  int Level = 20 + 55 - (int)round(Threshold / 100.0 * 55) - 1;
  M5.Lcd.fillRect(4, Level, 18, 2, RED); 
  //Serial.println(Threshold);
  //Serial.println(Level);
  Serial.println(Levels[0]);
  
  delay(500);
}
