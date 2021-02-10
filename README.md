# AirMonitor
M5Stack Air Quality monitor

## 概要/overview
M5Stackを使って、空気環境を測定するデバイスを作りました。<BR>
CCS811,BME280という２つのセンサーを使って下記の５項目を測定し、M5Stackの画面に表示します。
- 温度 (摂氏)
- 湿度 (%)
- 気圧 (hPa、ヘクトパスカル)
- CO2 (ppm)
- TVOC (ppb)
  
下記３つの表示モードがあります。
- 現在値モニタ<BR>
  現在の測定値を表示。
- グラフ<BR>
  過去53分間/8時間のデータを折れ線グラフで表示。
- データ<BR>
  直前13回分の測定値データを一覧表示。


### パーツリスト/parts list
* M5Stack GRAY<BR>
  M5Stack Basicでも大丈夫と思うけど未確認。
* SparkFun Environmental Combo Breakout - CCS811/BME280 (Qwiic)<BR>
  https://www.sparkfun.com/products/14348
* Qwiic Cable - Grove Adapter (100mm)<BR>
  https://www.sparkfun.com/products/15109<BR>
  私はブレッドボードを使いましたが、このケーブルでいけるはず。。
  
### 組み立て方/How to build
1. Arduino IDEをセットアップ<BR>

2. 
