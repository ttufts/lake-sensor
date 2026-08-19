# RAK Mock Sensors

The `rak_mock_node` firmware makes the radio and MQTT path testable before the
lake sensors arrive. A potentiometer simulates a 0–5 m pressure measurement and
a DHT22 supplies live ambient temperature. The firmware derives consistent
4–20 mA loop and 100-ohm shunt values from the knob position and uses a fixed
3.900 V mock battery value.

## Wiring

Disconnect USB power while making connections. Use only the WisBlock 3.3 V
rail; RAK4631 GPIO and analog inputs are not 5 V tolerant.

```text
RAK WisBlock IO header                 DHT22 (front grille facing you)

VDD (J12, 3.3 V) -------------------- pin 1 VCC
WB_IO1 ------------------------------- pin 2 DATA
          +--- 4.7–10 kΩ resistor ---- VDD
GND  -------------------------------- pin 4 GND
                                       pin 3 unused

RAK WisBlock IO header                 potentiometer

VDD (J12, 3.3 V) -------------------- one outside leg
AIN1 (J11 / firmware `WB_A1`) -------- center leg / wiper
GND  -------------------------------- other outside leg
```

Many three-pin DHT22 breakout modules already include the pull-up resistor. For
those, connect module `+` to VDD, `OUT`/`DATA` to `WB_IO1`, and `-` to GND; do
not add another resistor. A bare four-pin DHT22 normally needs the shown
pull-up. Swapping the potentiometer's outside legs only reverses knob direction.
A value from 1 kΩ through 100 kΩ is suitable; 10 kΩ is a conventional choice.

On the RAK19007, J12 `VDD` is 3.3 V and J11 exposes `AIN1`; the firmware name
for that analog input is `WB_A1`. Older WisBlock bases can expose `AIN0`
instead, so follow the printed signal labels rather than raw nRF52840 pin
numbers.

## Flash and observe

Keep both 900/915 MHz antennas attached, then identify current serial ports and
flash the mock node:

```sh
pio device list
pio run -e rak_mock_node -t upload --upload-port /dev/cu.usbmodem11421201
pio device monitor --port /dev/cu.usbmodem11421201 --baud 115200
```

Turning the potentiometer should move `depth` across approximately 0.000–5.000
m. A DHT read failure sets protocol flag `0x0100` instead of inventing a valid
temperature.
