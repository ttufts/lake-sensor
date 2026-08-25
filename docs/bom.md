# Bill of Materials — Working Field Prototype

| Qty. | Installed part | Purpose / important requirement |
|---:|---|---|
| 1 | RAKwireless RAK4631, US915 | nRF52840/SX1262 lake-node core |
| 1 | RAKwireless RAK19003 | Compact WisBlock base and battery connection |
| 1 | Heltec WiFi LoRa 32 V3, US915 | House-side LoRa/Wi-Fi/MQTT gateway |
| 2 | 915 MHz antennas | Attach before either radio transmits |
| 1 | ADS1115 breakout | I²C ADC, address `0x48`, A0 pressure input |
| 1 | MT3608 adjustable boost module | 3.3 V input, adjusted to 24.0 V output; [Amazon B089JYBF25](https://www.amazon.com/dp/B089JYBF25) |
| 1 | Tangxi/TL231-style 0–5 m, two-wire 4–20 mA submersible pressure transmitter | [Amazon B07WDK2PRN](https://www.amazon.com/dp/B07WDK2PRN) |
| 1 | Three-wire waterproof DS18B20-compatible probe | Installed field temperature probe |
| 1 | 5.1 kΩ resistor | DS18B20 DATA-to-3.3 V pull-up; 4.7 kΩ is nominal and interchangeable here |
| 1 | 100 Ω resistor | Pressure-loop shunt; use 0.1%, low-TC for the final build if available |
| 1 | 1100 mAh protected 1S Li-ion pack | Current endurance-test battery |
| 1 | Electrocookie/perfboard | Soldered interconnect board |
| 2 | 3-pin JST-XH board connectors | Temperature and pressure harnesses |
| 2+ | Cable glands sized for actual cable diameters | Watertight enclosure exits and strain relief |
| 1 | Weather-resistant enclosure | Electronics enclosure; verify it does not press reset or solder joints |
| 2 | Individually insulated probe harnesses | Protect every conductor splice separately |

## Recommended production additions

| Qty. | Part | Reason |
|---:|---|---|
| 1 | AO3401A P-channel MOSFET | High-side switch selected by the consolidated-board design |
| 2 | 5.1 kΩ resistor | MOSFET gate series (`RG`) and switched-rail bleed (`RB`) |
| 1 | 51 kΩ resistor | MOSFET gate pull-up/default-off (`RPU`) |
| 3 | 100 nF capacitor | Gate ramp (`CG`), boost bypass (`CI`), and ADS A0 filter (`CF`) |
| 1 | 1 kΩ resistor | ADS1115 A0 series filter (`RF`) |
| 1 | Electrocookie Large 30 x 10 + 4-rail board | Consolidates RAK, ADS1115, boost, switch, filters, and connectors; see [build map](consolidated-board.md) |
| 1 | JST-XH 3-pin connector | Consolidated-board DS18B20 connector |
| 1 | JST-XH 2-pin connector | Consolidated-board 24 V pressure-loop connector |
| 1 set | Calsgkspray/pingwave M12 breather vents, ASIN B0G1HLJVS3 | **Ordered prototype part:** four M12 x 1.5 pressure-equalization vents advertised as IP68; [Amazon](https://www.amazon.com/dp/B0G1HLJVS3). Listing says mounting hardware is included, but verify an O-ring and locknut are present before drilling. The known-specification alternative is the [TAKACHI PMF-12B](https://www.takachi-enclosure.com/products/PMF). |
| DNF | Optional bulk capacitor at `CB` | Leave unpopulated initially; fit only if measured switched-rail sag requires it |
| 1 | Larger protected battery or solar charging system | Required for multi-day deployment before power optimization |
| 1 | Perforated stilling tube/guard | Protect diaphragm and reduce wave/debris effects while allowing pressure transfer |

The 100 Ω shunt already listed in the prototype BOM is reused by the new
layout, but the build map calls for a 1% metal-film part and routes it with a
Kelvin ground reference. A 0.1%, low-temperature-coefficient part remains the
preferred accuracy upgrade.

## Temperature-probe sourcing

The inexpensive Prime-friendly bench suggestion remains the
[HiLetgo waterproof DS18B20 5-pack, B00M1PM55K](https://www.amazon.com/dp/B00M1PM55K),
but inexpensive encapsulated probes may be compatible/counterfeit parts. The
installed probe required a measured `-5.0 °C` firmware correction. Validate
each replacement in a dense, stirred ice-water slurry and beside a trusted
room-temperature reference.

## Do not substitute casually

- A battery JST cable may be wired backward even if it physically fits.
- Do not replace the 100 Ω shunt with an arbitrary value without updating the
  firmware conversion and recalibrating.
- Do not power the pressure transmitter directly from 3.3 V; it requires the
  24 V loop supply.
- Do not expose the RAK, ADS1115, or DS18B20 to the boost converter's 24 V rail.
