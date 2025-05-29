# Download Chrome
$LocalTempDir = $env:TEMP
$ChromeInstaller = "ChromeInstaller.exe"
(new-object    System.Net.WebClient).DownloadFile('http://dl.google.com/chrome/install/375.126/chrome_installer.exe', "$LocalTempDir\$ChromeInstaller")
# Install Chrome
& "$LocalTempDir\$ChromeInstaller" /silent /install
$Process2Monitor =  "ChromeInstaller"
Do { $ProcessesFound = Get-Process | ?{$Process2Monitor -contains $_.Name} | Select-Object -ExpandProperty Name; If ($ProcessesFound) { "Still running: $($ProcessesFound -join ', ')" | Write-Host; Start-Sleep -Seconds 2 } else { rm "$LocalTempDir\$ChromeInstaller" -ErrorAction SilentlyContinue -Verbose } } Until (!$ProcessesFound)
# Make Chrome default browser
Invoke-WebRequest https://kolbi.cz/SetDefaultBrowser.zip -OutFile SDB.zip
jar xf SDB.zip
./SetDefaultBrowser/SetDefaultBrowser HKLM "Google Chrome"
