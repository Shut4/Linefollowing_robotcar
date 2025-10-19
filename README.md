# Raspberry Pi x Arduino 4WD Linefollowing_robotcar
初めてそれっぽいのが出来たのでうれしい。
ただ、以下の問題があるため、改善の余地あり。
* カメラの画素数やデッドバンドの調整の関係で画像処理精度がまだ低い
* モーター速度の左右差等の微調整がまだ出来ておらず、ラインを外れたりするときがある。
* 回路と配線に関しても無駄が多く、まだ効率よく出来る。
* ボタンを押していなくても勝手にスタートすることがある。

Raspberry Pi（OpenCV/Python）でカメラ映像を処理し、Arduino Unoで4WDモーターを制御するライントレーサーロボットのファームウェアと制御コードを格納してます。
Raspberry Piはラインの重心を検出して制御コマンド（F, L, R, S）をシリアル通信で送信し、Arduinoがモーターを駆動します。

## 1. システム構成と機能

| 項目 | 詳細 |
| :--- | :--- |
| **プラットフォーム** | Raspberry Pi 4|
| **制御ボード** | Arduino Uno |
| **駆動方式** | 4輪駆動 (4WD) |
| **制御ロジック** | P制御（デッドバンド ±50 pixel） |
| **通信** | シリアル通信 (UART) |
| **センサー** | USBカメラ、PSD距離センサー (A0) (テスト時には使用しませんでした)|

## 2. ハードウェア接続の概要

Raspberry PiとArduino間の通信は、ロジックレベルコンバーターがなかったので1kΩと2.2kΩの抵抗分圧回路を使用しました。

### シリアル通信ピン配置

| 機能 | Raspberry Pi (GPIO BCM) | Arduino Uno (デジタルピン) | 備考 |
| :--- | :--- | :--- | :--- |
| **TX** (送信) | GPIO 14 (Pin 8) | Pin 0 (RX) | Piの3.3V → ArduinoのRX (抵抗で電流制限) |
| **RX** (受信) | GPIO 15 (Pin 10) | Pin 1 (TX) | Arduinoの5V → PiのRXD (**抵抗分圧必須**) |
| **GND** | GND (Pin 6) | GND | **共通グランド接続必須** |

### 4WDモーター制御ピン

このシステムは8本のピンで4つのモーターを制御します。

| モーター | 前進 (A) | 後退 (B) |
| :--- | :---: | :---: |
| **左前輪** | Pin 9 (`LM2_A`) | Pin 11 (`LM2_B`) |
| **右前輪** | Pin 3 (`RM2_A`) | Pin 2 (`RM2_B`) |
| **左後輪** | Pin 6 (`LM_A`) | Pin 7 (`LM_B`) |
| **右後輪** | Pin 5 (`RM_A`) | Pin 4 (`RM_B`) |

---

## 3. ソフトウェアのセットアップと実行

### A. Raspberry Pi側のセットアップ (`raspi_control.py`)

1.  **環境構築**: Python3と`pip`、`opencv-python`、`pyserial`をインストールします。
    ```bash
    pip install opencv-python pyserial numpy
    ```
2.  **シリアルポート設定**: `sudo raspi-config` でシリアルコンソールを無効化し、ハードウェアUARTを有効にします。
3.  **コードの配置**: リポジトリをクローンし、`raspi_control.py`を実行権限のあるディレクトリに配置します。

### B. Arduino側のセットアップ (`arduino_4wd.ino`)

1.  **ライブラリ**: `Adafruit NeoPixel` ライブラリが必要です。（NeoPixelを使用しない場合はコードから削除可能）
2.  **アップロード**: Arduino IDEでスケッチを開き、ボードにアップロードします。

### C. 自動実行 (ヘッドレスモード)

PCから切り離した後に自動で実行するには、`systemd`サービスを設定します。

1.  **サービスファイル作成**: `/etc/systemd/system/robot.service`を作成し、`ExecStart`にPythonスクリプトのフルパスを設定します。
2.  **サービス有効化**:
    ```bash
    sudo systemctl daemon-reload
    sudo systemctl enable robot.service
    sudo systemctl start robot.service
    ```

## 4. 制御パラメータ

| パラメータ | 値 | 役割 | 調整のポイント |
| :--- | :---: | :--- | :--- |
| **デッドバンド** | `50` | 中央維持の許容誤差（ピクセル） | ロボットがフラフラする場合、値を大きくします。 |
| **二値化閾値** | `60` | 黒線の検出基準 | 周囲の明るさやラインの色に合わせて調整します。 |
| **前進速度** |  | `moveForward`実行時の速度（Arduino側） | ロボットが速すぎると判断が追いつかないため、最初に低く設定します。 |

<img width="983" height="840" alt="image" src="https://github.com/user-attachments/assets/4997f662-c3a2-4bf2-8101-adbbb86b67b5" />
<img width="1192" height="673" alt="image" src="https://github.com/user-attachments/assets/653e9e47-1019-4085-ba2e-182755486fc5" />
<img width="780" height="729" alt="image" src="https://github.com/user-attachments/assets/d88dcd5a-4620-453b-b6f9-a20929b9d670" />

※L298NのGNDが分からなかったので回路図上では切り離しています。実際にはモータドライバ（Maker Drive）を使用していました。
