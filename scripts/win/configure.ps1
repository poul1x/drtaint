# Load variables
. ".\scripts\win\common.ps1"

# Clear build directory
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $BUILD_DIR
New-Item -Path $BUILD_DIR -ItemType "directory"

# Go to build directory
Set-Location -Path $BUILD_DIR

# Generate compile commands
cmake $PROJECT_ROOT -G"Visual Studio 15 Win64" `
	-DDynamoRIO_DIR="$DYNAMORIO_DIR" `
	-DDrMemoryFramework_DIR="$DRMEMORY_DIR"