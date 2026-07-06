/*
 * Laser power tester — NTC readout stage
 * Wiring: 3V3 --- NTC --- A0 --- 10k(1%) --- GND
 * Laser:  TTL module — 12V + GND from supply, PWM input driven from D6
 *         100R series on D6-> PWM in; 10K pull-down on PWM in to GND
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

const float R_FIXED = 10000.0;
const float R0      = 10000.0;
const float T0_K    = 298.15; 
const float BETA    = 3950.0; 
const int   ADC_MAX = 4095;      // 12-bit

//   find R_thermal by calibration    
const float R_THERMAL = 4.72;
const float T_AMBIENT = 26.75;

#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

#define LASER_PIN   6
#define SAFETY_PIN  7

const int N_AVG = 32;

// Global variables for tracking
float max_power_W = 0.0;
unsigned long fireStartTime = 0;

float readNTC_ohms() {
  long acc = 0;
  for (int i = 0; i < N_AVG; i++) { 
    acc += analogRead(A0); 
    delay(2); 
  }
  float adc = (float)acc / N_AVG;

  if (adc < 1)        return -1.0;      
  if (adc > ADC_MAX-1) return -2.0;

  return R_FIXED * ((ADC_MAX - adc) / adc);
}

float ohmsToC(float r) {
  float invT = 1.0 / T0_K + (1.0 / BETA) * log(r / R0);
  return (1.0 / invT) - 273.15;
}

void setup() {
  analogReadResolution(12);

  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, LOW); 

  pinMode(SAFETY_PIN, INPUT_PULLUP);

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  
  // Initialize trackers
  max_power_W = 0.0;
  fireStartTime = 0;
}

void loop() {
  bool fire = (digitalRead(SAFETY_PIN) == LOW);
  digitalWrite(LASER_PIN, fire ? HIGH : LOW);

  float r = readNTC_ohms();

  display.clearDisplay();
  display.setCursor(0, 0);

  if (r < 0) {
    display.setTextSize(1);
    display.println("NTC fault");
    display.println(r == -1.0 ? "open?" : "short?");
    display.display();
    delay(200);
    return;
  }

  float c = ohmsToC(r);
  
  // Calculate current power
  float power_W = (c - T_AMBIENT) / R_THERMAL;

  // Update Max Wattage ONLY if laser is currently firing and power is higher
  if (fire && power_W > max_power_W) {
    max_power_W = power_W;
  }

  // Calculate Timer
  unsigned long elapsedMs = 0;
  if (fire) {
    if (fireStartTime == 0) fireStartTime = millis(); // Start timer on first fire
    elapsedMs = millis() - fireStartTime;
  } else {
    // If laser is off, you can choose to:
    // 1. Keep timer running (until reset)
    // 2. Stop timer. 
    // Here we keep it running until reset, or you can set fireStartTime=0 to reset.
    // To reset timer when laser turns OFF, uncomment the next line:
    fireStartTime = 0; 
  }

  int totalSeconds = (int)(elapsedMs / 1000);
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;

  display.setTextSize(1);
  display.print("R: "); 
  display.print(r, 0); 
  display.println(" ohm");

  // Display Temp and Max Power on the same line
  display.setCursor(0, 20);
  display.print(c, 2); 
  display.print(" C  ");
  
  // Print Max Power with "W MAX" label
  display.print(max_power_W, 2);
  display.print(" W MAX");

  // Display Timer
  display.setCursor(0, 48);
  display.print("T: ");
  if (minutes < 10) display.print("0");
  display.print(minutes);
  display.print(":");
  if (seconds < 10) display.print("0");
  display.print(seconds);

  // Optional: Laser Status Indicator
  display.setCursor(100, 48);
  display.print(fire ? "ON" : "OFF");

  display.display();
  delay(200);
}