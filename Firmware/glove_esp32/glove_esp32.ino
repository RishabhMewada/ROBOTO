/*
 * ============================================================
 *  GLOVE CONTROLLER — ESP32 WROOM 28-PIN
 *  Hardware : BNO055 IMU  +  Flex Sensor (ADC)  +  ESP-NOW TX
 *  Target   : Robotic Arm ESP32 (peer MAC set below)
 * ============================================================
 *
 *  Wiring Summary
 *  ─────────────
 *  BNO055  SDA  →  GPIO 21
 *  BNO055  SCL  →  GPIO 22
 *  BNO055  VIN  →  3.3 V
 *  BNO055  GND  →  GND
 *
 *  Flex Sensor  →  GPIO 34 (ADC1_CH6)  via voltage divider
 *                  (10 kΩ from 3.3 V, sensor to GND)
 *
 *  Status LED   →  GPIO 2  (built-in LED, active HIGH)
 *
 *  Calibration button → GPIO 0 (BOOT button, active LOW)
 *    Hold 3 s to enter calibration mode.
 * ============================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Preferences.h>

// ── Pin Definitions ──────────────────────────────────────────
#define PIN_FLEX        34      // ADC – flex sensor
#define PIN_LED         2       // Status LED
#define PIN_CAL_BTN     0       // BOOT button for calibration

// ── Flex Sensor Thresholds (raw ADC 0-4095) ──────────────────
#define FLEX_OPEN_RAW   1500    // finger straight  → override in cal
#define FLEX_CLOSE_RAW  2800    // finger bent      → override in cal
#define FLEX_HYST       80      // hysteresis to avoid chatter

// ── BNO055 ───────────────────────────────────────────────────
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// ── Preferences (NVS) for persistent calibration ─────────────
Preferences prefs;

// ── ARM MAC ADDRESS ──────────────────────────────────────────
//  *** Replace with the actual MAC of your Arm ESP32 ***
uint8_t armMac[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

// ── Data Packet (must match arm side exactly) ─────────────────
typedef struct __attribute__((packed)) {
    float roll;          // degrees  -180 … +180
    float pitch;         // degrees  -90  … +90
    float yaw;           // degrees  0    … 360
    float quatW;         // quaternion W
    float quatX;
    float quatY;
    float quatZ;
    uint8_t gripClose;   // 1 = close gripper
    uint8_t calMode;     // 1 = calibration packet
    int16_t flexRaw;     // raw ADC for arm-side display
} GlovePacket;

// ── Calibration offsets stored in NVS ────────────────────────
struct CalOffset {
    float rollOffset;
    float pitchOffset;
    float yawOffset;
    int   flexOpen;
    int   flexClose;
} calOff = {0.0f, 0.0f, 0.0f, FLEX_OPEN_RAW, FLEX_CLOSE_RAW};

// ── Runtime State ─────────────────────────────────────────────
GlovePacket pkt;
bool  gripState      = false;   // current gripper state (with hysteresis)
bool  calPending     = false;
unsigned long calBtnPressTime = 0;
unsigned long lastSendMs      = 0;
const uint16_t SEND_INTERVAL_MS = 20;  // 50 Hz

// ── ESP-NOW send callback ─────────────────────────────────────
void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
    // Blink LED on successful send
    if (status == ESP_NOW_SEND_SUCCESS) {
        digitalWrite(PIN_LED, HIGH);
        delayMicroseconds(500);
        digitalWrite(PIN_LED, LOW);
    }
}

// ── Load calibration from NVS ─────────────────────────────────
void loadCalibration() {
    prefs.begin("glove_cal", true);
    calOff.rollOffset  = prefs.getFloat("rollOff",  0.0f);
    calOff.pitchOffset = prefs.getFloat("pitchOff", 0.0f);
    calOff.yawOffset   = prefs.getFloat("yawOff",   0.0f);
    calOff.flexOpen    = prefs.getInt("flexOpen",   FLEX_OPEN_RAW);
    calOff.flexClose   = prefs.getInt("flexClose",  FLEX_CLOSE_RAW);
    prefs.end();
    Serial.println("[CAL] Offsets loaded from NVS");
    Serial.printf("  Roll=%.2f  Pitch=%.2f  Yaw=%.2f\n",
                  calOff.rollOffset, calOff.pitchOffset, calOff.yawOffset);
    Serial.printf("  FlexOpen=%d  FlexClose=%d\n",
                  calOff.flexOpen, calOff.flexClose);
}

// ── Save calibration to NVS ───────────────────────────────────
void saveCalibration() {
    prefs.begin("glove_cal", false);
    prefs.putFloat("rollOff",   calOff.rollOffset);
    prefs.putFloat("pitchOff",  calOff.pitchOffset);
    prefs.putFloat("yawOff",    calOff.yawOffset);
    prefs.putInt("flexOpen",    calOff.flexOpen);
    prefs.putInt("flexClose",   calOff.flexClose);
    prefs.end();
    Serial.println("[CAL] Offsets saved to NVS");
}

// ── Run interactive calibration via Serial ────────────────────
void runCalibration() {
    Serial.println("\n========== CALIBRATION MODE ==========");
    Serial.println("Keep glove FLAT & STILL (neutral pose)…");

    // Wait for BNO055 to settle
    for (int i = 5; i > 0; i--) {
        Serial.printf("  Starting in %d s\r", i);
        delay(1000);
    }
    Serial.println();

    // Sample neutral pose over 1 second (50 samples)
    float sumR = 0, sumP = 0, sumY = 0;
    for (int s = 0; s < 50; s++) {
        sensors_event_t ev;
        bno.getEvent(&ev);
        sumR += ev.orientation.z;   // BNO055 roll
        sumP += ev.orientation.y;   // pitch
        sumY += ev.orientation.x;   // yaw/heading
        delay(20);
    }
    calOff.rollOffset  = sumR / 50.0f;
    calOff.pitchOffset = sumP / 50.0f;
    calOff.yawOffset   = sumY / 50.0f;
    Serial.printf("[CAL] Neutral → Roll=%.2f  Pitch=%.2f  Yaw=%.2f\n",
                  calOff.rollOffset, calOff.pitchOffset, calOff.yawOffset);

    // Flex sensor open
    Serial.println("\n[CAL] OPEN your hand fully and hold…");
    delay(3000);
    long sumFlex = 0;
    for (int s = 0; s < 50; s++) { sumFlex += analogRead(PIN_FLEX); delay(20); }
    calOff.flexOpen = (int)(sumFlex / 50);
    Serial.printf("[CAL] Flex OPEN raw = %d\n", calOff.flexOpen);

    // Flex sensor closed
    Serial.println("[CAL] CLOSE your hand fully and hold…");
    delay(3000);
    sumFlex = 0;
    for (int s = 0; s < 50; s++) { sumFlex += analogRead(PIN_FLEX); delay(20); }
    calOff.flexClose = (int)(sumFlex / 50);
    Serial.printf("[CAL] Flex CLOSE raw = %d\n", calOff.flexClose);

    saveCalibration();

    // Send a calibration packet to arm so it can reset its own offsets
    memset(&pkt, 0, sizeof(pkt));
    pkt.calMode = 1;
    esp_now_send(armMac, (uint8_t*)&pkt, sizeof(pkt));
    delay(100);

    Serial.println("[CAL] Calibration complete! Resuming normal operation.");
    Serial.println("======================================\n");

    // Long LED blink to signal done
    for (int i = 0; i < 6; i++) {
        digitalWrite(PIN_LED, HIGH); delay(150);
        digitalWrite(PIN_LED, LOW);  delay(150);
    }
}

// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n[GLOVE] Booting…");

    pinMode(PIN_LED,    OUTPUT);
    pinMode(PIN_CAL_BTN, INPUT_PULLUP);
    digitalWrite(PIN_LED, LOW);

    // I2C + BNO055
    Wire.begin(21, 22);
    if (!bno.begin()) {
        Serial.println("[ERR] BNO055 not found — check wiring!");
        while (true) {
            digitalWrite(PIN_LED, HIGH); delay(100);
            digitalWrite(PIN_LED, LOW);  delay(100);
        }
    }
    bno.setExtCrystalUse(true);
    Serial.println("[OK] BNO055 ready");

    // Load stored calibration
    loadCalibration();

    // Wi-Fi in station mode (required for ESP-NOW)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print our MAC so you can paste it on the arm side
    Serial.printf("[NET] Glove MAC: %s\n", WiFi.macAddress().c_str());

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("[ERR] ESP-NOW init failed");
        while (true) delay(1000);
    }
    esp_now_register_send_cb(onDataSent);

    // Register arm as peer
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, armMac, 6);
    peer.channel = 0;
    peer.encrypt = false;
    if (!esp_now_is_peer_exist(armMac)) {
        esp_now_add_peer(&peer);
    }

    Serial.println("[OK] ESP-NOW ready → Arm MAC set");
    Serial.println("[GLOVE] Running — hold BOOT 3 s to calibrate\n");
}

// ─────────────────────────────────────────────────────────────
void loop() {
    unsigned long now = millis();

    // ── Calibration button detection ──────────────────────────
    if (digitalRead(PIN_CAL_BTN) == LOW) {
        if (calBtnPressTime == 0) calBtnPressTime = now;
        if (now - calBtnPressTime > 3000 && !calPending) {
            calPending = true;
            runCalibration();
            calPending       = false;
            calBtnPressTime  = 0;
        }
    } else {
        calBtnPressTime = 0;
    }

    // ── 50 Hz data send ───────────────────────────────────────
    if (now - lastSendMs < SEND_INTERVAL_MS) return;
    lastSendMs = now;

    // Read BNO055 Euler angles
    sensors_event_t ev;
    bno.getEvent(&ev);

    float rawRoll  = ev.orientation.z;
    float rawPitch = ev.orientation.y;
    float rawYaw   = ev.orientation.x;

    // Apply neutral offsets
    pkt.roll  = rawRoll  - calOff.rollOffset;
    pkt.pitch = rawPitch - calOff.pitchOffset;
    pkt.yaw   = rawYaw   - calOff.yawOffset;

    // Quaternion (for smooth interpolation on arm side)
    imu::Quaternion q = bno.getQuat();
    pkt.quatW = (float)q.w();
    pkt.quatX = (float)q.x();
    pkt.quatY = (float)q.y();
    pkt.quatZ = (float)q.z();

    // Read flex sensor (12-bit ADC, averaged 4 samples)
    int flexSum = 0;
    for (int i = 0; i < 4; i++) flexSum += analogRead(PIN_FLEX);
    int flexVal = flexSum / 4;
    pkt.flexRaw = (int16_t)flexVal;

    // Map flex to gripper command with hysteresis
    int threshold = gripState
        ? (calOff.flexClose - FLEX_HYST)   // already closed: open at lower threshold
        : (calOff.flexOpen  + FLEX_HYST);  // already open : close at higher threshold

    if (!gripState && flexVal > threshold) gripState = true;   // close
    if ( gripState && flexVal < threshold) gripState = false;  // open
    pkt.gripClose = gripState ? 1 : 0;

    pkt.calMode = 0;

    // Send packet
    esp_now_send(armMac, (uint8_t*)&pkt, sizeof(pkt));

    // Serial debug (comment out for max throughput)
    Serial.printf("R:%.1f P:%.1f Y:%.1f | Flex:%d Grip:%d\n",
                  pkt.roll, pkt.pitch, pkt.yaw, flexVal, pkt.gripClose);
}
