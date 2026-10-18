# Compatibility Profiles

Compatibility Profiles let you switch FreeCAD presentation toward **FreeCAD Native**,
**Creo Inspired**, or **SolidWorks Inspired** habits without changing the OCCT geometry
kernel.

Vendor names are **inspired** labels only. This feature is not affiliated with PTC or
Dassault Systemes.

## What a profile changes

Each profile applies a Preference Pack stack:

- Theme YAML tokens (`Gui/Stylesheets/parameters/`)
- Navigation style (`Gui::CADNavigationStyle`, `Gui::CreoInspiredNavigationStyle`,
  or `Gui::SolidWorksNavigationStyle`)
- Curated shortcuts
- Main-window dock/toolbar defaults

## How to switch

1. Start FreeCAD.
2. Open **Tools → Compatibility Profiles**, or
   **Edit → Preferences → General → Compatibility Profiles**.
3. Choose a profile and click **Apply profile**.
4. Use **Revert to baseline** to restore the configuration captured before the first
   Apply in this profile session.

The first Apply stores a baseline snapshot under the user FreeCAD config directory:

`CompatibilityProfiles/compatibility_profile_baseline.cfg`

Switching profiles restores that baseline first, then applies the new pack so merges
do not stack.

## Command facade

Each profile includes a small curated map of vendor-facing labels to FreeCAD command
names (for example `Zoom to Fit` → `Std_ViewFitAll`). The map is shown on the
preferences page. Full ribbon parity is out of scope for this phase.

## Windows build and test

This Linux cloud environment cannot produce a Windows `.exe`. Use one of these paths
on a Windows machine:

### Option A — Pixi / conda bundle (recommended for a portable tree)

```bat
git clone <your-fork-url> FreeCAD
cd FreeCAD
git checkout cursor/compatibility-profiles-d60f
pixi install
pixi run build
pixi run package-windows
```

See `package/bundle/windows/create_bundle.sh` and `package/bundle/pixi.toml` for the
Windows packaging entry points used by the project.

### Option B — GitHub Actions Windows CI

Push this branch and run (or wait for) the Windows workflow:

- `.github/workflows/sub_buildWindows.yml`
- Invoked from `.github/workflows/CI_primary.yml` / release workflows

Download the FreeCAD Windows artifact from the successful run and install or unpack
it locally.

### Option C — MSVC + LibPack (classic)

Follow upstream FreeCAD Windows LibPack instructions, configure with CMake against
the LibPack, build `FreeCAD` / `FreeCADMain`, and run `bin\FreeCAD.exe`.

## Smoke test checklist

1. Preferences page lists FreeCAD Native, Creo Inspired, SolidWorks Inspired.
2. Apply Creo Inspired → theme darkens, Navigation style becomes Creo Inspired,
   middle-mouse rotate / Shift+middle pan / Ctrl+middle zoom.
3. Apply SolidWorks Inspired → SolidWorks navigation, light theme accents.
4. Revert → previous user baseline returns.
5. Restart once if a viewer does not pick up navigation immediately.
