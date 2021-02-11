# AirMonitor
Air Quality monitor for M5Stack<BR>
本ページに記載された内容、およびプログラムコードは無保証です。

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
在宅や職場などで、スタンドアローンで空気環境をチェックしたいときにどうぞ。

## パーツリスト/Parts list
* M5Stack Basic/GRAY<BR>
  私はGRAYを使いましたが、Basicでもいけるはず。。未確認
* SparkFun Environmental Combo Breakout - CCS811/BME280 (Qwiic)<BR>
  https://www.sparkfun.com/products/14348
* Qwiic Cable - Grove Adapter (100mm)<BR>
  https://www.sparkfun.com/products/15109<BR>
  私はブレッドボードを使いましたが、このケーブルでいけるはず。。未確認
* 適当なUSB Type-C電源<BR>
  M5Stackへの電源供給用。
  
## 組み立て方/How to build
1. Arduino IDEをセットアップ<BR>
  Arduino IDEをインストールしてM5Stackとの通信COMポートを設定した後、必要なライブラリをインストールします。
   - M5Stack by M5Stack<BR>
     https://github.com/m5stack/m5stack
   - SparkFun BME280 Arduino Library by SparkFun Electronics<BR>
     https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
   - SparkFun CCS811 Arduino Library by SparkFun Electronics<BR>
     https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library
2. スケッチのダウンロード<BR>
  本ページからairMon.ino、envList.hをダウンロードした後、"airMon"という名前のフォルダに入れます。
3. ハードウェアの接続<BR>
   - M5Stackとセンサーを、Qwiic Cableにて接続
   - Arduino IDEをインストールしたパソコンのUSBポートに、M5Stackを接続
4. スケッチのコンパイルと、M5Stackへの書き込み<BR>
  airMon.inoをダブルクリックしてArduino IDEにて開いた後、「マイコンボードに書き込む」を実行。
  
## 使い方/How to use
### 電源オン、オフ
* M5Stackの電源ボタンを押すと、電源が入ります。<BR>
  「Wait...」と数秒表示されたあと、現在値モニタ画面が表示されます。
* M5Stackの電源ボタンを２回続けて押すと、電源が切れます。<BR>
  USBにて電源が供給されている場合、電源を切ることは出来ません。
### 表示モード切り替え
  前面ボタンを押すと、表示モードが切り替わります。
