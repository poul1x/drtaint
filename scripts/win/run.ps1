[CmdletBinding()]
param(
	[Parameter(Mandatory = $true)]
	[String]$BuildMode,
	[String]$ClientName,
	[String]$ClientArgs = "",
	[String]$AppCommand
)

# Load variables
. ".\scripts\win\common.ps1"

# Run x86 or x64 dynamorio
if ($(cl.exe 2>&1 | Select-String -Pattern 'x64')) {
	Start-Process -FilePath "$DYNAMORIO_ROOT\bin64\drrun.exe" `
		-ArgumentList "-c", "$BUILD_DIR\$ClientName\$BuildMode\$ClientName.dll", "--", $AppCommand
}
else {
	Start-Process -FilePath "$DYNAMORIO_ROOT\bin32\drrun.exe" `
		-ArgumentList "-c", "$BUILD_DIR\$ClientName\$BuildMode\$ClientName.dll", "--", $AppCommand
}

# Start-Process -FilePath "$DYNAMORIO_ROOT\bin32\drrun.exe" -ArgumentList "-c","c:\Users\pasha\Documents\Projects\drtaint\dynamorio\samples\bin32\bbcount.dll","--","notepad.exe"