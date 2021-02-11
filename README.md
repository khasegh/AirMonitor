# AirMonitor
Air Quality monitor for M5Stack

## 概要/Overview
M5Stackを使って、空気環境を測定するデバイスを作りました。<BR>
CCS811,BME280という２つのセンサーを使って下記の５項目を測定し、M5Stackの画面に表示します。<BR>
- 温度/Temperature [℃, 摂氏]
- 湿度/Humidity [%]
- 気圧/Pressure [hPa, ヘクトパスカル]
- 二酸化炭素相当物/eCO2 [ppm]
- 総揮発性有機化合物/TVOC [ppb]
  
測定値はM5Stackの液晶画面に表示され、下記３つの表示モードがあります。
- 現在値モニタ<BR>
  現在の測定値を表示。
- グラフ<BR>
  過去53分間/8時間のデータを折れ線グラフで表示。
- データ<BR>
  直前13回分の測定値を一覧表示。
  
wifiネットワークは不要で、電源を繋いでおけば動作します。<BR>

### パーツリスト/Parts list
* M5Stack Basic/GRAY<BR>
  私はGRAYを使いましたが、Basicでもいけるはず。。未確認
* SparkFun Environmental Combo Breakout - CCS811/BME280 (Qwiic)<BR>
  https://www.sparkfun.com/products/14348
* Qwiic Cable - Grove Adapter (100mm)<BR>
  https://www.sparkfun.com/products/15109<BR>
  私はブレッドボードを使いましたが、このケーブルでいけるはず。。未確認
* 適当なUSB Type-C電源<BR>
  M5Stackへの電源供給用。
  
### 組み立て方/How to build
1. Arduino IDEをセットアップ<BR>
  Arduino IDEをインストールし、COMポートを設定します。

2. M5Stack、センサー用ライブラリのインストール<BR>
  Arduino IDEにて、下記３つのライブラリをインストールします。<BR>
   - M5Stack by M5Stack<BR>
     https://github.com/m5stack/m5stack
   - SparkFun BME280 Arduino Library by SparkFun Electronics<BR>
     https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
   - SparkFun CCS811 Arduino Library by SparkFun Electronics<BR>
     https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library
  
  
