#!/usr/bin/env python3
"""Validate Compatibility Profile data files without building FreeCAD."""

from __future__ import annotations

import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PACK_ROOT = ROOT / "src" / "Gui" / "PreferencePacks"
STYLE_ROOT = ROOT / "src" / "Gui" / "Stylesheets" / "parameters"

REQUIRED_PACKS = (
    "FreeCAD Native",
    "Creo Inspired",
    "SolidWorks Inspired",
)
REQUIRED_THEMES = (
    "Creo Inspired.yaml",
    "SolidWorks Inspired.yaml",
)
REQUIRED_NAV = (
    ROOT / "src" / "Gui" / "Navigation" / "CreoInspiredNavigationStyle.cpp",
    ROOT / "src" / "Gui" / "CompatibilityProfileManager.cpp",
    ROOT / "src" / "Gui" / "PreferencePages" / "DlgSettingsCompatibilityProfiles.cpp",
)


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    package_xml = PACK_ROOT / "package.xml"
    if not package_xml.is_file():
        fail(f"missing {package_xml}")

    tree = ET.parse(package_xml)
    names = {node.findtext("name") for node in tree.findall(".//preferencepack")}
    for pack in REQUIRED_PACKS:
        if pack not in names:
            fail(f"package.xml missing preference pack '{pack}'")
        cfg = PACK_ROOT / pack / f"{pack}.cfg"
        if not cfg.is_file():
            fail(f"missing pack config {cfg}")
        ET.parse(cfg)

    for theme in REQUIRED_THEMES:
        path = STYLE_ROOT / theme
        if not path.is_file():
            fail(f"missing theme tokens {path}")
        text = path.read_text(encoding="utf-8")
        if "PrimaryColor:" not in text:
            fail(f"{path} has no PrimaryColor token")

    for path in REQUIRED_NAV:
        if not path.is_file():
            fail(f"missing source file {path}")

    cmake_lists = (ROOT / "src" / "Gui" / "CMakeLists.txt").read_text(encoding="utf-8")
    if "CreoInspiredNavigationStyle.cpp" not in cmake_lists:
        fail("Gui CMakeLists.txt does not list CreoInspiredNavigationStyle.cpp")
    if "CompatibilityProfileManager.cpp" not in cmake_lists:
        fail("Gui CMakeLists.txt does not list CompatibilityProfileManager.cpp")

    print("Compatibility Profiles data check passed.")


if __name__ == "__main__":
    main()
