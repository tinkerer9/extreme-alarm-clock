#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
RTC_DS1307 rtc;

byte clocksymbol[8] = {
  0x00,
  0x04,
  0x0E,
  0x0E,
  0x1F,
  0x00,
  0x04,
  0x00
};

const int backlight = 8;
const int onLed = 13;
const int light = 12;
const int lightButton = A3;
const int btSet = A0;
const int btAdj = A1;
const int btAlarm = A2;

char daysOfTheWeek[7][12] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

int DD, MM, YY, H, M, S, AH, AM;
int i = 0;
int btnCount = 0;
String sDD;
String sMM;
String sYY;
String sH;
String sM;
String sS;
String sW;
String aH = "12";
String aM = "00";

long interval = 500;
long previousMillis = 0;

bool set_state, adjust_state, alarm_state, light_button_state, pre_light_button_state;
bool setupScreen = false;
bool alarmON = false;
bool turnItOn = false;
bool backlightON = false;
bool lightState = false;

void setup() {
  rtc.begin();

  lcd.begin(16, 2);

  pinMode(btSet, INPUT_PULLUP);
  pinMode(btAdj, INPUT_PULLUP);
  pinMode(btAlarm, INPUT_PULLUP);
  pinMode(lightButton, INPUT_PULLUP);

  pinMode(backlight, OUTPUT);
  pinMode(onLed, OUTPUT);
  pinMode(light, OUTPUT);

  lcd.createChar(1, clocksymbol);

  digitalWrite(backlight, backlightON);


  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");

    rtc.adjust(DateTime(2000, 01, 01, 00, 00, 0));
  }

  AH = EEPROM.read(0);
  AM = EEPROM.read(1);
  alarmON = EEPROM.read(2);

  if (AH > 23) {
    AH = 0;
  }
  if (AM > 59) {
    AM = 0;
  }
}

void loop() {
  readBtns();
  getTimeDate();

  if (setupScreen) {
    timeSetup();
  } else {
    if (backlightON) {
      lcdPrint();
    }
    if (alarmON) {
      callAlarm();
    } else if (backlightON) {
      lcd.setCursor(10, 0);
      lcd.write(" ");
    }
  }
}

void readBtns() {
  set_state = digitalRead(btSet);
  adjust_state = digitalRead(btAdj);
  alarm_state = digitalRead(btAlarm);
  light_button_state = digitalRead(lightButton);

  if (!setupScreen) {
    if (alarm_state == LOW && !turnItOn) {
      if (!backlightON) {
        backlightON = true;
        digitalWrite(backlight, HIGH);
      }
      if (alarmON) {
        alarmON = false;
      } else {
        alarmON = true;
      }
      EEPROM.write(2, alarmON);
      delay(500);
    }

    if (adjust_state == LOW) {
      if (backlightON) {
        backlightON = false;
        digitalWrite(backlight, LOW);
        lcd.clear();
        digitalWrite(onLed, LOW);
      } else {
        backlightON = true;
        digitalWrite(backlight, HIGH);
      }
      delay(500);
    }
  }

  if (set_state == LOW) {
    if (btnCount < 7) {
      btnCount++;
      setupScreen = true;
      if (btnCount == 1) {
        if (!backlightON) {
          backlightON = true;
          digitalWrite(backlight, HIGH);
        }
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("SET");
        lcd.setCursor(1, 1);
        lcd.print("TIME and DATE");
        delay(2000);
        lcd.clear();
      }
    } else {
      lcd.clear();
      rtc.adjust(DateTime(YY, MM, DD, H, M, 0));  //Save time and date to RTC IC
      EEPROM.write(0, AH);                        //Save the alarm hours to EEPROM 0
      EEPROM.write(1, AM);                        //Save the alarm minuted to EEPROM 1
      lcd.setCursor(3, 0);
      lcd.print("Saving....");
      delay(2000);
      lcd.clear();
      setupScreen = false;
      btnCount = 0;
    }
    delay(500);
  }

  if (light_button_state == LOW && pre_light_button_state == HIGH) {
    lightState = !lightState;
    digitalWrite(light, lightState);
  }

  pre_light_button_state = light_button_state;
}

void getTimeDate() {
  if (!setupScreen) {
    DateTime now = rtc.now();
    DD = now.day();
    MM = now.month();
    YY = now.year() % 100;
    H = now.hour();
    M = now.minute();
    S = now.second();
    sW = daysOfTheWeek[now.dayOfTheWeek()];
  }
  //Make some fixes...
  if (DD < 10) {
    sDD = '0' + String(DD);
  } else {
    sDD = DD;
  }
  if (MM < 10) {
    sMM = '0' + String(MM);
  } else {
    sMM = MM;
  }
  sYY = YY;
  if (H < 10) {
    sH = '0' + String(H);
  } else {
    sH = H;
  }
  if (M < 10) {
    sM = '0' + String(M);
  } else {
    sM = M;
  }
  if (S < 10) {
    sS = '0' + String(S);
  } else {
    sS = S;
  }
  if (AH < 10) {
    aH = '0' + String(AH);
  } else {
    aH = AH;
  }
  if (AM < 10) {
    aM = '0' + String(AM);
  } else {
    aM = AM;
  }
}

