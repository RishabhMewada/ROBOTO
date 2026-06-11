/*
 * ============================================================
 *  ROBOTIC ARM CONTROLLER — ESP32 WROOM 28-PIN
 *  Hardware : PCA9685 PWM Driver  +  WP5320 Servos (6 total)
 *             5 DOF arm  +  1 Gripper
 *  Input    : ESP-NOW packets from Glove ESP32
 * ============================================================
 *
 *  Wiring Summary
 *  ─────────────
 *  PCA9685  SDA  →  GPIO 21
 *  PCA9685  SCL  →  GPIO 22
 *  PCA9685  VIN  →  3.3 V (logic)
 *  PCA9685  V+   →  5–6 V  (servo power rail)
 *  PCA9685  GND  →  GND (common with ESP32 GND)
 *
 *  Servo Channel Map (PCA9685 channels 0-5)
 *  ─────────────────────────────────────────
 *  CH 0  →  Base    (yaw  / rotation)
 *  CH 1  →  Shoulder (pitch – lower)
 *  CH 2  →  Elbow   (pitch – upper)
 *  CH 3  →  Wrist Pitch
 *  CH 4  →  Wrist Roll
 *  CH 5  →  Gripper
 *
 *  Status LED  →  GPIO 2 (built-in, active HIGH)
 *  Cal button  →  GPIO 0 (BOOT, active LOW)
 *    Hold 3 s to enter arm-side calibration trim.
 * ============================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_PWMServoDriver.h>
#include <Preferences.h>

// ── PCA9685 ───────────────────────────────────────────────────
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40, Wire);

// ── WP5320 Servo PWM Range ────────────────────────────────────
//  WP5320 is a standard 270° digital servo
//  Pulse width:  500 µs → 2500 µs  (adjust if your unit differs)
//  At 50 Hz the PCA9685 counts run 0-4095 (20 ms period)
//    count = (pulse_us / 20000) * 4096
#define SERVO_FREQ       50
#define PULSE_MIN_US     500
#define PULSE_MAX_US     2500
#define SERVO_COUNT      6

// ── Servo Channels ────────────────────────────────────────────
#define CH_BASE          0
#define CH_SHOULDER      1
#define CH_ELBOW         2
#define CH_WRIST_PITCH   3
#define CH_WRIST_ROLL    4
#define CH_GRIPPER       5

// ── Servo mechanical limits (degrees) ────────────────────────
//    Adjust per your physical setup to avoid binding!
struct ServoLimits {
    float minDeg;
    float maxDeg;
    float centerDeg;    // neutral / home position
} limits[SERVO_COUNT] = {
    {  0.0f, 270.0f, 135.0f },  // CH0 Base
    { 30.0f, 200.0f, 115.0f },  // CH1 Shoulder
    { 20.0f, 220.0f, 120.0f },  // CH2 Elbow
    { 30.0f, 240.0f, 135.0f },  // CH3 Wrist Pitch
    {  0.0f, 270.0f, 135.0f },  // CH4 Wrist Roll
    { 30.0f, 150.0f,  30.0f },  // CH5 Gripper  (30=open 150=close)
};

// ── Glove Data Packet (must match glove side exactly) ─────────
typedef struct __attribute__((packed)) {
    float roll;
    float pitch;
    float yaw;
    float quatW;
    float quatX;
    float quatY;
    float quatZ;
    uint8_t gripClose;
    uint8_t calMode;
    int16_t flexRaw;
} GlovePacket;

// ── Runtime servo targets & current positions ─────────────────
float targetDeg[SERVO_COUNT];
float currentDeg[SERVO_COUNT];

// ── Smoothing factor (0.0 = no movement, 1.0 = instant) ───────
//    Lower = smoother & slower.  0.12–0.18 feels natural.
const float ALPHA = 0.15f;

// ── Calibration trim offsets (arm side, stored in NVS) ────────
struct ArmCalib {
    float trimDeg[SERVO_COUNT];     // added to computed angle
    float rollScale;                // glove roll  → servo deg multiplier
    float pitchScale;
    float yawScale;
} armCal = {
    {0,0,0,0,0,0},
    1.0f, 1.0f, 1.0f
};

// ── Gripper FSM ───────────────────────────────────────────────
bool  lastGripClose   = false;
float gripTargetDeg   = 30.0f;     // open

// ── Comms State ───────────────────────────────────────────────
volatile bool   newPacket       = false;
GlovePacket     rxPkt;
unsigned long   lastRxMs        = 0;
const uint32_t  TIMEOUT_MS      = 500;  // go to safe-hold if no data

// ── Calibration ───────────────────────────────────────────────
bool  calPending        = false;
unsigned long calBtnPressMs = 0;
Preferences prefs;

// ── Status LED ────────────────────────────────────────────────
#define PIN_LED  2
#define PIN_CAL  0

// ─── Helpers ──────────────────────────────────────────────────

// Convert degrees to PCA9685 tick count
uint16_t degToTick(float deg, int ch) {
    // clamp
    deg = constrain(deg, limits[ch].minDeg, limits[ch].maxDeg);
    // map deg (0-270) to pulse width
    float pulseUs = map(deg * 100, 0, 27000,
                        PULSE_MIN_US * 100, PULSE_MAX_US * 100) / 100.0f;
    return (uint16_t)((pulseUs / 20000.0f) * 4096.0f);
}

void writeServo(int ch, float deg) {
    uint16_t tick = degToTick(deg + armCal.trimDeg[ch], ch);
    pca.setPWM(ch, 0, tick);
}

// Move all servos to home position
void goHome() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        targetDeg[i]  = limits[i].centerDeg;
        currentDeg[i] = limits[i].centerDeg;
        writeServo(i, currentDeg[i]);
    }
    Serial.println("[ARM] Moved to HOME position");
}

// Load calibration from NVS
void loadArmCal() {
    prefs.begin("arm_cal", true);
    for (int i = 0; i < SERVO_COUNT; i++) {
        char key[16];
        snprintf(key, sizeof(key), "trim%d", i);
        armCal.trimDeg[i] = prefs.getFloat(key, 0.0f);
    }
    armCal.rollScale  = prefs.getFloat("rScale", 1.0f);
    armCal.pitchScale = prefs.getFloat("pScale", 1.0f);
    armCal.yawScale   = prefs.getFloat("yScale", 1.0f);
    prefs.end();
    Serial.println("[CAL] Arm calibration loaded");
}

void saveArmCal() {
    prefs.begin("arm_cal", false);
    for (int i = 0; i < SERVO_COUNT; i++) {
        char key[16];
        snprintf(key, sizeof(key), "trim%d", i);
        prefs.putFloat(key, armCal.trimDeg[i]);
    }
    prefs.putFloat("rScale", armCal.rollScale);
    prefs.putFloat("pScale", armCal.pitchScale);
    prefs.putFloat("yScale", armCal.yawScale);
    prefs.end();
    Serial.println("[CAL] Arm calibration saved");
}

// ── Interactive arm calibration via Serial ────────────────────
void runArmCalibration() {
    Serial.println("\n======== ARM CALIBRATION MODE ========");
    Serial.println("Commands:");
    Serial.println("  t<ch> <deg>  — set trim for channel (e.g. t0 -5.0)");
    Serial.println("  s<r|p|y> <f> — set scale (e.g. sr 0.8)");
    Serial.println("  h            — send arm to HOME");
    Serial.println("  q            — save & quit");
    Serial.println("======================================");

    // Sweep each servo to show user the current range
    Serial.println("Sweeping all servos to HOME…");
    goHome();
    delay(1000);

    while (true) {
        if (Serial.available()) {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();
            if (cmd.length() == 0) continue;

            char type = cmd.charAt(0);

            if (type == 'q') {
                saveArmCal();
                Serial.println("[CAL] Saved. Exiting calibration.");
                break;
            }
            else if (type == 'h') {
                goHome();
            }
            else if (type == 't') {
                int ch  = cmd.charAt(1) - '0';
                float v = cmd.substring(3).toFloat();
                if (ch >= 0 && ch < SERVO_COUNT) {
                    armCal.trimDeg[ch] = v;
                    writeServo(ch, limits[ch].centerDeg);
                    Serial.printf("  CH%d trim = %.2f°\n", ch, v);
                }
            }
            else if (type == 's') {
                char axis = cmd.charAt(1);
                float v   = cmd.substring(3).toFloat();
                if      (axis == 'r') { armCal.rollScale  = v; Serial.printf("  Roll scale  = %.3f\n", v); }
                else if (axis == 'p') { armCal.pitchScale = v; Serial.printf("  Pitch scale = %.3f\n", v); }
                else if (axis == 'y') { armCal.yawScale   = v; Serial.printf("  Yaw scale   = %.3f\n", v); }
            }
        }
        delay(10);
    }

    // Confirmation blinks
    for (int i = 0; i < 6; i++) {
        digitalWrite(PIN_LED, HIGH); delay(120);
        digitalWrite(PIN_LED, LOW);  delay(120);
    }
}

// ── ESP-NOW receive callback ───────────────────────────────────
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len == sizeof(GlovePacket)) {
        memcpy((void*)&rxPkt, data, sizeof(GlovePacket));
        newPacket = true;
        lastRxMs  = millis();
    }
}

// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n[ARM] Booting robotic arm controller…");

    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_CAL, INPUT_PULLUP);
    digitalWrite(PIN_LED, LOW);

    // I2C + PCA9685
    Wire.begin(21, 22);
    pca.begin();
    pca.setOscillatorFrequency(27000000);  // trim for accuracy
    pca.setPWMFreq(SERVO_FREQ);
    Serial.println("[OK] PCA9685 ready at 50 Hz");

    // Load arm calibration
    loadArmCal();

    // Init current positions at home
    for (int i = 0; i < SERVO_COUNT; i++) {
        currentDeg[i] = limits[i].centerDeg;
        targetDeg[i]  = limits[i].centerDeg;
    }
    goHome();
    delay(500);

    // Wi-Fi STA for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.printf("[NET] Arm MAC: %s\n", WiFi.macAddress().c_str());
    Serial.println("  ↑ Copy this MAC into glove_esp32.ino → armMac[]");

    // ESP-NOW init
    if (esp_now_init() != ESP_OK) {
        Serial.println("[ERR] ESP-NOW init failed");
        while (true) delay(1000);
    }
    esp_now_register_recv_cb(onDataRecv);
    Serial.println("[OK] ESP-NOW listening for glove packets");
    Serial.println("[ARM] Ready — hold BOOT 3 s for calibration\n");
}

// ─────────────────────────────────────────────────────────────
void loop() {
    unsigned long now = millis();

    // ── Calibration button ────────────────────────────────────
    if (digitalRead(PIN_CAL) == LOW) {
        if (calBtnPressMs == 0) calBtnPressMs = now;
        if (now - calBtnPressMs > 3000 && !calPending) {
            calPending = true;
            runArmCalibration();
            calPending    = false;
            calBtnPressMs = 0;
        }
    } else {
        calBtnPressMs = 0;
    }

    // ── Connection timeout → safe hold ────────────────────────
    if (now - lastRxMs > TIMEOUT_MS && lastRxMs != 0) {
        // Hold current position — just keep smoothing toward home slowly
        for (int i = 0; i < SERVO_COUNT; i++)
            targetDeg[i] = limits[i].centerDeg;
        digitalWrite(PIN_LED, (now / 300) % 2);  // blink = no signal
    } else {
        digitalWrite(PIN_LED, HIGH);
    }

    // ── Process new packet ────────────────────────────────────
    if (newPacket) {
        newPacket = false;

        // If glove triggered calibration reset
        if (rxPkt.calMode) {
            Serial.println("[ARM] Cal-reset signal received from glove → going HOME");
            goHome();
        } else {
            /*
             * MAPPING STRATEGY
             * ─────────────────
             *  Glove orientation ranges are roughly:
             *    Roll   -90 … +90  (wrist lean)
             *    Pitch  -60 … +60  (tilt fwd/back)
             *    Yaw      0 … 360  (rotation)
             *
             *  We remap these to servo angles.
             *  Tweak the map() calls to taste or adjust via scale in cal.
             */

            // CH0 Base — driven by Yaw
            float baseTarget = map(rxPkt.yaw * 10,
                                   -1800, 1800,
                                   limits[CH_BASE].minDeg * 10,
                                   limits[CH_BASE].maxDeg * 10) / 10.0f;
            targetDeg[CH_BASE] = constrain(baseTarget * armCal.yawScale,
                                           limits[CH_BASE].minDeg,
                                           limits[CH_BASE].maxDeg);

            // CH1 Shoulder — driven by Pitch (tilt forward = shoulder down)
            float shoulderTarget = map(rxPkt.pitch * 10,
                                       -600, 600,
                                       limits[CH_SHOULDER].maxDeg * 10,
                                       limits[CH_SHOULDER].minDeg * 10) / 10.0f;
            targetDeg[CH_SHOULDER] = constrain(shoulderTarget * armCal.pitchScale,
                                               limits[CH_SHOULDER].minDeg,
                                               limits[CH_SHOULDER].maxDeg);

            // CH2 Elbow — driven by Pitch (upper half range)
            float elbowTarget = map(rxPkt.pitch * 10,
                                    -600, 600,
                                    limits[CH_ELBOW].minDeg * 10,
                                    limits[CH_ELBOW].maxDeg * 10) / 10.0f;
            targetDeg[CH_ELBOW] = constrain(elbowTarget * armCal.pitchScale,
                                            limits[CH_ELBOW].minDeg,
                                            limits[CH_ELBOW].maxDeg);

            // CH3 Wrist Pitch — driven by Roll
            float wristPTarget = map(rxPkt.roll * 10,
                                     -900, 900,
                                     limits[CH_WRIST_PITCH].minDeg * 10,
                                     limits[CH_WRIST_PITCH].maxDeg * 10) / 10.0f;
            targetDeg[CH_WRIST_PITCH] = constrain(wristPTarget * armCal.rollScale,
                                                   limits[CH_WRIST_PITCH].minDeg,
                                                   limits[CH_WRIST_PITCH].maxDeg);

            // CH4 Wrist Roll — also driven by Roll (inverted feel)
            float wristRTarget = map(rxPkt.roll * 10,
                                     -900, 900,
                                     limits[CH_WRIST_ROLL].maxDeg * 10,
                                     limits[CH_WRIST_ROLL].minDeg * 10) / 10.0f;
            targetDeg[CH_WRIST_ROLL] = constrain(wristRTarget * armCal.rollScale,
                                                  limits[CH_WRIST_ROLL].minDeg,
                                                  limits[CH_WRIST_ROLL].maxDeg);

            // CH5 Gripper — binary from flex sensor
            if (rxPkt.gripClose && !lastGripClose) {
                gripTargetDeg = limits[CH_GRIPPER].maxDeg;  // CLOSE
                Serial.println("[GRIP] Close");
            } else if (!rxPkt.gripClose && lastGripClose) {
                gripTargetDeg = limits[CH_GRIPPER].minDeg;  // OPEN
                Serial.println("[GRIP] Open");
            }
            lastGripClose = rxPkt.gripClose;
            targetDeg[CH_GRIPPER] = gripTargetDeg;
        }
    }

    // ── Smooth interpolation (exponential filter) ─────────────
    for (int i = 0; i < SERVO_COUNT; i++) {
        currentDeg[i] = currentDeg[i] + ALPHA * (targetDeg[i] - currentDeg[i]);
        writeServo(i, currentDeg[i]);
    }

    // ── Debug output at ~5 Hz ──────────────────────────────────
    static unsigned long lastDbg = 0;
    if (now - lastDbg > 200) {
        lastDbg = now;
        Serial.printf("B:%.0f S:%.0f E:%.0f WP:%.0f WR:%.0f G:%.0f | RX R:%.1f P:%.1f Y:%.1f\n",
                       currentDeg[0], currentDeg[1], currentDeg[2],
                       currentDeg[3], currentDeg[4], currentDeg[5],
                       rxPkt.roll, rxPkt.pitch, rxPkt.yaw);
    }

    delay(10);  // ~100 Hz loop for smooth ALPHA filtering
}
