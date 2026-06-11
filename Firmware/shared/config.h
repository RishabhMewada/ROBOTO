/*
 * ============================================================
 *  config.h  —  Shared configuration for ROBOTO
 *  Include this file in BOTH:
 *    → firmware/glove_esp32/glove_esp32.ino
 *    → firmware/arm_esp32/arm_esp32.ino
 * ============================================================
 *
 *  Verified against wiring diagrams:
 *    RB-1.png       — Glove Arm Diagram
 *    RB-1__3_.png   — Robotic Arm Diagram
 * ============================================================
 */

#pragma once

// ============================================================
//  SECTION 1 — GLOVE ESP32 PIN DEFINITIONS
//  (from Glove Arm Diagram — RB-1.png)
// ============================================================

// ── I²C Bus (BNO055) ─────────────────────────────────────────
#define GLOVE_PIN_SDA           21      // GPIO 21 → BNO055 SDA  (green wire)
#define GLOVE_PIN_SCL           22      // GPIO 22 → BNO055 SCL  (purple wire)

// ── Flex Sensor ───────────────────────────────────────────────
#define GLOVE_PIN_FLEX          34      // GPIO 34 → Flex sensor via 10kΩ divider
                                        // (blue wire — ADC1_CH6, input only)

// ── Power & Control ───────────────────────────────────────────
#define GLOVE_PIN_LED           2       // GPIO 2  → Built-in status LED
#define GLOVE_PIN_CAL_BTN       0       // GPIO 0  → BOOT button (calibration trigger)
                                        //           Hold 3 s to enter calibration

// ── Power Rail (reference only — not digitalWrite'd) ─────────
// 3V3  → BNO055 VIN                    (red wire)
// GND  → BNO055 GND, Flex GND, common  (black wire)
// VIN  → MT3608 VOUT                   (blue OUT+ wire from boost converter)
// MT3608 VIN+ → TP4056 OUT+            (yellow/orange wires)
// TP4056 B+/B- → 18650 battery         (red/black)
// Rocker switch → MT3608 VIN+ rail     (power on/off)


// ============================================================
//  SECTION 2 — ARM ESP32 PIN DEFINITIONS
//  (from Robotic Arm Diagram — RB-1__3_.png)
// ============================================================

// ── I²C Bus (PCA9685) ────────────────────────────────────────
#define ARM_PIN_SDA             21      // GPIO 21 → PCA9685 SDA  (blue wire)
#define ARM_PIN_SCL             22      // GPIO 22 → PCA9685 SCL  (green wire)

// ── Power & Control ───────────────────────────────────────────
#define ARM_PIN_LED             2       // GPIO 2  → Built-in status LED
#define ARM_PIN_CAL_BTN         0       // GPIO 0  → BOOT button (arm trim mode)

// ── Power Rail (reference only) ───────────────────────────────
// 3V3  → PCA9685 VCC  (logic power)    (red wire)
// GND  → PCA9685 GND + Servo GND       (black wire — common ground)
// External supply (+) → PCA9685 V+     (orange wire — servo power rail)
// External supply (-) → GND            (black wire — MUST share common GND)


// ============================================================
//  SECTION 3 — PCA9685 SERVO CHANNEL MAP
//  (from Robotic Arm Diagram — channels 0-5, top to bottom)
// ============================================================

#define SERVO_CH_BASE           0       // CH 0 — Base rotation    (driven by Yaw)
#define SERVO_CH_SHOULDER       1       // CH 1 — Shoulder         (driven by Pitch)
#define SERVO_CH_ELBOW          2       // CH 2 — Elbow            (driven by Pitch)
#define SERVO_CH_WRIST_PITCH    3       // CH 3 — Wrist Pitch      (driven by Roll)
#define SERVO_CH_WRIST_ROLL     4       // CH 4 — Wrist Roll       (driven by Roll)
#define SERVO_CH_GRIPPER        5       // CH 5 — Gripper          (driven by Flex)
#define SERVO_COUNT             6       // Total servo count

// ── WP5320 PWM Parameters ─────────────────────────────────────
#define SERVO_PWM_FREQ          50      // Hz  — standard servo frequency
#define SERVO_PULSE_MIN_US      500     // µs  — 0°   position pulse width
#define SERVO_PULSE_MAX_US      2500    // µs  — 270° position pulse width

// ── PCA9685 I²C Address ───────────────────────────────────────
#define PCA9685_I2C_ADDR        0x40    // Default address (all ADDR pins to GND)
#define PCA9685_OSC_FREQ        27000000UL  // 27 MHz — trimmed oscillator value

// ── BNO055 I²C Address ────────────────────────────────────────
#define BNO055_I2C_ADDR         0x28    // Default (ADR pin to GND)


// ============================================================
//  SECTION 4 — SERVO MECHANICAL LIMITS (degrees)
//  Adjust minDeg / maxDeg to match your physical build.
//  centerDeg = safe home / neutral position.
// ============================================================

struct ServoConfig {
    const char* name;
    float       minDeg;
    float       maxDeg;
    float       centerDeg;
};

