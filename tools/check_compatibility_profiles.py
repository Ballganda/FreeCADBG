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
REQUIRED_SOURCES = (
    ROOT / "src" / "Gui" / "Navigation" / "CreoInspiredNavigationStyle.cpp",
    ROOT / "src" / "Gui" / "CompatibilityProfileManager.cpp",
    ROOT / "src" / "Gui" / "PreferencePages" / "DlgSettingsCompatibilityProfiles.cpp",
    ROOT / "src" / "Gui" / "RibbonDefinition.h",
    ROOT / "src" / "Gui" / "RibbonCatalog.h",
    ROOT / "src" / "Gui" / "RibbonCatalog.cpp",
    ROOT / "src" / "Gui" / "RibbonWidget.h",
    ROOT / "src" / "Gui" / "RibbonWidget.cpp",
    ROOT / "src" / "Gui" / "RibbonManager.h",
    ROOT / "src" / "Gui" / "RibbonManager.cpp",
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

    for path in REQUIRED_SOURCES:
        if not path.is_file():
            fail(f"missing source file {path}")

    cmake_lists = (ROOT / "src" / "Gui" / "CMakeLists.txt").read_text(encoding="utf-8")
    if "CreoInspiredNavigationStyle.cpp" not in cmake_lists:
        fail("Gui CMakeLists.txt does not list CreoInspiredNavigationStyle.cpp")
    if "CompatibilityProfileManager.cpp" not in cmake_lists:
        fail("Gui CMakeLists.txt does not list CompatibilityProfileManager.cpp")
    for ribbon_src in ("RibbonWidget.cpp", "RibbonCatalog.cpp", "RibbonManager.cpp"):
        if ribbon_src not in cmake_lists:
            fail(f"Gui CMakeLists.txt does not list {ribbon_src}")

    catalog = (ROOT / "src" / "Gui" / "RibbonCatalog.cpp").read_text(encoding="utf-8")
    if "sharedSkeleton" in catalog:
        fail("RibbonCatalog.cpp must not use a shared skeleton for Creo/SW ribbons")
    if "creoLayout" not in catalog or "solidWorksLayout" not in catalog:
        fail("RibbonCatalog.cpp must define separate creoLayout and solidWorksLayout")
    if '"creo_model"' not in catalog or '"sw_features"' not in catalog:
        fail("RibbonCatalog.cpp missing Creo Model / SolidWorks Features tab ids")

    prefs = (
        ROOT / "src" / "Gui" / "PreferencePages" / "DlgSettingsCompatibilityProfiles.cpp"
    ).read_text(encoding="utf-8")
    if "Show ribbon" not in prefs or "Hide classic toolbars" not in prefs:
        fail("preferences page missing ribbon checkboxes")
    if "setRibbonEnabled" not in prefs or "setHideClassicToolbars" not in prefs:
        fail("preferences page must wire ribbon checkboxes to RibbonManager")

    print("Compatibility Profiles data check passed.")


if __name__ == "__main__":
    main()