void lcdPrint() {
  digitalWrite(onLed, alarmON);

  String line1 = sH + ":" + sM + ":" + sS + " |";
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (alarmON) {
    lcd.setCursor(10, 0);
    lcd.write(1);
  }
  String line2 = aH + ":" + aM;
  lcd.setCursor(11, 0);
  lcd.print(line2);

  String line3 = sW + " " + sMM + "-" + sDD + "-" + sYY;
  lcd.setCursor(2, 1);
  lcd.print(line3);
}

void timeSetup() {
  int up_state = adjust_state;
  int down_state = alarm_state;
  if (btnCount <= 5) {
    if (btnCount == 1) {
      lcd.setCursor(4, 0);
      lcd.print(">");
      if (up_state == LOW) {
        if (H < 23) {
          H++;
        } else {
          H = 0;
        }
        delay(350);
      }
      if (down_state == LOW) {
        if (H > 0) {
          H--;
        } else {
          H = 23;
        }
        delay(350);
      }
    } else if (btnCount == 2) {
      lcd.setCursor(4, 0);
      lcd.print(" ");
      lcd.setCursor(9, 0);
      lcd.print(">");
      if (up_state == LOW) {
        if (M < 59) {
          M++;
        } else {
          M = 0;
        }
        delay(350);
      }
      if (down_state == LOW) {
        if (M > 0) {
          M--;
        } else {
          M = 59;
        }
        delay(350);
      }
    } else if (btnCount == 3) {
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(1, 1);
      lcd.print(">");
      if (up_state == LOW) {
        if (MM < 12) {
          MM++;
        } else {
          MM = 1;
        }
        delay(350);
      }
      if (down_state == LOW) {
        if (MM > 1) {
          MM--;
        } else {
          MM = 12;
        }
        delay(350);
      }
    } else if (btnCount == 4) {
      lcd.setCursor(1, 1);
      lcd.print(" ");
      lcd.setCursor(6, 1);
      lcd.print(">");
      if (up_state == LOW) {
        if (DD < 31) {
          DD++;
        } else {
          DD = 1;
        }
        delay(350);
      }
      if (down_state == LOW) {
        if (DD > 1) {
          DD--;
        } else {
          DD = 31;
        }
        delay(350);
      }
    } else if (btnCount == 5) {
      lcd.setCursor(6, 1);
      lcd.print(" ");
      lcd.setCursor(11, 1);
      lcd.print(">");
      if (up_state == LOW) {
        if (YY < 2999) {
          YY++;
        } else {
          YY = 2000;
        }
        delay(350);
      }
      if (down_state == LOW) {
        if (YY >= 2000) {
          YY--;
        } else {
          YY = 2999;
        }
        delay(350);
      }
    }
    lcd.setCursor(5, 0);
    lcd.print(sH);
    lcd.setCursor(8, 0);
    lcd.print(":");
    lcd.setCursor(10, 0);
    lcd.print(sM);
    lcd.setCursor(2, 1);
    lcd.print(sMM);
    lcd.setCursor(5, 1);
    lcd.print("-");
    lcd.setCursor(7, 1);
    lcd.print(sDD);
    lcd.setCursor(10, 1);
    lcd.print("-");
    lcd.setCursor(12, 1);
    lcd.print(sYY);
  } else {
    setAlarmTime();
  }
}

void setAlarmTime() {
  int up_state = adjust_state;
  int down_state = alarm_state;
  String line2;
  lcd.setCursor(1, 0);
  lcd.print("SET ALARM TIME");
  if (btnCount == 6) {
    if (up_state == LOW) {
      if (AH < 23) {
        AH++;
      } else {
        AH = 0;
      }
      delay(350);
    }
    if (down_state == LOW) {
      if (AH > 0) {
        AH--;
      } else {
        AH = 23;
      }
      delay(350);
    }
    line2 = "    >" + aH + " : " + aM + "    ";
  } else if (btnCount == 7) {
    if (up_state == LOW) {
      if (AM < 59) {
        AM++;
      } else {
        AM = 0;
      }
      delay(350);
    }
    if (down_state == LOW) {
      if (AM > 0) {
        AM--;
      } else {
        AM = 59;
      }
      delay(350);
    }
    line2 = "     " + aH + " :>" + aM + "    ";
  }
  lcd.setCursor(5, 1);
  lcd.print(line2);
}

void callAlarm() {
  if (aM == sM && aH == sH && S >= 0 && S <= 10) {
    turnItOn = true;
    backlightON = true;
    digitalWrite(backlight, HIGH);
  } else if (alarm_state == LOW || M >= (AM + 3)) {
    turnItOn = false;
    alarmON = true;
    delay(50);
  }

  if (turnItOn) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;
      lightState = !lightState;
      digitalWrite(light, lightState);
    }
  }
}