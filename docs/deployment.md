# Deployment and Operations

## Deployment checklist

- Charge the protected 1S battery fully and record start time/voltage.
- Confirm both 915 MHz antennas are attached.
- Confirm flags are `0x0000` and sequence numbers advance.
- Verify enclosure cable glands and strain relief; the lid must not pinch the
  pressure cable/vent or press reset buttons, JSTs, or solder joints.
- Verify the dry enclosure breathes through a hydrophobic ePTFE pressure-
  equalization vent. Do not seal a gauge transmitter's reference vent.
- Individually insulate every splice and keep all electronics/splices dry.
- Flood and tighten the pressure cap underwater.
- Measure depth from the diaphragm and compare with Home Assistant before
  leaving the site.
- Leave the USB-powered Heltec gateway online for the whole deployment.

## Current field behavior

The lake node transmits frequently and keeps the 24 V pressure loop powered.
The current 1100 mAh battery is expected to last roughly 16–24 hours. A full
USB-disconnected endurance run began 2026-08-22 at 21:42 EDT with 4.213 V.

Home Assistant Recorder stores each changing sequence number. If the node
stops, the exact last received packet time is available; electrical failure is
bounded between that packet and the next expected packet, approximately 30
seconds later.

## Interpreting failures

| Observation | Likely cause |
|---|---|
| Gateway offline | Heltec power, Wi-Fi, or MQTT failure |
| Gateway online; sequence stopped | Lake-node battery, radio, reset, or wiring failure |
| Loop current near/below 3.6 mA | Open/underpowered pressure loop |
| Depth zero but current plausible | Calibration/clamping or trapped pressure-cap air |
| Temperature invalid flag | DS18B20 power/data/ground fault or CRC failure |
| Entire 3.3 V rail low | Harness short; inspect individually insulated splices |

## Production improvements

Add a MOSFET/load switch for the boost and pressure loop, deep sleep between
samples, a per-node MQTT availability watchdog, and a low-battery shutdown.
Recalibrate pressure after changing power sequencing because warm-up affects
zero. Use a protected battery; current firmware warns at 3.4 V but does not
disconnect the load.

Do not charge a wet/open enclosure or a Li-ion cell below its specified charging
temperature. Do not attach a solar panel directly to USB, battery, or 5 V pins.

## Enclosure pressure vent

Use a **TAKACHI PMF-12B** or equivalent M12 x 1.5 hydrophobic membrane
pressure-equalization vent. It is gasketed, rated IP68, and sized for sealed
enclosures up to 5 L:

- [TAKACHI PMF-series specification](https://www.takachi-enclosure.com/products/PMF)
- [MISUMI US purchase page](https://us.misumi-ec.com/vona2/detail/222000225926/?HissuCode=PMF-12B)
- [Ordered prototype: Calsgkspray/pingwave B0G1HLJVS3](https://www.amazon.com/dp/B0G1HLJVS3)
- [Kryoflare B0FJ654QSP](https://www.amazon.com/dp/B0FJ654QSP) — avoid for
  this enclosure unless an appropriate locknut is separately confirmed; the
  listing describes a threaded breather but does not list a locknut.

For a thin plastic enclosure wall, use a bulkhead style supplied with both an
O-ring and locknut. A screw-only vent requires a correctly tapped M12 x 1.5
boss and must not simply be forced into a drilled plastic hole. Verify that air
passes through the selected vent before installation, then verify the assembled
enclosure with a dry paper-towel leak test before installing electronics.

The ordered B0G1HLJVS3 listing says that mounting hardware is included but does
not enumerate it. On arrival, confirm that the set contains an O-ring and an
M12 x 1.5 locknut. Do not drill the enclosure until the actual threaded-body
diameter, gasket land, usable thread length, and required clearance hole have
been measured.

Drill the specified 12.2 mm mounting hole and install it with its O-ring and
locknut. Put it high on a vertical wall, with the exposed face sideways or
downward, and above the highest expected waterline. Do not coat the membrane
with sealant or rely on it for permanent submersion. The lid and cable glands
still require their own watertight seals.
