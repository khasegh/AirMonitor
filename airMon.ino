/******************************************************************************

  M5Stack Air Quality monitor
  https://github.com/khasegh/AirMonitor

******************************************************************************/

#include <Wire.h>
#include <M5Stack.h>
#include "SparkFunCCS811.h"
#include "SparkFunBME280.h"

#include "envList.h"
envList el;   //測定値保存用リスト
envListDsc ed;//リスト状態

#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

CCS811 mySensorCCS(CCS811_ADDR);
BME280 mySensorBME;

struct co2Lv {
  unsigned int minCO2;   //CO2レベル最小値
  unsigned int noticeLv; //通知レベル 0:通知なし、1:画面、2:画面＋音
  uint16_t clr;          //CO2棒グラフの色
};

struct co2Lv co2LvList[] = {
  {3500, 1, TFT_RED},
  {2500, 1, TFT_PINK},
  {1500, 1, TFT_ORANGE},
  {1000, 0, TFT_YELLOW},
  {0,    0, TFT_GREEN},
};

void setup()
{
  M5.Power.begin();
  M5.Power.setPowerBtnEn(true);
  M5.Power.setPowerBoostSet(false);
  M5.Power.setPowerBoostOnOff(false);
  M5.Power.setPowerBoostKeepOn(false);
  M5.Power.setPowerVin(false);

  M5.begin(true, false, true, true);
  M5.Speaker.begin();
  M5.Speaker.mute();
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextFont(2);

  // initialize CCS811
  if (mySensorCCS.begin() == false) {
    M5.Lcd.println("CCS811 error");
    while (1);
  }
  mySensorCCS.setDriveMode(2); // 0=idle, 1=1sec, 2=10sec, 3=60sec, 4=RAW mode

  // initialize BME280
  mySensorBME.settings.commInterface = I2C_MODE;
  mySensorBME.settings.I2CAddress = 0x77;
  mySensorBME.settings.runMode = 3; //Normal mode
  mySensorBME.settings.tStandby = 0;
  mySensorBME.settings.filter = 4;
  mySensorBME.settings.tempOverSample = 5;
  mySensorBME.settings.pressOverSample = 5;
  mySensorBME.settings.humidOverSample = 5;

  delay(10); // BME280 requires 2ms to start up.
  if (mySensorBME.begin() != 0x60) {
    M5.Lcd.println("BME280 error");
    while (1);
  }

  // 初期のリスト状態を取得
  ed = el.getDesc();

  // センサー読み込みスレッド開始
  xTaskCreatePinnedToCore(readSensorThread, "readSensorThread", 4096, NULL, 1, NULL, 1);
}

// メインスレッド ... 画面表示とボタン操作
void loop()
{
  static bool isSleep = false;  //省電力モードか
  static bool isNotice = false; //通知表示中か
  static int dispMode = 0;      //現在の表示モード
  static int prevDispMode = -1; //前回の表示モード
  static unsigned long lastReleasedTime = 0; //最後にボタンを離した時刻(ms)
  static unsigned long lastSleepTime = 0; //最後に眠りに入った時刻(ms)
  static unsigned long noticeStartTime = 0; //通知画面の表示を開始した時刻(ms)

  // ボタン検知と表示モード切替
  bool btnReleased = true;
  M5.update();
  if (M5.BtnA.wasReleased()) {
    dispMode = 0;
  }
  else if (M5.BtnB.wasReleased()) {
    dispMode = 1;
  }
  else if (M5.BtnC.wasReleased()) {
    dispMode = 2;
  }
  else {
    btnReleased = false;
  }

  bool chgMode = (dispMode != prevDispMode); //違うボタンを押された
  bool chgView = (!chgMode && btnReleased) && !isSleep;  //同じボタンを連続して押された

  if (btnReleased) {
    lastReleasedTime = millis();
    if (isSleep) {
      isSleep = false;
      M5.Lcd.wakeup();
      M5.Lcd.setBrightness(100);
    }
  } else if (!isSleep && millis() - lastReleasedTime > 180000) { //３分無操作で、画面OFF
    isSleep = true;
    M5.Lcd.sleep();
    M5.Lcd.setBrightness(0);
    lastSleepTime = millis();
  }

  if (!isSleep) {
    if (el.IsUpdated(ed) || chgMode || chgView) {
      // 測定値画面の更新
      M5.Speaker.mute();
      ed = el.getDesc();

      prevDispMode = dispMode;
      if (dispMode == 0) {
        drawMonitor(chgMode, chgView);
      }
      else if (dispMode == 1) {
        drawGraph(chgMode, chgView);
      }
      else if (dispMode == 2) {
        drawData(chgMode, chgView);
      }
    }
  }
  else if (millis() - lastSleepTime > 60000) { // １分おきに通知
    // CO2通知
    if (!isNotice) {
      envListDsc ed2 = el.getDesc();
      envInfo ei = el.get(ed2, 0);

      co2Lv* lvp = co2LvList;
      do {
        if (ei.CO2 > lvp->minCO2 && lvp->noticeLv > 0) {
          // 通知開始
          M5.Lcd.wakeup();
          M5.Lcd.setBrightness(100);
          M5.Lcd.clear(lvp->clr);
          if (lvp->noticeLv > 1) {
            M5.Speaker.setVolume(2); //通知音の大きさ
            M5.Speaker.tone(1700, 100); //通知音の高さ(Hz)、長さ(ms)
          }
          isNotice = true;
          noticeStartTime = millis();
          prevDispMode = -1;
          break;
        }
      } while ((lvp++)->minCO2 != 0);
    }
    else if (millis() - noticeStartTime > 3000) { //通知画面は３秒表示
      // 通知終了
      M5.Lcd.sleep();
      M5.Lcd.setBrightness(0);
      M5.Lcd.clear(BLACK);
      M5.Speaker.mute();
      isNotice = false;
      lastSleepTime = millis();
    }
  }

  delay(10);
}

