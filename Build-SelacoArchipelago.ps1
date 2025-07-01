# PowerShell build script for Selaco with Archipelago
param(
    [ValidateSet('Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$BuildType = 'Release',
    
    [switch]$Clean,
    [switch]$RunAfterBuild
)

$ErrorActionPreference = "Stop"

Write-Host "================================================================================`n" -ForegroundColor Cyan
Write-Host "🚀 Selaco Archipelago Build Script (PowerShell)`n" -ForegroundColor Green
Write-Host "================================================================================`n" -ForegroundColor Cyan

# Check if we're in the right directory
if (-not (Test-Path "src\CMakeLists.txt")) {
    Write-Host "❌ ERROR: This script must be run from the Selaco root directory!" -ForegroundColor Red
    Write-Host "   Looking for src\CMakeLists.txt but it was not found.`n" -ForegroundColor Red
    Write-Host "   Please run this script from the directory containing:" -ForegroundColor Yellow
    Write-Host "   - src\" -ForegroundColor Yellow
    Write-Host "   - libraries\" -ForegroundColor Yellow
    Write-Host "   - CMakeLists.txt`n" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

$BuildDir = "build_archipelago"

Write-Host "📋 Build Configuration:" -ForegroundColor Cyan
Write-Host "   - Build Type: $BuildType" -ForegroundColor White
Write-Host "   - Build Directory: $BuildDir" -ForegroundColor White
Write-Host "   - Archipelago: ENABLED" -ForegroundColor Green
Write-Host "   - Clean Build: $(if ($Clean) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host ""

# Clean if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "🧹 Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    Write-Host "📁 Creating build directory: $BuildDir" -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Push-Location $BuildDir

try {
    # Configure with CMake
    Write-Host "`n⚙️  Configuring with CMake...`n" -ForegroundColor Cyan
    
    & cmake .. `
        -DENABLE_ARCHIPELAGO=ON `
        -DCMAKE_BUILD_TYPE=$BuildType `
        -Wno-dev
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed!"
    }
    
    Write-Host "`n✅ CMake configuration complete!`n" -ForegroundColor Green
    
    # Build
    Write-Host "🔨 Building Selaco with Archipelago...`n" -ForegroundColor Cyan
    
    & cmake --build . --config $BuildType --parallel
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed!"
    }
    
    Write-Host "`n✅ Build completed successfully!`n" -ForegroundColor Green
    
    # Find executable
    $exePath = $null
    if (Test-Path "$BuildType\zdoom.exe") {
        $exePath = "$BuildType\zdoom.exe"
    } elseif (Test-Path "zdoom.exe") {
        $exePath = "zdoom.exe"
    }
    
    if ($exePath) {
        Write-Host "📍 Executable location:" -ForegroundColor Cyan
        Write-Host "   $(Resolve-Path $exePath)`n" -ForegroundColor White
        
        if ($RunAfterBuild) {
            Write-Host "🎮 Starting Selaco with Archipelago..." -ForegroundColor Green
            Start-Process -FilePath $exePath
        }
    } else {
        Write-Host "⚠️  Could not locate zdoom.exe in build directory" -ForegroundColor Yellow
    }
    
} catch {
    Write-Host "`n❌ Error: $_" -ForegroundColor Red
    Pop-Location
    Read-Host "`nPress Enter to exit"
    exit 1
}

Pop-Location

Write-Host "🏁 Build script complete!" -ForegroundColor Green
if (-not $RunAfterBuild) {
    Read-Host "`nPress Enter to exit"
}