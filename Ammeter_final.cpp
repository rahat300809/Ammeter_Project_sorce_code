#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

Adafruit_ADS1115 ads;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== SETTINGS =====
#define NUM_SAMPLES 50   // more samples = more stable

// ADS1115 GAIN_EIGHT → 0.015625 mV per bit
float LSB_mV = 0.015625;

// ===== FINAL BALANCED CALIBRATION =====
float CAL_FACTOR = 0.83;

// ===== OFFSET =====
float offset_voltage = 0;

// ===== VARIABLES =====
float current;
float smoothCurrent = 0;

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();

  ads.setGain(GAIN_EIGHT);
  ads.begin();

  lcd.setCursor(0, 0);
  lcd.print("Digital Ammeter");

  // ===== AUTO ZERO (no load required) =====
  delay(1000);
  long sum = 0;

  for (int i = 0; i < 50; i++) {
    sum += ads.readADC_Differential_0_1();
    delay(5);
  }

  float avg = sum / 50.0;
  offset_voltage = (avg * LSB_mV) / 1000.0;
}

// ===== READ VOLTAGE =====
float readVoltage() {
  long sum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += ads.readADC_Differential_0_1();
  }

  float avg = sum / (float)NUM_SAMPLES;
  float voltage = (avg * LSB_mV) / 1000.0;

  // remove offset
  voltage -= offset_voltage;

  return voltage;
}

void loop() {

  float voltage = readVoltage();

  // I = V / R (R = 0.1 ohm)
  current = (voltage / 0.1) * CAL_FACTOR;

  // ===== SMOOTHING =====
  smoothCurrent = (0.85 * smoothCurrent) + (0.15 * current);

  // ===== NOISE CUT =====
  if (fabs(smoothCurrent) < 0.0002) smoothCurrent = 0;

  lcd.setCursor(0, 1);

  // ===== AUTO UNIT DISPLAY =====
  if (fabs(smoothCurrent) < 1.0) {

    float mA = smoothCurrent * 1000.0;
    mA = round(mA * 10.0) / 10.0;

    lcd.print("I: ");
    lcd.print(mA, 1);
    lcd.print(" mA   ");

  } else {

    float amp = round(smoothCurrent * 1000.0) / 1000.0;

    lcd.print("I: ");
    lcd.print(amp, 3);
    lcd.print(" A    ");
  }

  delay(100);
}