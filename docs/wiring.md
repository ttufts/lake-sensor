# Prototype Wiring and Bench Safety

## Pin allocation

| Function | GPIO | Notes |
|---|---:|---|
| Vext control | 36 | Active low: LOW enables Vext |
| External I²C SDA/SCL | 4 / 5 | ADS1115 bus |
| DS18B20 data | 6 | 4.7 kΩ pull-up to Vext |
| Battery divider enable | 37 | V3.2 only; active high, verify revision |
| Battery ADC | 1 | Built-in divider input |
| SX1262 NSS/SCK/MOSI/MISO | 8 / 9 / 10 / 11 | Do not reuse |
| SX1262 reset/busy/DIO1 | 12 / 13 / 14 | Do not reuse |

## Connections

```text
Vext 3.3 V ---+--- ADS1115 VDD
              +--- boost VIN+
              +--- DS18B20 VDD
              +--- 4.7 kΩ --- GPIO6 --- DS18B20 DQ
common GND ---+--- ADS1115 GND
              +--- boost VIN-
              +--- 100 Ω shunt bottom
              +--- DS18B20 GND
GPIO4 ------------ ADS1115 SDA
GPIO5 ------------ ADS1115 SCL

boost OUT+ 24 V -- sensor positive
sensor negative -- sense node -- 100 Ω shunt -- GND
                              +-- 1 kΩ -- ADS1115 A0
                                           |
                                         100 nF
                                           |
                                          GND
ADS1115 ADDR ------ GND (address 0x48)
```

All grounds are common. The 24 V output connects only to the sensor loop.

## Safe bring-up

1. Attach the correct 915 MHz antenna before allowing radio transmission.
2. Confirm board revision and battery connector polarity; initially use USB.
3. With the sensor disconnected, set the boost to 24.0 V using a multimeter.
4. Load-test it with approximately 1.2 kΩ rated at 1 W (20 mA at 24 V).
5. Verify Vext startup does not reset the Heltec and measure Vext sag.
6. Confirm sensor polarity from its label, then verify loop current by meter.
7. Add the ADS1115 and compare A0 voltage to the meter at several currents.
8. Add the DS18B20, verify its ROM/readings, and test the disconnected fault.
9. Only then enable unattended sleep/wake cycling on a protected 1S cell.

Place the temperature probe beside the pressure probe in the stilling well, but
do not block the pressure port. Verify the supplied probe's wire colors rather
than relying on common conventions. The stainless capsule may be submerged;
protect the cable splice and dry-end termination according to the seller's
actual immersion rating.

Never solder to a bare 18650, connect cells in series, charge below the cell's
specified temperature, or seal a vented transmitter's atmospheric tube.
