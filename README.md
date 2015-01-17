AVRを読み書きするやつ
======

AVRマイコンのプログラムメモリやEEPROMを読み書きするライブラリです。

AVRのプログラムの開発には、
[AVRのプログラムを書くやつ(仮)](https://github.com/mikecat/avr_devel)などが利用できます。

### 参考資料
* [Atmel AVR 8-bit and 32-bit Microcontrollers](http://www.atmel.com/products/microcontrollers/avr/?tab=documents)  
  このページの"ATmega48A/PA/88A/PA/168A/PA/328/P Complete"に読み書きの方法、huseの設定の意味などが載っています。

### ATmega328Pの仕様メモ(上記参考資料より)

FuseやLockは1がunprogrammed、0がprogrammed

#### Lock bits
|Bit7|Bit6|Bit5 |Bit4 |Bit3 |Bit2 |Bit1|Bit0|
|----|----|-----|-----|-----|-----|----|----|
|-   |-   |BLB12|BLB11|BLB02|BLB01|LB2 |LB1 |

* LB
  * 11 : ロック無し
  * 10 : FlashとEEPROM書き込み禁止、Fuse bitsロック
  * 00 : FlashとEEPROM読み書き禁止、Boot Lock BitsとFuse bitsロック
* BLB02
  * 1 : 制限なし
  * 0 : ブートローダーからのLPM(Load Program Memory命令)によるアプリケーション読み込み禁止、
         Interrupt Vectorsがブートローダーにある場合はアプリケーションでの割り込み無効
* BLB01
  * 1 : 制限なし
  * 0 : アプリケーションへのSPMでの書き込み禁止
* BLB12
  * 1 : 制限なし
  * 0 : アプリケーションからのLPM(Load Program Memory命令)によるブートローダー読み込み禁止、
         Interrupt Vectorsがアプリケーションにある場合はブートローダーでの割り込み無効
* BLB11
  * 1 : 制限なし
  * 0 : ブートローダーへのSPMでの書き込み禁止


#### Extended Fuse Byte
|Bit7|Bit6|Bit5|Bit4|Bit3|Bit2     |Bit1     |Bit0     |
|----|----|----|----|----|---------|---------|---------|
|-   |-   |-   |-   |-   |BODLEVEL2|BODLEVEL1|BODLEVEL0|

|Value|Min |Typ |Max |
|-----|----|----|----|
|110  |1.7V|1.8V|2.0V|
|101  |2.5V|2.7V|2.9V|
|100  |4.1V|4.3V|4.5V|

* 111 : BOD無効
* 011、010、001、000 : 予約

#### Fuse High Byte
|Bit7    |Bit6|Bit5 |Bit4 |Bit3  |Bit2   |Bit1   |Bit0   |
|--------|----|-----|-----|------|-------|-------|-------|
|RSTDISBL|DWEN|SPIEN|WDTON|EESAVE|BOOTSZ1|BOOTSZ0|BOOTRST|

* RSTDISBL
  * 1 : PC6をリセットピンとして利用し、そこのI/Oは無効
  * 0 : PC6を普通のI/Oピンにし、リセットピンは無し
* DWEN
  * 1 : debugWIRE無効?
  * 0 : Lock bitsがprogramされていない時、debugWIRE有効
* SPIEN
  * 1 : シリアルプログラミング無効?
  * 0 : シリアルプログラミング有効
* WDTON
  * 1 : WDTタイムアウト時の動作をWDEとWDIEで設定
  * 0 : WDTタイムアウト時はリセットする
* EESAVE
  * 1 : Chip Erase時にEEPROMのデータを消す
  * 0 : Chip Erase時もEEPROMのデータを残す
* BOOTSZ
  * 11 : 0x3F00 - 0x3FFF (256 words)
  * 10 : 0x3E00 - 0x3FFF (512 words)
  * 01 : 0x3C00 - 0x3FFF (1024 words)
  * 00 : 0x3800 - 0x3FFF (2048 words)
* BOOTRST
  * 1 : リセット時にアプリケーションの先頭(0x0000)から実行
  * 0 : リセット時にブートローダーの先頭から実行

#### Fuse Low Byte
|Bit7  |Bit6 |Bit5|Bit4|Bit3  |Bit2  |Bit1  |Bit0  |
|------|-----|----|----|------|------|------|------|
|CKDIV8|CKOUT|SUT1|SUT0|CKSEL3|CKSEL2|CKSEL1|CKSEL0|

* CLDIV8
  * 1 : CLKPSを0000(クロックをPrescalerで1/1に)にする
  * 0 : CLKPSを0011(クロックをPrescalerで1/8に)にする
* CKOUT
  * 1 : PORTB0をI/Oとして使う?
  * 0 : 分周されたシステムクロックをCLKO(PORTB0)に出力する
* SUT
  * 00 : BODが有効の時用
  * 01 : 電源の立ち上がりが速い時用
  * 10 : 電源の立ち上がりが遅い時用
  * 11 : 予約
* CKSEL
  * 0000 : 外部クロック(XTAL1、0-20MHz)
  * 0001 : 予約
  * 0010 : 内部RC発振、7.3-8.1MHz
  * 0011 : 内部RC発振、128kHz
  * 010x : Low Frequency 水晶発振子 (時計用32.768kHz水晶)
    * 0100 : 通常
    * 0101 : 起動時に周波数を安定させる
  * 011y : Full Swing 水晶発振子(0.4-20MHz, 推奨容量 = 12-22pF)
    * y=0 / y=1, SUT=00 : セラミック発振子
    * y=1, SUT!=00 : 水晶発振子
  * 1xxy : Low Power 水晶発振子
    * 100y : 0.4-0.9MHz, 推奨容量 = -
    * 101y : 0.9-3.0MHz, 推奨容量 = 12-22pF
    * 110y : 3.0-8.0MHz, 推奨容量 = 12-22pF
    * 111y : 8.0-16.0MHz, 推奨容量 = 12-22pF
    * y=0 / y=1, SUT=00 : セラミック発振子
    * y=1, SUT!=00 : 水晶発振子
