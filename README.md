# AirMonitor
Air Quality monitor for M5Stack<BR>
本ページに記載された内容、および プログラムコードは無保証です。

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
 ![test](image/無題.png) 
## 使い方/How to use
### 電源オン、オフ
* M5Stackの電源ボタンを押すと、電源が入ります。<BR>
  「Wait...」と数秒表示されたあと、現在値モニタ画面が表示されます。
* M5Stackの電源ボタンを２回続けて押すと、電源が切れます。<BR>
  USBにて電源が供給されている場合、電源を切ることは出来ません。
### 表示モード切り替え
  前面ボタンを押すと、表示モードが切り替わります。
#### 左ボタン - 現在値モニタモード<BR>
  最も最近に測定した値が表示されます。<BR>
  右側の棒グラフはCO2レベルを表わしたもので、下記のように色が変化します。<BR>
  基準は、日本産業衛生学会様のホームページを参考にしました。<BR>
  http://jsoh-ohe.umin.jp/covid_simulator/covid_simulator.html
  - ～1000ppm: 緑 - 良い
  - ～1500ppm: 黄色 - やや良い
  - ～2500ppm: オレンジ - 悪い
  - ～3500ppm: ピンク - 非常に悪い
  - ～6000ppm: 赤 - 極めて悪い
  - 6000ppm～: 棒グラフ全体が赤くなります
#### 中央ボタン - グラフモード<BR>
  過去の測定値を折れ線グラフで表示します。<BR>
  右端が最も最近の測定値であり、左に行くほど古い測定値になります。<BR>
  連続して押すと、表示幅が53分/8時間で切り替わります。
#### 右ボタン - データモード<BR>
  直前13回分の測定値を一覧表示します。<BR>
  オレンジ色のデータは、8時間幅でグラフを表示した時に選択される値です。<BR>
  一番上(No=0)が最も最近に測定した値であり、下に行くほど古い測定値になります
### その他
  * 何もボタンを押さずに３分経過すると省電力モードになり、バックライトも消灯します。
  * 空気環境の測定は、10秒毎に行っています。

## 技術的なこと/Technical notes
- データ領域は循環して使用するので、リング型のバッファとしています。
- バッファサイズは3006個であり、10秒毎に測定するので10x3006で8.35時間分の測定値を内部メモリーに保存しています。
- 測定値を漏れなく記録するため、測定値を記録するスレッドと画面表示スレッドは別に分けて動作しています。
- スレッド間で競合する部分は、排他処理としています。
- CCS811とBME280が一緒になったコンボボードではなく、別になったボードをデイジーチェーン接続しても<BR>
  動作するかもしれない。その場合は、I2Cアドレス(CSS811_ADDR)の定義を変更すると良いかも。
