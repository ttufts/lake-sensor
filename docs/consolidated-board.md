# Consolidated Lake-Node Board (Planned Revision)

The authoritative assembly drawing is the five-page
[Lake Node Build Map PDF](artifacts/lake-node-build-map.pdf), received
2026-08-24. It places the working prototype circuit and the planned
GPIO-controlled pressure-loop power switch on one Electrocookie Large board
(30 x 10 tie points plus four power rails).

This revision has **not yet been built or electrically validated**. The node in
the lake still uses the continuously powered prototype documented in
[wiring.md](wiring.md). Treat this page and the PDF as the build target, not as
proof that the switch circuit is already installed.

## Electrical changes from the field prototype

- An AO3401A P-channel MOSFET high-side-switches the MT3608 input rail.
- RAK19003 `TXD` (`C15`) becomes the switch-control GPIO through a 5.1 kΩ gate
  resistor. A 51 kΩ gate-to-source pull-up makes power-off the default.
- A 100 nF gate-to-source capacitor ramps turn-on; the design estimate is about
  464 microseconds and roughly 50 mA startup inrush.
- The 100 Ω loop shunt uses a Kelvin ground return to ADS1115 GND. The 20 mA
  loop return leaves directly from the shunt low-side star point.
- ADS1115 A0 gains a 1 kΩ/100 nF low-pass filter.
- The switched 3.3 V rail gains a 5.1 kΩ bleed and 100 nF bypass capacitor.
- The pressure transmitter uses a two-pin 24 V/return connector. The DS18B20
  retains a three-pin `+ / DATA / -` connector and 5.1 kΩ pull-up.

## Module pin coordinates

| Module | Board row and pins |
|---|---|
| AO3401A | Row C, columns 6-8; source `C8`, drain `C7`, gate `C6` |
| MT3608 | Columns 7-14; input is on the column-7 end, output on column 14 |
| RAK19003 breakout | Row C; `RXD C14`, `TXD C15`, `GND C16`, `BOOT C17`, `SDA C21`, `SCL C22`, `GND C23`, `VDD C24` |
| ADS1115 | Row H; `VDD H16`, `GND H17`, `SCL H18`, `SDA H19`, `ADDR H20`, `ALRT H21`, `A0 H22`, `A1 H23`, `A2 H24`, `A3 H25` |
| DS18B20 JST-XH 3P | `+` column 26, DATA column 27, `-` column 28 |
| Pressure XH 2P | `24 V` column 27, transmitter return column 28 |

Verify the footprint orientation against the PDF before soldering; row segments
on this board are internally tied and physical adjacency is not a substitute
for the named nets.

## Jumper solder order

Install under-module wires before mounting the RAK and ADS1115. In particular,
`B15`, `A23`, `A24`, and the 1-Wire run pass under the RAK; `I17`, `I20`, `I22`,
`J16`, `J20`, and `J22` pass under the ADS1115.

| # | Jumper | Function |
|---:|---|---|
| 1 | `P1 -> R1` | Join ground rails around the left end |
| 2 | `Q1 -> S1` | Join 3.3 V rails around the left end, outside the ground link |
| 3 | `A24 -> Q24` | RAK VDD to always-on 3.3 V rail |
| 4 | `A23 -> P23` | RAK GND to ground rail, hopping the 3.3 V rail |
| 5 | `A8 -> Q8` | AO3401A source to always-on 3.3 V |
| 6 | `B15 -> B11` | RAK TXD to gate resistor |
| 7 | `E7 -> F7` | AO3401A drain to MT3608 input |
| 8 | `J8 -> R8` | MT3608 IN- to ground rail |
| 9 | `J14 -> R14` | MT3608 OUT- to ground rail |
| 10 | `J13 -> J27` | MT3608 OUT+ (24 V) to pressure connector |
| 11 | `I30 -> I17` | Kelvin shunt-low reference to ADS GND |
| 12 | `J30 -> R30` | Shunt-low star point to loop-return ground rail |
| 13 | `J26 -> J22` | Filtered sense node to ADS1115 A0 |
| 14 | `J16 -> S16` | ADS VDD to 3.3 V rail, hopping ground |
| 15 | `J20 -> R20` | ADS ADDR to ground (`0x48`) |
| 16 | `E21 -> F19` | RAK SDA to ADS SDA |
| 17 | `E22 -> F18` | RAK SCL to ADS SCL |
| 18 | `A27 -> A14` | DS18B20 DATA to RAK RXD |
| 19 | `A26 -> Q26` | DS18B20 `+` to 3.3 V rail |
| 20 | `A28 -> P28` | DS18B20 `-` to ground rail, hopping 3.3 V |

## Discrete components

| Ref | Value | Function | Holes |
|---|---:|---|---|
| `RG` | 5.1 kΩ | MOSFET gate series | `E9 <-> E11` |
| `RPU` | 51 kΩ | Gate pull-up/default off | `B9 <-> B8` |
| `CG` | 100 nF | Gate-to-source ramp capacitor | `D9 <-> D8` |
| `CB` | DNF | Optional bulk-cap footprint | `Q12 <-> P12` |
| `CI` | 100 nF | MT3608 input bypass | `G7 <-> G8` |
| `RB` | 5.1 kΩ | Switched-rail bleed | `I7 <-> I8` |
| `RSH` | 100 Ω, 1% metal film | Pressure-loop shunt | `H28 <-> H30` |
| `RF` | 1 kΩ | ADS1115 A0 series filter | `I28 <-> I26` |
| `CF` | 100 nF | ADS1115 A0 filter to Kelvin ground | `I22 <-> I20` |
| `R1W` | 5.1 kΩ | DS18B20 DATA pull-up | `B27 <-> B26` |

`CB` is deliberately not fitted for the initial build. The MT3608 already has
an input capacitor; populate the optional footprint only if measured switched-
rail sag or startup behavior justifies it.

## Kelvin shunt rule

The shunt low side at column 30 is a star point with two distinct conductors:

- `I30 -> I17` is the measurement reference and carries only ADS1115 input/
  supply return current.
- `J30 -> R30` carries the pressure loop's approximately 20 mA back to the
  ground rail.

Do not daisy-chain the loop current through ADS GND. Voltage drop in that shared
path would be added directly to the pressure measurement.

## Firmware dependency

The present firmware does not yet drive TXD as the pressure-power control.
Before using this revision, implement and bench-test a power sequence that:

1. holds the P-channel gate high/off during reset and sleep;
2. drives the gate low to enable the MT3608;
3. waits for boost and transmitter stabilization;
4. samples ADS1115 A0; and
5. disables the loop after sampling.

Recalibrate pressure after switching is enabled because warm-up time can affect
the zero and span.
