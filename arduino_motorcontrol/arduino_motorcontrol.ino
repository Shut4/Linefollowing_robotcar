#include <Adafruit_NeoPixel.h>

// --- 制御ピン定義 ---

// 左前輪 / 右前輪
const int LM2_A = 9;
const int LM2_B = 11;
const int RM2_A = 3;
const int RM2_B = 2;

// 左後輪 / 右後輪
const int LM_A = 6;
const int LM_B = 7;
const int RM_A = 5;
const int RM_B = 4;

// 入力ピン
const int BUTTON_PIN = 8;
const int PSD_SENSOR = A0;
const int SPEAKER = 12;

// 状態定義
#define WAKEUP 1
#define ANGER 2
#define HAPPY 3

// 制御定数
const int DISTANCE_THRESHOLD = 400;
const int MOTOR_POWER = 200; // PWM出力（0〜255）

// --- モーター制御関数 ---
//小回りきかせたかったので2WD RRにした
void moveForward() {
  analogWrite(LM_A, MOTOR_POWER); analogWrite(LM_B, 0);
  analogWrite(LM2_A, MOTOR_POWER); analogWrite(LM2_B, 0);
  analogWrite(RM_A, MOTOR_POWER); analogWrite(RM_B, 0);
  analogWrite(RM2_A, MOTOR_POWER); analogWrite(RM2_B, 0);
}

void turnLeft() {
  analogWrite(LM_A, 0); analogWrite(LM_B, 0);
  analogWrite(LM2_A, 0); analogWrite(LM2_B, 0);
  analogWrite(RM_A, MOTOR_POWER); analogWrite(RM_B, 0);
  analogWrite(RM2_A, MOTOR_POWER); analogWrite(RM2_B, 0);
}

void turnRight() {
  analogWrite(LM_A, MOTOR_POWER); analogWrite(LM_B, 0);
  analogWrite(LM2_A, MOTOR_POWER); analogWrite(LM2_B, 0);
  analogWrite(RM_A, 0); analogWrite(RM_B, 0);
  analogWrite(RM2_A, 0); analogWrite(RM2_B, 0);
}

void stopBreak() {
  analogWrite(LM_A, 0); analogWrite(LM_B, 0);
  analogWrite(LM2_A, 0); analogWrite(LM2_B, 0);
  analogWrite(RM_A, 0); analogWrite(RM_B, 0);
  analogWrite(RM2_A, 0); analogWrite(RM2_B, 0);
}

// --- フィードバック関数 ---

void sing(int emotion) {
  if (emotion == WAKEUP) {
    tone(SPEAKER, 1047, 200); delay(200);
    tone(SPEAKER, 1319, 200); delay(200);
    tone(SPEAKER, 1175, 400); delay(400);
  } else if (emotion == ANGER) {
    tone(SPEAKER, 247, 200); delay(200);
    tone(SPEAKER, 247, 200); delay(200);
  } else if (emotion == HAPPY) {
    tone(SPEAKER, 1760, 100); delay(100);
    tone(SPEAKER, 1976, 100); delay(100);
  }
}

// --- 初期化処理 ---

void setup() {
  pinMode(LM_A, OUTPUT); pinMode(LM_B, OUTPUT);
  pinMode(RM_A, OUTPUT); pinMode(RM_B, OUTPUT);
  pinMode(LM2_A, OUTPUT); pinMode(LM2_B, OUTPUT);
  pinMode(RM2_A, OUTPUT); pinMode(RM2_B, OUTPUT);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(SPEAKER, OUTPUT);

  Serial.begin(9600);

  while (digitalRead(BUTTON_PIN) == 0);
  delay(500);
  sing(WAKEUP);
}

// --- メインループ ---

void loop() {
  int dist = analogRead(PSD_SENSOR);
  if (dist > DISTANCE_THRESHOLD) {
    stopBreak();
    // sing(ANGER);
    return;
  }

  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'F') {
      moveForward();
      Serial.println("f");
    } else if (cmd == 'L') {
      turnLeft();
      Serial.println("l");
    } else if (cmd == 'R') {
      turnRight();
      Serial.println("r");
    } else if (cmd == 'S') {
      stopBreak();
      // sing(ANGER);
    }
  }
}
