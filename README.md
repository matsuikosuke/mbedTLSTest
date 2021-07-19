# mbedTLSTest

## プロジェクトファイル
SEGGER Embedded Studioを用いて以下のファイルを開いてください。
\mbedTLSTest\mbedTLSTest\ble_peripheral\project\pca10040\s132\sesmbedTLSTest_s132.emProject

プロジェクトはnRF52832を対象にしていますが、他のnRF52シリーズを使用する場合は設定を変更ください。

nRFマイコンの環境構築方法については、以下をご参照ください。

[Nordic社製BLEの開発環境構築と動作確認](https://qiita.com/Kosuke_Matsui/items/5d61ce77e928b9f117cc)

## 追加ファイル
Noridicの公式サイトからSDKやソフトデバイスをダウンロードしてください。

ソフトデバイスはs132_nrf52_7.2.0_softdevice.hexを使用しています。

SDKの以下のフォルダを\mbedTLSTestの階層に置いてください。
もし別の階層に置きたい場合は、プロジェクトファイルを編集して、自分の環境に合わせてファイルの場所を修正してください

- components
- external
- integration
- modules
