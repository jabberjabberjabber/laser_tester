/*
 * Laser power meter — NTC readout stage
 * Board:  Seeeduino XIAO (SAMD21)
 * Sensor: 10k NTC (beta ~3950) in a metal lug, bonded to Vantablack copper slug
 * Wiring: 3V3 --- NTC --- A0 --- 10k(1%) --- GND
 *         Divider runs off 3V3 so it's ratiometric with the ADC reference
 * Display: SSD1306 I2C OLED
 *
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// ---- divider / NTC constants ----
const float R_FIXED = 10000.0;   // 1% metal-film resistor, 10K ohms
const float R0      = 10000.0;   // NTC resistance at T0
const float T0_K    = 298.15;    // 25 C in kelvin
const float BETA    = 3950.0;    // NTC beta — CALIBRATE THIS (see notes)
const int   ADC_MAX = 4095;      // 12-bit

// ---- OLED ----
#define SCREEN_W 128
#define SCREEN_H 64              // use 32 if 128x32
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

// ---- averaging ----
const int N_AVG = 32;

float readNTC_ohms() {
  long acc = 0;
  for (int i = 0; i < N_AVG; i++) { acc += analogRead(A0); delay(2); }
  float adc = (float)acc / N_AVG;

  // guard rails: open/short sensor
  if (adc < 1)        return -1.0;        // junction at GND -> NTC open
  if (adc > ADC_MAX-1) return -2.0;       // junction at rail -> NTC short

  // 3V3 -- NTC -- A0 -- Rfixed -- GND
  // adc/ADC_MAX = Rfixed / (Rntc + Rfixed)   (Vref cancels, ratiometric)
  return R_FIXED * ((ADC_MAX - adc) / adc);
}

float ohmsToC(float r) {
  // beta equation: 1/T = 1/T0 + (1/beta)*ln(R/R0)
  float invT = 1.0 / T0_K + (1.0 / BETA) * log(r / R0);
  return (1.0 / invT) - 273.15;
}

void setup() {
  analogReadResolution(12);      // SAMD21 supports 12-bit
  // AR_DEFAULT (VDDANA) is already ratiometric with the 3V3 divider — leave it.

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void loop() {
  float r = readNTC_ohms();

  display.clearDisplay();
  display.setCursor(0, 0);

  if (r < 0) {
    display.setTextSize(1);
    display.println("NTC fault");
    display.println(r == -1.0 ? "open?" : "short?");
  } else {
    float c = ohmsToC(r);

    display.setTextSize(1);
    display.print("R: "); display.print(r, 0); display.println(" ohm");

    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(c, 2); display.println(" C");

    // ---- TODO: power stage ----
    // Transient (single laser pulse into an insulated slug):
    //   P = m * c_p * (dT/dt)
    //   copper c_p = 385 J/kg/K; m = slug mass in kg; dT/dt from timestamps.
    // Steady-state (continuous beam, slug losing heat to ambient):
    //   P = (T - T_ambient) / R_thermal
    //   find R_thermal by a known-power calibration first.
    // Uncomment once you've measured your slug.
    //
    // display.setTextSize(1);
    // display.setCursor(0, 48);
    // display.print(power_W, 2); display.println(" W");
  }

  display.display();
  delay(200);
}
