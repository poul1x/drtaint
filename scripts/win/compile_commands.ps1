# Load variables
. ".\scripts\win\common.ps1"

# Clear build directory
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $BUILD_DIR
New-Item -Path $BUILD_DIR -ItemType "directory"

# Go to build directory
Set-Location -Path $BUILD_DIR

# Generate compile commands
cmake.exe $PROJECT_ROOT -G"Ninja" `
	-DDynamoRIO_DIR="$DYNAMORIO_DIR" `
	-DDrMemoryFramework_DIR="$DRMEMORY_DIR" `
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Move compile commands to project root
Move-Item -Path "compile_commands.json" -Destination $PROJECT_ROOT -Force