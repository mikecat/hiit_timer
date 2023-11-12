#include <M5StickCPlus.h>

float BAT_MAX_VOLTAGE = 4.2f;
float BAT_MIN_VOLTAGE = 3.0f;

struct TimerParams {
  int runSeconds;
  int pauseSeconds;
  int middleBeepIntervalSeconds;
};

const TimerParams timerParams[] = {
  {20, 10, 5},
  {60, 0, 10},
};

bool inTitle;

bool prevInTitle;
int batPercentDrawn;
bool prevIsRunPhase;
unsigned long prevSeconds;
int prevPeriod;

TimerParams timerParamsUsed;
bool isRunPhase;
unsigned long startTime;
unsigned long targetTime;

void setup() {
  M5.begin();
  M5.Lcd.begin();
  M5.Lcd.setRotation(1);
  inTitle = true;
  prevInTitle = false;
  batPercentDrawn = -1;
  prevSeconds = -1;
}

void loop() {
  M5.update();
  if (inTitle) {
    int batPercent = (M5.Axp.GetBatVoltage() - BAT_MIN_VOLTAGE) / (BAT_MAX_VOLTAGE - BAT_MIN_VOLTAGE) * 100;
    if (batPercent < 0) batPercent = 0;
    if (batPercent > 100) batPercent = 100;
    if (prevInTitle != inTitle || batPercent != batPercentDrawn) {
      char batBuffer[64];
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.drawString("HIIT Timer", 10, 10);
      snprintf(batBuffer, sizeof(batBuffer), "BAT:%3d%%", batPercent);
      M5.Lcd.drawString(String(batBuffer), 10, 30);
      prevInTitle = inTitle;
      batPercentDrawn = batPercent;
    }
    if (M5.BtnA.wasReleased()) {
      timerParamsUsed = timerParams[0];
      targetTime = 0;
      isRunPhase = false;
      inTitle = false;
      startTime = millis();
    }
    if (M5.BtnB.wasReleased()) {
      timerParamsUsed = timerParams[1];
      targetTime = 0;
      isRunPhase = false;
      inTitle = false;
      startTime = millis();
    }
  } else {
    unsigned long currentTime = millis() - startTime;
    int periodLeft = (targetTime - currentTime) / 1000 / timerParamsUsed.middleBeepIntervalSeconds;
    int secondsLeftDisplay = (targetTime - currentTime + 999) / 1000;
    if (prevInTitle != inTitle || prevSeconds != secondsLeftDisplay || prevIsRunPhase != isRunPhase) {
      char secondBuffer[16];
      M5.Lcd.fillScreen(isRunPhase ? TFT_GREEN : TFT_ORANGE);
      M5.Lcd.setTextSize(10);
      M5.Lcd.setTextColor(TFT_BLACK);
      snprintf(secondBuffer, sizeof(secondBuffer), "%2d", secondsLeftDisplay);
      M5.Lcd.drawString(String(secondBuffer), 10, 10);
      prevInTitle = inTitle;
      prevSeconds = secondsLeftDisplay;
      prevIsRunPhase = isRunPhase;
    }
    if (currentTime >= targetTime) {
      M5.Beep.tone(500, 500);
      if (isRunPhase) {
        if (timerParamsUsed.pauseSeconds > 0) {
          isRunPhase = false;
          targetTime = timerParamsUsed.pauseSeconds * 1000;
        } else {
          inTitle = true;
        }
      } else {
        isRunPhase = true;
        targetTime = timerParamsUsed.runSeconds * 1000;
      }
      startTime = millis();
      prevPeriod = targetTime / 1000 / timerParamsUsed.middleBeepIntervalSeconds;
    } else if (prevPeriod != periodLeft) {
      if (secondsLeftDisplay != targetTime / 1000) M5.Beep.tone(500, 50);
      prevPeriod = periodLeft;
    }
    if (M5.BtnA.wasReleased()) {
      M5.Beep.tone(500, 200);
      inTitle = true;
    }
  }
}
