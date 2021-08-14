# drtaint

## In active development. It's not yet for practical use.

## Build

### Windows

1. Download Visual studio 2017 build tools.
2. Open `x64 Native Tools Command Prompt for VS 2017`.
3. Launch `powershell.exe` from there
4. Then enter commands below:

```powershell
Invoke-WebRequest `
	-Uri "https://github.com/DynamoRIO/dynamorio/releases/download/cronbuild-8.0.18780/DynamoRIO-Windows-8.0.18780.zip" `
	-OutFile "dynamorio.zip"

Expand-Archive -Path .\DynamoRIO-Windows-8.0.18780.zip -DestinationPath .\dynamorio\ -Force
mv .\dynamorio\DynamoRIO-Windows-8.0.18780\* .\dynamorio\
rm .\dynamorio\DynamoRIO-Windows-8.0.18780

$DYNAMORIO_ROOT="$(pwd)\dynamorio"
$DYNAMORIO_DIR="$(pwd)\dynamorio\cmake"
$DRMEMORY_DIR="$(pwd)\dynamorio\drmemory\drmf"

git clone https://github.com/poul1x/drtaint.git
cd drtaint

mkdir build
cd build

# Run this for x64 build
# cmake ../ -G"Visual Studio 15 Win64" -DDynamoRIO_DIR="$DYNAMORIO_DIR" -DDrMemoryFramework_DIR="$DRMEMORY_DIR"
# Run this for x86 build
cmake ../ -DDynamoRIO_DIR="$DYNAMORIO_DIR" -DDrMemoryFramework_DIR="$DRMEMORY_DIR"
cmake --build . --config Release
```