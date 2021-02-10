/******************************************************************************

  M5Stack Air Quality monitor
  https://github.com/khasegh/AirMonitor

******************************************************************************/
#ifndef ENVLIST_H_INCLUDE
#define ENVLIST_H_INCLUDE

// 測定値を保存するクラス

#define LIST_SIZE 3006  //リストサイズ(10以上、9の倍数)
const float _referencePressure = 101325.0; // sea level pressure

// 測定値情報
class envInfo
{
  public:
    float         tempC;
    float         humidity;
    float         pressure;
    unsigned int  CO2;
    unsigned int  TVOC;

    envInfo() {
      tempC   = 0;
      humidity = 0;
      pressure = 0;
      CO2     = 0;
      TVOC    = 0;
    }

    envInfo(float t, float h, float p, unsigned int c, unsigned int v) {
      tempC   = t;
      humidity = h;
      pressure = p;
      CO2     = c;
      TVOC    = v;
    }

    float calcAltitudeMeters(void) {
      return ((float) - 44330.77) * (pow(((float)pressure / (float)_referencePressure), 0.190263) - (float)1);
    }
};

// リスト状態
class envListDsc
{
  private:
    int _start; //開始位置
    int _count; //格納数

  public:
    envListDsc() {
      _start = -1;
      _count = -1;
    }

    envListDsc(int s, int c) {
      _start = s;
      _count = c;
    }

    int getStart() {
      if (_start == -1) {
        M5.Lcd.println("envListDsc.getStart error");
        vTaskSuspendAll();
        while (1);
      }
      return _start;
    }
    int getCount() {
      if (_count == -1) {
        M5.Lcd.println("envListDsc.getCount error");
        vTaskSuspendAll();
        while (1);
      }
      return _count;
    }
};

// 測定値リスト
class envList {
  private:
    envInfo _envInfoList[LIST_SIZE]; //測定値保存配列
    int _start_num = 0;              //格納開始位置
    int _count = 0;                  //格納数

    envListDsc _nextDsc;            //next取得時のリスト状態
    int _nextStep = 1;              //next取得間隔
    int _nextCount = -1;            //現在のnext番号

    SemaphoreHandle_t _xMutex = NULL;//排他処理用

  public:
    // コンストラクタ
    envList() {
      _xMutex = xSemaphoreCreateMutex();
    }

    // 現在のリスト状態を返す
    envListDsc getDesc() {
      if (xSemaphoreTake(_xMutex, portMAX_DELAY) != pdTRUE) {
        M5.Lcd.println("envList.getDesc: mutex error");
        vTaskSuspendAll();
        while (1);
      }

      envListDsc retDsc = envListDsc(_start_num, _count);
      xSemaphoreGive(_xMutex);
      return retDsc;
    }

    // リストが更新されたか
    bool IsUpdated(envListDsc ld) {
      return (_start_num != ld.getStart() || _count != ld.getCount());
    }

    // 測定値の格納
    void put(envInfo ei) {
      if (xSemaphoreTake(_xMutex, portMAX_DELAY) != pdTRUE) {
        M5.Lcd.println("envList.put: mutex error");
        vTaskSuspendAll();
        while (1);
      }

      _envInfoList[(_start_num + _count) % LIST_SIZE] = ei;
      if (++_count > LIST_SIZE) {
        _count = LIST_SIZE;

        if (++_start_num == LIST_SIZE) {
          _start_num = 0;
        }
      }

      xSemaphoreGive(_xMutex);
    }

    // 番号指定での測定値取得
    envInfo get(envListDsc ld, int getNo) {
      if (getNo < 0 || getNo > LIST_SIZE || getNo > ld.getCount() || getNo > LIST_SIZE - 10) {
        M5.Lcd.printf("envList.get: getNo error - getNo=%d\n", getNo);
        vTaskSuspendAll();
        while (1);
      }

      return _envInfoList[getInternalNo(ld, getNo)];
    }

    // next取得をリセット
    void resetNext(envListDsc ld, int skipCnt) {
      _nextDsc = ld;
      _nextStep = skipCnt;
      _nextCount = -1;
    }

    // 次のnext番号を返す
    int getNext() {
      if (_nextCount < 0) {
        //最初のNoを探す
        for (int i = 0; i < _nextDsc.getCount(); i++) {
          if ((getInternalNo(_nextDsc, i) % _nextStep) == 0) {
            _nextCount = i; //発見
            break;
          }
        }
      }
      else {
        //次のNoを返す
        if ((_nextCount + _nextStep) < _nextDsc.getCount()) {
          _nextCount += _nextStep;
        }
        else {
          _nextCount = -1; //終わりに達した
        }
      }
      return (_nextCount);
    }

    // 内部管理番号を返す
    int getInternalNo(envListDsc ld, int no) {
      return ((ld.getStart() + ld.getCount() - no - 1) % LIST_SIZE);
    }
};

#endif