// Index matches SERVO_CH_* defines above
const ServoConfig SERVO_CFG[SERVO_COUNT] = {
    { "Base",        0.0f,  270.0f,  135.0f },  // CH 0
    { "Shoulder",   30.0f,  200.0f,  115.0f },  // CH 1
    { "Elbow",      20.0f,  220.0f,  120.0f },  // CH 2
    { "WristPitch", 30.0f,  240.0f,  135.0f },  // CH 3
    { "WristRoll",   0.0f,  270.0f,  135.0f },  // CH 4
    { "Gripper",    30.0f,  150.0f,   30.0f },  // CH 5  (30=open, 150=closed)
};


// ============================================================
//  SECTION 5 — ESP-NOW COMMUNICATION
// ============================================================

// !! IMPORTANT — Replace with your actual Arm ESP32 MAC address !!
// Step 1: Flash arm_esp32.ino, open Serial Monitor @ 115200
// Step 2: Copy the MAC printed as "[NET] Arm MAC: XX:XX:XX:XX:XX:XX"
// Step 3: Paste the 6 bytes below
#define ARM_MAC     { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }   // ← REPLACE THIS

#define ESPNOW_CHANNEL          0       // Wi-Fi channel (0 = auto-match)
#define ESPNOW_ENCRYPT          false   // Set true to enable AES-128 encryption

// ── Packet Timing ─────────────────────────────────────────────
#define SEND_INTERVAL_MS        20      // Glove TX interval  = 50 Hz
#define ARM_TIMEOUT_MS          500     // Arm safe-hold timeout (no packet)


// ============================================================
//  SECTION 6 — GLOVE SENSOR PARAMETERS
// ============================================================

// ── Flex Sensor ADC (12-bit: 0–4095) ─────────────────────────
#define FLEX_OPEN_DEFAULT       1500    // Raw ADC — finger fully open
#define FLEX_CLOSE_DEFAULT      2800    // Raw ADC — finger fully closed
#define FLEX_HYSTERESIS         80      // ADC counts — prevents chatter

// ── ADC Averaging ─────────────────────────────────────────────
#define FLEX_SAMPLES            4       // Samples averaged per reading


// ============================================================
//  SECTION 7 — MOTION SMOOTHING & MAPPING
// ============================================================

// ── Exponential filter on arm side ───────────────────────────
// Range: 0.05 (very smooth/slow) → 0.50 (fast/snappy)
#define SMOOTH_ALPHA            0.15f

// ── Arm control loop rate ─────────────────────────────────────
#define ARM_LOOP_DELAY_MS       10      // 10 ms = 100 Hz filter loop

// ── Glove orientation → servo degree input ranges ─────────────
#define GLOVE_YAW_MIN          -180.0f  // degrees — maps to SERVO_CH_BASE   min
#define GLOVE_YAW_MAX           180.0f  // degrees — maps to SERVO_CH_BASE   max
#define GLOVE_PITCH_MIN        -60.0f   // degrees — maps to SHOULDER / ELBOW
#define GLOVE_PITCH_MAX         60.0f
#define GLOVE_ROLL_MIN         -90.0f   // degrees — maps to WRIST PITCH / ROLL
#define GLOVE_ROLL_MAX          90.0f


// ============================================================
//  SECTION 8 — CALIBRATION
// ============================================================

#define CAL_BTN_HOLD_MS         3000    // Hold duration to trigger calibration
#define CAL_SAMPLE_COUNT        50      // Samples averaged during neutral capture
#define CAL_SAMPLE_INTERVAL_MS  20      // ms between calibration samples

// ── NVS (non-volatile storage) namespace keys ─────────────────
#define NVS_GLOVE_NS            "glove_cal"
#define NVS_ARM_NS              "arm_cal"


// ============================================================
//  SECTION 9 — SHARED PACKET STRUCT
//  !! Must be identical on both Glove and Arm firmware !!
// ============================================================

typedef struct __attribute__((packed)) {
    float    roll;          // degrees, neutral-offset corrected  (-180 … +180)
    float    pitch;         // degrees, neutral-offset corrected  (-90  … +90)
    float    yaw;           // degrees, neutral-offset corrected  (0    … 360)
    float    quatW;         // quaternion W  (for smooth interpolation)
    float    quatX;         // quaternion X
    float    quatY;         // quaternion Y
    float    quatZ;         // quaternion Z
    uint8_t  gripClose;     // 1 = close gripper,  0 = open
    uint8_t  calMode;       // 1 = calibration reset signal, 0 = normal operation
    int16_t  flexRaw;       // raw ADC value (0–4095) for Serial diagnostics
} GlovePacket;              // Total: 30 bytes
// sizeof(GlovePacket) must equal 30 on both boards — verified with __attribute__((packed))


// ============================================================
//  SECTION 10 — DEBUG
// ============================================================

#define DEBUG_SERIAL_BAUD       115200
#define DEBUG_ARM_INTERVAL_MS   200     // Arm prints servo angles every 200 ms
// Comment out the line below to disable all Serial debug output
#define DEBUG_ENABLED
