#!/usr/bin/env python3
"""Emit package_lgt8funlocked_index.json for the Arduino Boards Manager.

Usage: make_package_index.py <release-zip> <release-tag> [--version X.Y.Z]
Reads boards.txt / platform.txt in the repo root. Writes the index to stdout
(ASCII-escaped, as required by the Arduino index validator).

Boards-manager archive layout (required by arduino-cli/IDE): the zip must
contain exactly ONE root folder (convention: "avr") with boards.txt and
platform.txt directly inside it at depth 1, e.g. avr/boards.txt. The
installer re-homes that root folder's contents into the version directory.
A flat archive (boards.txt at zip root) is rejected ("no unique root dir"),
and a manual-install zip that nests lgt8funlocked/avr/... installs but
hides the board from the IDE menu.

The index URL to share with users is
  https://cdn.jsdelivr.net/gh/luckyprdn/lgt8f328p-unlocked-core@main/package_lgt8funlocked_index.json
"""
import argparse
import hashlib
import json
import os
import re
import sys

GITHUB = "https://github.com/luckyprdn/lgt8f328p-unlocked-core"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("zip")
    ap.add_argument("tag")
    ap.add_argument("--version", default=None)
    args = ap.parse_args()

    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    boards_txt = open(os.path.join(root, "boards.txt"), encoding="utf-8").read()
    plat_txt = open(os.path.join(root, "platform.txt"), encoding="utf-8").read()
    plat_version = re.search(r"^version=(.+)$", plat_txt, re.M).group(1).strip()
    version = args.version or plat_version
    plat_name = re.search(r"^name=(.+)$", plat_txt, re.M).group(1).strip()

    boards = []
    for m in re.finditer(r"^([A-Za-z0-9_]+)\.name=(.+)$", boards_txt, re.M):
        boards.append({"name": m.group(2).strip()})

    raw = open(args.zip, "rb").read()
    sha = hashlib.sha256(raw).hexdigest()
    size = len(raw)
    fname = os.path.basename(args.zip)

    index = {
        "packages": [
            {
                "name": "lgt8funlocked",
                "maintainer": "LGT8F328P unlocked-core contributors",
                "websiteURL": GITHUB,
                "email": "luckyprdn@users.noreply.github.com",
                "help": {
                    "online": f"{GITHUB}/blob/main/docs/INSTALL.md",
                },
                "platforms": [
                    {
                        "name": plat_name,
                        "architecture": "avr",
                        "version": version,
                        "category": "Contributed",
                        "url": f"{GITHUB}/releases/download/{args.tag}/{fname}",
                        "archiveFileName": fname,
                        "checksum": f"SHA-256:{sha}",
                        "size": str(size),
                        "boards": boards,
                        "toolsDependencies": [
                            {
                                "packager": "arduino",
                                "name": "avr-gcc",
                                "version": "7.3.0-atmel3.6.1-arduino7",
                            },
                            {
                                "packager": "arduino",
                                "name": "avrdude",
                                "version": "6.3.0-arduino17",
                            },
                        ],
                    }
                ],
                "tools": [],
            }
        ]
    }
    # ASCII-only: Arduino's index parser requires it (no raw non-ASCII bytes).
    sys.stdout.write(json.dumps(index, ensure_ascii=True, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
