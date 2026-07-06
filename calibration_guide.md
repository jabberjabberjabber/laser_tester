# Calibrating the Laser Power Meter with a Power Resistor

Resistors will convert electricity entirely to heat, so putting a known voltage and current through a known resistance and reading the temperature will tell us how to read the sensor's response. 

## Parts and Tools

- 5W+ power resistor, 5ohms to 25ohms
- Bench supply or LM317 set to variable voltage 
- Multimeter
- Thermal tape
- Fully assembled laser meter

## Mount the resistor

Wipe the non-black side of the slug with isopropyl and when dry apply thermal transfer tape to the back and stick the resistor on to it. Place it and the meter inside a cooler or insulated, fire-resistant box.
 
## Measure R_thermal

1. Power up the laser meter and let it sit until the reading is steady. Record this as **T_ambient**
2. Send a steady voltage through the resistor at the power level of the laser to measure. For instance measuring a 3W laser with a 5ohm resistor, the lasers you'll measure. Voltage is equal to the square root of power (watts) times resistance (ohms), V = sqrt(3W x 5R) which is 3.87V. Power is Voltage^2 divided by resistance, so if you set the voltage at 4V you get 16 / 5 = 3.2W theoretical
3. Wait for the temperature to hit the plateau
4. Put the multimeter in DC voltage mode and test put a probe on each of the resistor legs and measure the voltage. Remove the leads and set the multimeter into current mode and disconnect one leg of the resistor, put the red multimeter lead into the 'current' socket and then prove the voltage supple with the red lead and the resistor leg with the black lead and read the current. Multiply current by voltage to get the actual power through the resistor
5. Record the temperature as **T_steady**
6. Compute:

   **R_thermal = (T_steady − T_ambient) / P**   (units: °C per watt)

Repeat at two or three power levels (say 1 W, 2 W, 4 W). If R_thermal comes out roughly constant, you're linear and one number suffices. If it drops at higher power, convection is nonlinear, you need to fit a curve or interpolate between points.

## Update the sketch

Uncomment the power block and add the constant:

```cpp
const float R_THERMAL = 8.7;   // measured value, C per W
float power_W = (c - t_ambient) / R_THERMAL;
```


