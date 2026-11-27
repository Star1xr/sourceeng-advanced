#!/usr/bin/env python3
"""
Compare the dynamic-combo count of every shipped .vcs (logged by the engine as
"[vcs] <file>: version N, dynamic combos D, total T") with the count our
materialsystem/stdshaders/fxctmp9/*.inc headers assume.

A mismatch means every combo index for that shader is off: the engine then picks
the wrong variant, or none ("Couldn't load combo N of shader (dyn=D)").

  python3 scripts/check_vcs_dynamic_combos.py <engine log> [fxctmp9 dir]
"""
import os
import re
import sys


def expected_dynamic_combos(inc_path):
    """Product of the (max + 1) of every setter in the *_Dynamic_Index class."""
    text = open(inc_path, encoding="latin-1").read()
    m = re.search(r"class\s+\w+_Dynamic_Index\b(.*?)\n};", text, re.S)
    if not m:
        return 1  # shader without dynamic combos
    total = 1
    for hi in re.findall(r"Assert\(\s*i\s*>=\s*0\s*&&\s*i\s*<=\s*(\d+)\s*\)", m.group(1)):
        total *= int(hi) + 1
    return total


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    log = sys.argv[1]
    incdir = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "materialsystem", "stdshaders", "fxctmp9")

    seen = {}
    for line in open(log, encoding="utf-8", errors="replace"):
        m = re.search(r"\[vcs\]\s+(\S+?\.vcs):\s+version\s+(\d+),\s+dynamic combos\s+(\d+)", line)
        if m:
            name = os.path.basename(m.group(1).replace("\\", "/"))[:-4]
            seen[name] = int(m.group(3))

    if not seen:
        print("no '[vcs]' lines found in the log (is it from a build with the vcs logging?)")
        return 2

    bad = ok = skipped = 0
    for name in sorted(seen):
        inc = os.path.join(incdir, name + ".inc")
        if not os.path.isfile(inc):
            skipped += 1
            continue
        exp = expected_dynamic_combos(inc)
        if exp == seen[name]:
            ok += 1
        else:
            bad += 1
            print("MISMATCH %-48s vcs=%-5d header=%d" % (name, seen[name], exp))
    print("\n%d match, %d MISMATCH, %d without a header (hlsl/other)" % (ok, bad, skipped))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
