# Bill of Materials

| Qty. | Part | Requirements / notes |
|---:|---|---|
| 1 | Heltec WiFi LoRa 32 V3/V3.2 | US 902–928 MHz version |
| 1 | 915 MHz LoRa antenna | Attach before transmitting |
| 1 | 4–20 mA submersible level transmitter | Two-wire, loop-powered, nominal 0–5 m; verify vent and polarity |
| 1 | Waterproof DS18B20 temperature probe | Three-wire, 3.0–5.5 V, stainless probe; cable long enough for fixed submerged placement |
| 1 | 4.7 kΩ resistor | DS18B20 data pull-up to switched 3.3 V Vext |
| 1 | ADS1115 breakout | 3.3 V compatible; default address `0x48` |
| 1 | Adjustable boost converter | Starts at 3.3 V; maintains 24 V at 20 mA output |
| 1 | 100 Ω shunt resistor | 0.1%, low temperature coefficient, at least 0.25 W |
| 1 | 1 kΩ resistor | Series resistor from shunt sense node to ADS1115 A0 |
| 1 | 100 nF capacitor | ADS1115 A0-to-ground filter |
| 1 | 47–100 µF capacitor | Optional boost-input startup reservoir |
| 1 | Protected 18650 cell | Reputable 3000–3500 mAh 1S cell |
| 1 | Quality holder/protected pack | Do not solder directly to a bare cell |
| 1 | SH1.25 battery lead | Verify polarity with a multimeter |
| 1 | Weatherproof enclosure | Mount above flood level |
| 3+ | Cable glands | Pressure cable, temperature cable, optional charging/solar |
| 1 | Perforated PVC stilling well | Protects both probes from waves and debris |
| 1 | House-side LoRa gateway | Second Heltec V3 recommended |

## Temperature probe recommendation

For the first prototype, the recommended Amazon option is the
[HiLetgo 5-pack waterproof 1 m DS18B20 probe, ASIN B00M1PM55K](https://www.amazon.com/dp/B00M1PM55K).
It runs from 3.0–5.5 V and provides inexpensive spares for comparison testing.
Amazon showed it in stock at about $10.99, sold by HiLetgo and fulfilled by
Amazon when checked on 2026-08-18. Confirm the Prime badge, delivery date, and
cable length for the delivery ZIP before ordering because those vary by account.

Low-cost encapsulated probes may contain compatible rather than genuine Analog
Devices parts. Compare all probes together in stirred ice water and room-
temperature water, reject outliers, and keep a spare. For higher confidence, the
3 m Adafruit probe (product 3846) is a stronger-quality alternative, though it
is not the requested Amazon Prime option.

