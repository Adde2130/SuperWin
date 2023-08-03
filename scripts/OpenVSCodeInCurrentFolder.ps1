<# Script for opening code in the focused directory #>
param (
    [int]$windowHandle
)

# Get the current folder from the specified File Explorer window
$shell = New-Object -ComObject Shell.Application
$folderPath = $null
$shell.Windows() | ForEach-Object {
    if ($_.HWND -eq $windowHandle) {
        $folderPath = $_.Document.Folder.Self.Path
    }
}

# Check if the folder path was found
if ($folderPath) {
    # Open VS Code in the folder path
    Start-Process code $folderPath
} 