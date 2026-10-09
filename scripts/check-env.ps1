# Reports whether the tools needed to build are installed, and version numbers
# Run from  repository root:  powershell -ExecutionPolicy Bypass -File scripts/check-env.ps1

$msys = 'C:\msys64\ucrt64\bin'
if (Test-Path $msys) { $env:PATH = "$msys;$env:PATH" }

$os = (Get-CimInstance Win32_OperatingSystem)
"{0}  (build {1})" -f $os.Caption, $os.BuildNumber
""

$tools = 'git', 'gcc', 'g++', 'gdb', 'cmake', 'ninja', 'cppcheck'
$missing = 0

foreach ($tool in $tools) {
    if (Get-Command $tool -ErrorAction SilentlyContinue) {
        $version = (& $tool --version 2>&1 | Select-Object -First 1)
        "{0,-9} OK       {1}" -f $tool, $version
    } else {
        "{0,-9} MISSING" -f $tool
        $missing++
    }
}

""
if ($missing -eq 0) { "All tools found." } else { "$missing tool(s) missing." }
exit $missing
