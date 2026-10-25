#Requires -Version 5.1
<#
.SYNOPSIS
  Read-only Creo 7.0.3.0 PE string/resource inventory for FreeCAD creoLayout evidence.

.DESCRIPTION
  Lists parametric.exe / xtop.exe / sibling PE sizes, then scans ASCII + UTF-16LE
  for ribbon label needles. Writes machine-readable JSON. Does not modify Creo files.
#>
[CmdletBinding()]
param(
    [string]$BinDir = 'C:\Program Files\PTC\Creo 7.0.3.0\Parametric\bin',
    [string]$OutDir = ''
)

$ErrorActionPreference = 'Stop'

if (-not $OutDir) {
    $repoRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
    $OutDir = Join-Path $repoRoot 'tools\creo_evidence'
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$needles = @(
    'Get Data', 'Datums', 'Shapes', 'Round', 'Chamfer', 'Model', 'Annotate',
    'Extrude', 'Revolve', 'Sweep', 'Blend', 'Mirror', 'Editing', 'Operations',
    'Full Round', 'Analysis', 'Measure', 'Sketch', 'Axis', 'Plane', 'Point'
)

function Get-PeKind {
    param([string]$Path)
    try {
        $fs = [System.IO.File]::Open($Path, 'Open', 'Read', 'ReadWrite')
        try {
            $br = New-Object System.IO.BinaryReader($fs)
            if ($br.ReadUInt16() -ne 0x5A4D) { return 'not_mz' }
            $fs.Seek(0x3C, 'Begin') | Out-Null
            $peOff = $br.ReadInt32()
            $fs.Seek($peOff, 'Begin') | Out-Null
            if ($br.ReadUInt32() -ne 0x00004550) { return 'mz_not_pe' }
            $machine = $br.ReadUInt16()
            $fs.Seek($peOff + 0x18, 'Begin') | Out-Null
            $magic = $br.ReadUInt16()
            $arch = switch ($machine) {
                0x014c { 'i386' }
                0x8664 { 'amd64' }
                default { ('machine_0x{0:X4}' -f $machine) }
            }
            $peKind = switch ($magic) {
                0x10b { 'PE32' }
                0x20b { 'PE32+' }
                default { ('opt_0x{0:X4}' -f $magic) }
            }
            return "$peKind/$arch"
        }
        finally { $fs.Close() }
    }
    catch {
        return "error:$($_.Exception.Message)"
    }
}

function Find-NeedlesInFile {
    param([string]$Path, [string[]]$Needles, [long]$MaxBytes = 256MB)
    $info = Get-Item -LiteralPath $Path
    if ($info.Length -gt $MaxBytes) {
        return @{ skipped = $true; reason = "file_larger_than_${MaxBytes}"; hits = @() }
    }
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    $ascii = [System.Text.Encoding]::ASCII.GetString($bytes)
    $utf16 = [System.Text.Encoding]::Unicode.GetString($bytes)
    $hits = @()
    foreach ($n in $Needles) {
        $a = $ascii.IndexOf($n, [StringComparison]::Ordinal)
        $u = $utf16.IndexOf($n, [StringComparison]::Ordinal)
        if ($a -ge 0 -or $u -ge 0) {
            $hits += [ordered]@{
                needle     = $n
                ascii      = ($a -ge 0)
                utf16le    = ($u -ge 0)
                ascii_index = $(if ($a -ge 0) { $a } else { $null })
                utf16_char_index = $(if ($u -ge 0) { $u } else { $null })
            }
        }
    }
    return @{ skipped = $false; reason = $null; hits = $hits }
}

$parametric = Join-Path $BinDir 'parametric.exe'
$xtop = Join-Path $BinDir 'xtop.exe'

$filesExamined = @()
$binListing = @()

$binExists = Test-Path -LiteralPath $BinDir
if ($binExists) {
    $binListing = @(
        Get-ChildItem -LiteralPath $BinDir -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Extension -match '^\.(exe|dll)$' } |
            Sort-Object Name |
            ForEach-Object {
                [ordered]@{
                    full_path = $_.FullName
                    name      = $_.Name
                    size_bytes = $_.Length
                    last_write_utc = $_.LastWriteTimeUtc.ToString('o')
                    pe_kind   = (Get-PeKind -Path $_.FullName)
                }
            }
    )
}

foreach ($p in @($parametric, $xtop)) {
    if (Test-Path -LiteralPath $p) {
        $item = Get-Item -LiteralPath $p
        $filesExamined += [ordered]@{
            full_path  = $item.FullName
            size_bytes = $item.Length
            pe_kind    = (Get-PeKind -Path $item.FullName)
            exists     = $true
            role       = $(if ($item.Name -eq 'parametric.exe') { 'launcher_candidate' } else { 'ui_candidate' })
        }
    }
    else {
        $filesExamined += [ordered]@{
            full_path  = $p
            size_bytes = $null
            pe_kind    = $null
            exists     = $false
            role       = $(if ($p -like '*parametric.exe') { 'launcher_candidate' } else { 'ui_candidate' })
        }
    }
}

# Prefer scanning parametric, xtop, then UI/resource-named PEs (cap count)
$scanTargets = New-Object System.Collections.Generic.List[string]
foreach ($p in @($parametric, $xtop)) {
    if (Test-Path -LiteralPath $p) { [void]$scanTargets.Add($p) }
}
foreach ($f in $binListing) {
    if ($f.name -match '(?i)ui|ribbon|resource|proe|creo|menu|cmd|uif') {
        if (-not $scanTargets.Contains($f.full_path)) {
            [void]$scanTargets.Add($f.full_path)
        }
    }
}
# Always include first N largest DLLs if still thin — UI often lives in large modules
$extra = $binListing |
    Where-Object { $_.name -match '\.dll$' } |
    Sort-Object size_bytes -Descending |
    Select-Object -First 25