// 現在の測定値表示
void drawMonitor(bool chgMode, bool chgView) {
  if (chgView)
    return;

  static bool isWait = false;
  if (ed.getCount() == 0) {
    isWait = true;

    M5.Lcd.clear(BLACK);
    M5.Lcd.setTextFont(4);
    M5.Lcd.setCursor(123, 100);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.printf("Wait ...");
    return;
  } else if (isWait) {
    isWait = false;
    chgMode = true;
  }

  if (chgMode) {
    M5.Lcd.clear(BLACK);
    M5.Lcd.setTextFont(4);

    M5.Lcd.setCursor(0, 8);//P1:
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.printf("Temp");

    M5.Lcd.setCursor(0, 86); //P3: P2+48+3
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.printf("Humidity");

    M5.Lcd.setCursor(0, 164); //P5: P4+48+3
    M5.Lcd.setTextColor(TFT_CYAN);
    M5.Lcd.printf("Pressure");

    M5.Lcd.setCursor(210, 8); //y=P1
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.printf("CO2");

    M5.Lcd.setCursor(262, 73); //+38
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.printf("ppm");

    M5.Lcd.drawRect(240, 108, 50, 120, TFT_WHITE); //CO2グラフの枠 P2+37
  }
  else {
    M5.Lcd.fillRect(2, 35, 318, 39, BLACK); //温度,CO2
    M5.Lcd.fillRect(2, 113, 208, 39, BLACK); //湿度
    M5.Lcd.fillRect(2, 191, 208, 39, BLACK); //気圧
    M5.Lcd.fillRect(241, 109, 48, 118, TFT_BLACK); //CO2グラフの中
  }

  envInfo ei = el.get(ed, 0);

  M5.Lcd.setTextFont(6);
  M5.Lcd.setCursor(2, 35); //P2: P1+27
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.printf("%5.1f", ei.tempC);
  M5.Lcd.setTextFont(2);
  M5.Lcd.printf(" O");
  M5.Lcd.setTextFont(4);
  M5.Lcd.printf("C");

  M5.Lcd.setTextFont(6);
  M5.Lcd.setCursor(2, 113); //P4: P3+27
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.printf("%5.1f", ei.humidity);
  M5.Lcd.setTextFont(4);
  M5.Lcd.printf(" %%");

  M5.Lcd.setTextFont(6);
  M5.Lcd.setCursor(2, 191); //P6: P5+27
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.printf("%6.1f", ei.pressure / 100);
  M5.Lcd.setTextFont(4);
  M5.Lcd.printf(" hPa");

  M5.Lcd.setTextFont(6);
  M5.Lcd.setCursor(210, 35); //y=P2  P1+27
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.printf("%4d", ei.CO2);

  //CO2グラフ
  // 計算誤差で隙間が空くのを防ぐため、重ね合わせて描画
  if (ei.CO2 > 6000) {
    M5.Lcd.fillRect(241, 109, 48, 118, TFT_RED);
  }
  else {
    co2Lv* lvp = co2LvList;
    int maxCO2 = 6000;
    do {
      if (ei.CO2 > lvp->minCO2) {
        int h = (118.0 / 6000.0) * (ei.CO2 > maxCO2 ? maxCO2 : ei.CO2);
        M5.Lcd.fillRect(241, 109 + (118 - h), 48, h, lvp->clr);
      }
      maxCO2 = lvp->minCO2;
    } while ((lvp++)->minCO2 != 0);
  }
}

