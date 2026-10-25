# Creo evidence capture (Windows worker)

Cloud Linux agents cannot open `C:\Program Files\PTC\...`.
Use the FreeCADBG self-hosted worker on DESKTOP-8I3RN1B
(`workerId` `833d56bc-403a-5a5b-8d94-f263a269a52e`), or run the dump script
on that PC and commit/paste the JSON.

## Status artifacts

| File | Meaning |
|------|---------|
| `inventory_cloud_linux_blocked.json` | Cloud Linux run: host mismatch; **zero** Creo strings observed |
| `creo_string_dump.json` / `.txt` | Produced by Windows script or gcad inventory (not present until run on Creo PC) |
| `Extract-CreoRibbonStrings.ps1` | Read-only PE size + ASCII/UTF-16LE needle scan |

Do **not** treat FreeCAD `creoLayout()` labels as Creo evidence.

## Target

- `C:\Program Files\PTC\Creo 7.0.3.0\Parametric\bin\parametric.exe`
- Sibling `xtop.exe` (often the real UI binary)

## PowerShell dump (run on the Creo PC)

```powershell
cd C:\Users\user\Projects\freecadbg-1   # or your FreeCADBG checkout
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\creo_evidence\Extract-CreoRibbonStrings.ps1
```

Writes `tools/creo_evidence/creo_string_dump.json` and `.txt`.

## Agent prompt (pin chat to FreeCADBG Windows worker)

```text
Inventory Creo 7.0.3.0 for FreeCAD Creo-inspired ribbon evidence.
Path: C:\Program Files\PTC\Creo 7.0.3.0\Parametric\bin\parametric.exe
Also open sibling xtop.exe. List bin *.exe/*.dll with sizes and PE kind.
Use gcad: binary_session, open_binary, inspect_pe_resources, search_strings
for: Get Data, Datums, Shapes, Round, Chamfer, Model, Annotate, Extrude,
Revolve, Sweep, Blend, Mirror.
If gcad cannot open: run tools/creo_evidence/Extract-CreoRibbonStrings.ps1.
Save raw findings under tools/creo_evidence/. Return structured report only
from observed strings/resources. No guessed Creo UX. No RibbonCatalog.cpp
edits unless evidence clearly maps.
```
