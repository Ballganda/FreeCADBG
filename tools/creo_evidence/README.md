# Creo evidence capture (Windows worker)

Cloud Linux agents cannot open `C:\Program Files\PTC\...`.
Use the FreeCADBG self-hosted worker on DESKTOP-8I3RN1B, or run the dump script below and paste output into the cloud chat.

## Target

- `C:\Program Files\PTC\Creo 7.0.3.0\Parametric\bin\parametric.exe`
- Also check sibling `xtop.exe` (often the real UI binary)

## PowerShell dump (run on the Creo PC)

```powershell
$bin = 'C:\Program Files\PTC\Creo 7.0.3.0\Parametric\bin'
$out = Join-Path $PWD 'tools\creo_evidence\creo_string_dump.txt'
New-Item -ItemType Directory -Force -Path (Split-Path $out) | Out-Null
$needles = @('Get Data','Datums','Shapes','Editing','Operations','Round','Chamfer','Extrude','Revolve','Sweep','Blend','Annotate','Model','Mirror','Full Round')
$files = Get-ChildItem -LiteralPath $bin -Include *.exe,*.dll -File -ErrorAction SilentlyContinue
"=== FILES ===" | Set-Content -LiteralPath $out
$files | Select-Object Name,Length,FullName | Format-Table -AutoSize | Out-String | Add-Content -LiteralPath $out
"=== STRING HITS ===" | Add-Content -LiteralPath $out
foreach ($f in $files) {
  foreach ($n in $needles) {
    $hits = Select-String -LiteralPath $f.FullName -Pattern $n -Encoding byte -ErrorAction SilentlyContinue
    # Fallback: ASCII scan via .NET for large binaries
  }
}
# ASCII UTF-16LE scan (Creo often stores UI as UTF-16)
Add-Type -TypeDefinition @"
using System; using System.IO; using System.Text;
public static class CreoScan {
  public static void Run(string path, string[] needles, string outPath) {
    var bytes = File.ReadAllBytes(path);
    var ascii = Encoding.ASCII.GetString(bytes);
    var utf16 = Encoding.Unicode.GetString(bytes);
    using (var w = File.AppendText(outPath)) {
      w.WriteLine("--- " + path + " ---");
      foreach (var n in needles) {
        if (ascii.IndexOf(n, StringComparison.OrdinalIgnoreCase) >= 0)
          w.WriteLine("ASCII: " + n);
        if (utf16.IndexOf(n, StringComparison.OrdinalIgnoreCase) >= 0)
          w.WriteLine("UTF16: " + n);
      }
    }
  }
}
"@
foreach ($f in @(
  (Join-Path $bin 'parametric.exe'),
  (Join-Path $bin 'xtop.exe')
) + @($files | Where-Object { $_.Name -match 'ui|ribbon|resource|proe|creo' } | Select-Object -ExpandProperty FullName -First 40)) {
  if (Test-Path -LiteralPath $f) {
    try { [CreoScan]::Run($f, $needles, $out) } catch { "ERR $f : $_" | Add-Content $out }
  }
}
Get-Item $out | Format-List FullName,Length
Write-Host "Wrote $out"
```

## Agent prompt (paste into a chat pinned to the FreeCADBG Windows worker)

```text
Inventory Creo 7.0.3.0 for FreeCAD Creo-inspired ribbon evidence.
Path: C:\Program Files\PTC\Creo 7.0.3.0\Parametric\bin\parametric.exe
Also open sibling xtop.exe.
Use gcad open_binary / inspect_pe_resources / search_strings if available;
else run tools/creo_evidence PowerShell dump.
Return observed Model-tab labels, Round/Chamfer family strings, file sizes,
and evidence-backed deltas for src/Gui/RibbonCatalog.cpp creoLayout() only.
Read-only. Do not invent Creo UX.
```