// グラフ表示
void drawGraph(bool chgMode, bool chgView) {
  //１単位当たりのドット数
  static float pDot1 = 222.0 / 140.0;  // 右軸： 温度,湿度      -40～100
  static float pDot2 = 222.0 / 6000.0; // 左軸： 気圧,CO2,TVOC   0～6000 (6000を超えるとはみ出る仕様)

  static int graphMode = 0;     //0=53.3分モード(1:1)  1=8時間モード(1:9)

  if (chgView) {
    graphMode = !graphMode;
  }

  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextFont(2);

  // 横の基準線
  M5.Lcd.drawFastHLine(0, 222 - (pDot2 * 0), 319, TFT_DARKGREY); //0ppm

  for (int x = 0; x < 320; x += 5) {
    // 右軸
    M5.Lcd.drawFastHLine(x, 222 - (pDot1 * (0  + 40)), 4, TFT_PURPLE); //0℃
    M5.Lcd.drawFastHLine(x, 222 - (pDot1 * (40 + 40)), 2, TFT_PURPLE); //40℃
    M5.Lcd.drawFastHLine(x, 222 - (pDot1 * (80 + 40)), 2, TFT_PURPLE); //80℃

    // 左軸
    for (int val = 1000; val <= 5000; val += 1000) {
      M5.Lcd.drawFastHLine(x, 222 - (pDot2 * val), 2, TFT_DARKGREY);
    }
  }

  // タイトル
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.printf("Temp, ");
  M5.Lcd.setTextColor(TFT_BLUE);
  M5.Lcd.printf("Humidity");

  M5.Lcd.setCursor(319 / 2 - (graphMode ? 20 : 25), 0);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.printf(graphMode ? "8h" : "53m");

  M5.Lcd.setCursor(319 - 127, 0);
  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.printf("Pressure, ");
  M5.Lcd.setTextColor(TFT_GREEN);
  M5.Lcd.printf("CO2, ");
  M5.Lcd.setTextColor(TFT_YELLOW);
  M5.Lcd.printf("TVOC");

  // 左軸目盛り
  M5.Lcd.setTextColor(TFT_PURPLE);
  M5.Lcd.setCursor(0, 222 - 15);
  M5.Lcd.printf("-40");

  M5.Lcd.setCursor(0, 222 - (pDot1 * (0 + 40)) - 15);
  M5.Lcd.printf("0");

  M5.Lcd.setCursor(0, 222 - (pDot1 * (40 + 40)) - 15);
  M5.Lcd.printf("40");

  M5.Lcd.setCursor(0, 222 - (pDot1 * (80 + 40)) - 15);
  M5.Lcd.printf("80");

  // 右軸目盛り
  M5.Lcd.setTextColor(TFT_DARKGREY);
  M5.Lcd.setCursor(319 - 8, 222 - 15);
  M5.Lcd.printf("0");

  for (int val = 1000; val <= 5000; val += 1000) {
    M5.Lcd.setCursor(319 - 32, 222 - (pDot2 * val) - 15);
    M5.Lcd.printf("%d", val);
  }

  // 下目盛りと縦の基準線
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(319 - 8, 239 - 15);
  M5.Lcd.printf("0");

  if (graphMode) {
    for (int y = 0; y < 222; y += 5) {
      M5.Lcd.drawFastVLine(319 -  80, y, 2, TFT_DARKGREY);
      M5.Lcd.drawFastVLine(319 - 160, y, 2, TFT_DARKGREY);
      M5.Lcd.drawFastVLine(319 - 240, y, 2, TFT_DARKGREY);
    }
    M5.Lcd.setCursor(319 -  80 - 12, 239 - 15);
    M5.Lcd.printf("-2h");

    M5.Lcd.setCursor(319 - 160 - 12, 239 - 15);
    M5.Lcd.printf("-4h");

    M5.Lcd.setCursor(319 - 240 - 12, 239 - 15);
    M5.Lcd.printf("-6h");
  }
  else {
    for (int y = 0; y < 222; y += 5) {
      M5.Lcd.drawFastVLine(319 -  90, y, 2, TFT_DARKGREY);
      M5.Lcd.drawFastVLine(319 - 180, y, 2, TFT_DARKGREY);
      M5.Lcd.drawFastVLine(319 - 270, y, 2, TFT_DARKGREY);
    }
    M5.Lcd.setCursor(319 -  90 - 16, 239 - 15);
    M5.Lcd.printf("-15m");

    M5.Lcd.setCursor(319 - 180 - 16, 239 - 15);
    M5.Lcd.printf("-30m");

    M5.Lcd.setCursor(319 - 270 - 16, 239 - 15);
    M5.Lcd.printf("-45m");
  }

  // グラフ線
  int no;
  int x = 319;
  envInfo prevEi;

  el.resetNext(ed, graphMode ? 9 : 1);
  while ((no = el.getNext()) != -1) {
    envInfo ei = el.get(ed, no);

    if (x < 319) {
      M5.Lcd.drawLine(x + 1, 222 - (pDot1 * (prevEi.tempC + 40)),    x , 222 - (pDot1 * (ei.tempC + 40)),    TFT_RED); //温度
      M5.Lcd.drawLine(x + 1, 222 - (pDot1 * (prevEi.humidity + 40)), x , 222 - (pDot1 * (ei.humidity + 40)), TFT_BLUE); //湿度
      M5.Lcd.drawLine(x + 1, 222 - (pDot2 * prevEi.pressure / 100),  x , 222 - (pDot2 * ei.pressure / 100),  TFT_CYAN); //気圧
      M5.Lcd.drawLine(x + 1, 222 - (pDot2 * prevEi.CO2),             x , 222 - (pDot2 * ei.CO2),             TFT_GREEN); //CO2
      M5.Lcd.drawLine(x + 1, 222 - (pDot2 * prevEi.TVOC),            x , 222 - (pDot2 * ei.TVOC),            TFT_YELLOW); //TVOC
    }

    if (--x < 0) {
      break;
    }

    prevEi = ei;
  }
}

