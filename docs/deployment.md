# Deployment Notes

- Mount the probe below expected minimum water and above sediment in a fixed,
  perforated PVC stilling well.
- Secure the temperature probe beside the pressure probe without obstructing
  the pressure port.
- Put electronics above plausible flood level in a shaded, UV-resistant,
  vented weatherproof enclosure with strain relief and desiccant.
- Keep boost switching parts away from the ADS1115 and antenna.
- For vented gauge sensors, keep the dry vent opening open, dry, un-kinked, and
  above flood level. An absolute sensor requires barometric compensation.
- Long exposed cable runs need later surge/lightning hardening.
- Use an external solar-aware 1S charger with temperature cutoff if solar is
  added; do not attach a panel directly to the battery, USB, or 5 V pin.

Before outdoor deployment, record actual sleep current, wake-cycle energy,
packet loss at the lake-to-house path, battery calibration, temperature-probe
ROM address/offset, and manual gauge agreement against `design.md`.
