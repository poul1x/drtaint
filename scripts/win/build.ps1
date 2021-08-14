[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)]
  [String]$BuildMode
)

# Load variables
. ".\scripts\win\common.ps1"

# Go to build directory
Set-Location -Path $BUILD_DIR

# Build
cmake --build $BUILD_DIR --config $BuildMode