// データ表示モード
void drawData(bool chgMode, bool chgView) {
  if (chgView)
    return;

  M5.Lcd.setTextFont(2); // Small 16 pixel high font   8x16

  if (chgMode) {
    M5.Lcd.clear(BLACK);

    // header
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("No   Temp     Humidity    Pressure   CO2   TVOC");
  }
  else {
    M5.Lcd.fillRect(0, 17, 319, 239 - 17, BLACK);
  }

  // values
  int max = ed.getCount();
  if (max > 13)
    max = 13;

  for (int i = 0; i < max; i++) {
    envInfo ei = el.get(ed, i);
    int y = 17 * (i + 1);

    M5.Lcd.setTextColor((el.getInternalNo(ed, i) % 9) == 0 ?  TFT_ORANGE : TFT_WHITE);

    M5.Lcd.setCursor(0, y);
    M5.Lcd.printf("%2d", i);

    M5.Lcd.setCursor(8 * 3, y);
    M5.Lcd.printf("%6.2fC,", ei.tempC);

    M5.Lcd.setCursor(8 * 11, y);
    M5.Lcd.printf("%6.2f%%,", ei.humidity);

    M5.Lcd.setCursor(8 * 19, y);
    M5.Lcd.printf("%7.2fhPa,", ei.pressure / 100);

    M5.Lcd.setCursor(8 * 30, y);
    M5.Lcd.printf("%4d,", ei.CO2);

    M5.Lcd.setCursor(8 * 35, y);
    M5.Lcd.printf("%4d", ei.TVOC);
  }
}

// 測定値読み込みスレッド
void readSensorThread(void* arg) {
  while (1) {
    if (mySensorCCS.dataAvailable()) {
      mySensorCCS.readAlgorithmResults();

      // 測定値の格納
      envInfo ei = envInfo(
                     mySensorBME.readTempC(),          // -40C to +85C
                     mySensorBME.readFloatHumidity(),  //   0% to 100%
                     mySensorBME.readFloatPressure(),  // 300hPa to 1100hPa
                     mySensorCCS.getCO2(),             // 400ppm to 8192ppm
                     mySensorCCS.getTVOC()             //   0ppb to 1187ppb
                   );
      el.put(ei);

      // 補正値のセット
      mySensorCCS.setEnvironmentalData(ei.humidity, ei.tempC);
    }
    else if (mySensorCCS.checkForStatusError()) {
      M5.Lcd.printf("CCS.dataAvailable error: %d\n", mySensorCCS.getErrorRegister());
      vTaskSuspendAll();
      while (1);
    }
    delay(10);
  }
}