foreach ($f in $extra) {
    if (-not $scanTargets.Contains($f.full_path)) {
        [void]$scanTargets.Add($f.full_path)
    }
}

$stringHits = @()
foreach ($path in $scanTargets) {
    Write-Host "Scanning $path"
    $result = Find-NeedlesInFile -Path $path -Needles $needles
    $stringHits += [ordered]@{
        full_path = $path
        skipped   = $result.skipped
        reason    = $result.reason
        hits      = $result.hits
    }
}

$modelNeedles = @('Get Data', 'Datums', 'Shapes', 'Model', 'Extrude', 'Revolve', 'Sweep', 'Blend', 'Mirror', 'Annotate')
$roundNeedles = @('Round', 'Chamfer', 'Full Round')
$navNeedles = @('Ctrl+', 'Alt+', 'Shift+', 'Accelerator', 'Shortcut')

function Select-Observed {
    param($Hits, [string[]]$Filter)
    $set = New-Object 'System.Collections.Generic.HashSet[string]'
    foreach ($fileHit in $Hits) {
        foreach ($h in $fileHit.hits) {
            if ($Filter -contains $h.needle) { [void]$set.Add($h.needle) }
        }
    }
    return @($set)
}

# Secondary nav scan only on parametric/xtop (smaller)
$navHits = @()
foreach ($path in @($parametric, $xtop)) {
    if (-not (Test-Path -LiteralPath $path)) { continue }
    $r = Find-NeedlesInFile -Path $path -Needles $navNeedles
    foreach ($h in $r.hits) { $navHits += $h.needle }
}

$paramSize = ($filesExamined | Where-Object { $_.full_path -eq $parametric -and $_.exists } | Select-Object -First 1).size_bytes
$isSmallLauncher = $null
if ($null -ne $paramSize) {
    $isSmallLauncher = ($paramSize -lt 2MB)
}

$report = [ordered]@{
    schema_version = 1
    inventory_id   = 'creo-7.0.3.0-ribbon-ux'
    generated_at_utc = (Get-Date).ToUniversalTime().ToString('o')
    status         = $(if ($binExists) { 'ok' } else { 'bin_dir_missing' })
    host           = $env:COMPUTERNAME
    bin_dir        = $BinDir
    bin_dir_exists = $binExists
    files_examined = $filesExamined
    parametric_exists = (Test-Path -LiteralPath $parametric)
    xtop_exists    = (Test-Path -LiteralPath $xtop)
    parametric_is_small_launcher = $isSmallLauncher
    bin_dir_exe_dll_listing = $binListing
    string_scan_targets = @($scanTargets)
    string_hits_by_file = $stringHits
    observed_model_tab_group_command_labels = @(Select-Observed -Hits $stringHits -Filter $modelNeedles)
    observed_round_chamfer_family_strings = @(Select-Observed -Hits $stringHits -Filter $roundNeedles)
    observed_nav_accelerator_strings = @($navHits | Select-Object -Unique)
    gcad = @{
        status = 'not_used_in_this_script'
        notes  = 'Prefer gcad open_binary/inspect_pe_resources/search_strings when available on this host.'
    }
    recommended_RibbonCatalog_creoLayout_deltas = @()
    recommended_RibbonCatalog_creoLayout_deltas_note = 'Fill only after human/agent maps exact observed strings to FreeCAD commands.'
}

$jsonPath = Join-Path $OutDir 'creo_string_dump.json'
$txtPath = Join-Path $OutDir 'creo_string_dump.txt'
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $jsonPath -Encoding UTF8

@(
    "Creo ribbon string dump @ $(Get-Date -Format o)"
    "Host: $env:COMPUTERNAME"
    "Bin: $BinDir (exists=$binExists)"
    "parametric.exe exists=$($report.parametric_exists) size=$paramSize small_launcher=$isSmallLauncher"
    "xtop.exe exists=$($report.xtop_exists)"
    ''
    '=== FILES (exe/dll) ==='
    ($binListing | ForEach-Object { '{0,12}  {1,-40}  {2}' -f $_.size_bytes, $_.name, $_.pe_kind })
    ''
    '=== MODEL/GROUP NEEDLE HITS ==='
    ($report.observed_model_tab_group_command_labels -join ', ')
    '=== ROUND/CHAMFER ==='
    ($report.observed_round_chamfer_family_strings -join ', ')
    '=== NAV/ACCEL (parametric/xtop only) ==='
    ($report.observed_nav_accelerator_strings -join ', ')
    ''
    '=== PER-FILE HITS ==='
) | Set-Content -LiteralPath $txtPath -Encoding UTF8

foreach ($fileHit in $stringHits) {
    "--- $($fileHit.full_path) skipped=$($fileHit.skipped) $($fileHit.reason) ---" | Add-Content $txtPath
    foreach ($h in $fileHit.hits) {
        "  $($h.needle)  ascii=$($h.ascii) utf16le=$($h.utf16le)" | Add-Content $txtPath
    }
}

Write-Host "Wrote $jsonPath"
Write-Host "Wrote $txtPath"
Get-Item -LiteralPath $jsonPath, $txtPath | Format-List FullName, Length
