import cv2
import serial
import time
import numpy as np
import sys

# --- 1. 初期設定 ---

# Arduinoとシリアル通信を開始
try:
    ser = serial.Serial('/dev/ttyS0', 9600, timeout=1)
    print("Arduinoと接続しました: /dev/ttyS0")
except serial.SerialException as e:
    print(f"警告: シリアルポート接続エラー: {e}")
    # 接続に失敗した場合のダミー設定
    class DummySerial:
        def in_waiting(self): return 0
        def read(self): return b''
        def write(self, data): pass
        def close(self): pass
    ser = DummySerial()

# カメラ起動
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("エラー: カメラを開けません")
    sys.exit()

# 制御パラメータ
DEAD_BAND = 50

# --- 2. メインループ ---

while True:
    ret, frame = cap.read()
    if not ret:
        time.sleep(0.01)
        continue

    # 画像処理
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    _, binary = cv2.threshold(gray, 60, 255, cv2.THRESH_BINARY_INV)
    # ROI抽出
    roi = binary[frame.shape[0]//2:, :]

    # 輪郭検出
    contours, _ = cv2.findContours(roi, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    command = b'S' # デフォルトは停止
    cx = None

    if contours:
        largest = max(contours, key=cv2.contourArea)
        M = cv2.moments(largest)

        if M['m00'] != 0:
            cx = int(M['m10'] / M['m00']) # 重心X座標
            center = frame.shape[1] // 2 # フレームの中心X座標

            # 制御指令の決定 (±50デッドバンド適用)
            if cx < center - DEAD_BAND:
                command = b'L' # 左へ
                # print("L")
            elif cx > center + DEAD_BAND:
                command = b'R' # 右へ
                # print("R")
            else:
                command = b'F' # 前進
                # print("F")
        else:
            command = b'S' # 重心計算不能 → 停止
            # print("S1")
    else:
        command = b'S' # 黒線なし → 停止
        # print("S2")

    # 指令送信 (シリアル接続が成功している場合)
    if ser:
        ser.write(command)

    # --- 3. デバッグ表示 --
    # 元のフレームに指令、重心、中央線を描画
    cv2.putText(frame, f"CMD: {command.decode()}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
    if cx is not None:
        # 重心位置の描画
        cy = frame.shape[0] // 2 + int(M['m01'] / M['m00'])
        cv2.circle(frame, (cx, cy), 10, (0, 0, 255), -1)
        # 中央線の描画
        cv2.line(frame, (center, 0), (center, frame.shape[0]), (255, 0, 0), 1)

    cv2.imshow("Original", frame)
    cv2.imshow("Binary ROI", roi)

    # 'q'キーで終了
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# --- 4. 終了処理 ---
cap.release()
cv2.destroyAllWindows()
if ser:
    ser.write(b'S') # 終了時にも停止指令
    ser.close()