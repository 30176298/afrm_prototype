#!/usr/bin/env python3
"""Independent oracle for the AFRM fuel model

Computes expected values for both model versions:
  v1.0  constant fuel flow at take-off weight
  v1.1  weight-dependent integration, reserves at landing weight, taxi allowance (CR-001)

Usage:  python3 tests/oracle/fuel_oracle.py [data_dir]
"""
import csv
import math
import sys
from pathlib import Path

CONTINGENCY_PCT, ALTERNATE_NM, FINAL_MIN, MARGINAL_PCT, TAXI_KG = 5.0, 100.0, 30.0, 5.0, 100.0


def read(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def status(margin, required):
    if margin < 0:
        return "Insufficient"
    return "Marginal" if margin < required * MARGINAL_PCT / 100 else "Feasible"


def flow(aircraft, weight_kg):
    k, s = float(aircraft["burn_kg_h_per_t"]), float(aircraft["stores_burn_pct"])
    return k * weight_kg / 1000.0 * (1 + s / 100.0)


def assess(flight, aircraft, version, taxi=TAXI_KG, cont=CONTINGENCY_PCT, alt_nm=ALTERNATE_NM, final_min=FINAL_MIN):
    t = [float(r["time_s"]) for r in flight]
    d = [float(r["distance_nm"]) for r in flight]
    w0 = float(aircraft["takeoff_weight_kg"])
    onboard = float(aircraft["fuel_onboard_kg"])
    hours, dist = (t[-1] - t[0]) / 3600.0, d[-1] - d[0]

    if version == "1.0":
        trip = flow(aircraft, w0) * hours
        landing_w, taxi = w0, 0.0
    else:
        w, trip = w0, 0.0
        for i in range(1, len(t)):
            burn = flow(aircraft, w) * (t[i] - t[i - 1]) / 3600.0
            burn = min(burn, w)
            trip += burn
            w -= burn
        landing_w = w0 - trip

    contingency = trip * cont / 100.0
    if version == "1.0":
        alternate = trip / dist * alt_nm
    else:
        alternate = flow(aircraft, landing_w) * (alt_nm * hours / dist)
    final = flow(aircraft, landing_w) * final_min / 60.0
    required = trip + contingency + alternate + final + taxi
    margin = onboard - required
    bpn = trip / dist
    rng = max(0.0, (onboard - taxi - final - alternate) / (1 + cont / 100.0) / bpn)
    return dict(trip=trip, contingency=contingency, alternate=alternate, final=final, taxi=taxi,
                required=required, margin=margin, range=rng, status=status(margin, required))


def closed_form_trip(flight, aircraft):
    """Exact solution of dW/dt = -c W: independent check on the stepwise sum."""
    k, s = float(aircraft["burn_kg_h_per_t"]), float(aircraft["stores_burn_pct"])
    c = k * (1 + s / 100.0) / 1000.0 / 3600.0
    T = float(flight[-1]["time_s"]) - float(flight[0]["time_s"])
    w0 = float(aircraft["takeoff_weight_kg"])
    return w0 * (1 - math.exp(-c * T))


def main():
    data = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[2] / "data"
    flights = {"short": "flight_short.csv", "medium": "flight_medium.csv", "long": "flight_long.csv"}
    aircraft = {"clean": "aircraft_clean.csv", "stores": "aircraft_stores.csv"}
    for ver in ("1.0", "1.1"):
        print(f"\n=== model v{ver} (taxi {'0' if ver == '1.0' else TAXI_KG:} kg) ===")
        print(f"{'flight':7}{'aircraft':9}{'trip':>9}{'alt':>8}{'final':>8}{'required':>10}{'margin':>10}{'range':>8}  status   exact-trip")
        for fn, ff in flights.items():
            for an, af in aircraft.items():
                fl, ac = read(data / ff), read(data / af)[0]
                r = assess(fl, ac, ver)
                ex = closed_form_trip(fl, ac) if ver == "1.1" else float("nan")
                print(f"{fn:7}{an:9}{r['trip']:9.1f}{r['alternate']:8.1f}{r['final']:8.1f}{r['required']:10.1f}"
                      f"{r['margin']:10.1f}{r['range']:8.1f}  {r['status']:12}{ex:9.1f}")


if __name__ == "__main__":
    main()
