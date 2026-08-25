# ************************************************************************************
#   Copyright (c) 2026 SingleStore, Inc.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public
#   License along with this library; if not see <http://www.gnu.org/licenses>
#   or write to the Free Software Foundation, Inc.,
#   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
# *************************************************************************************/

# Signs Windows artifacts with Azure Artifact Signing (Trusted Signing).
# Expects an Azure login (see the Publish installers workflow) and the .NET Sign CLI
# to be available.
param(
    [Parameter(Mandatory = $true)]
    [string[]]$Files
)

$ErrorActionPreference = "Stop"

foreach ($name in @("AZURE_SIGNING_ENDPOINT", "AZURE_SIGNING_ACCOUNT", "AZURE_SIGNING_PROFILE"))
{
    if (-not (Get-Item "env:$name" -ErrorAction SilentlyContinue).Value)
    {
        throw "$name must be set to sign Windows artifacts"
    }
}

# 'dotnet tool install -g' does not always update PATH of the running session
$signTool = (Get-Command sign -ErrorAction SilentlyContinue).Source
if (-not $signTool)
{
    $signTool = Join-Path $env:USERPROFILE ".dotnet\tools\sign.exe"
}
if (-not (Test-Path $signTool))
{
    throw "Sign CLI not found, install it with 'dotnet tool install -g --prerelease sign'"
}

$missing = $Files | Where-Object { -not (Test-Path $_) }
if ($missing)
{
    throw "Cannot sign files that do not exist: $($missing -join ', ')"
}

Write-Host "Signing with Azure Artifact Signing:"
$Files | ForEach-Object { Write-Host "  $_" }

& $signTool code artifact-signing `
    --artifact-signing-endpoint "$ENV:AZURE_SIGNING_ENDPOINT" `
    --artifact-signing-account "$ENV:AZURE_SIGNING_ACCOUNT" `
    --artifact-signing-certificate-profile "$ENV:AZURE_SIGNING_PROFILE" `
    --timestamp-url "http://timestamp.acs.microsoft.com" `
    @Files

if ($LASTEXITCODE -ne 0)
{
    throw "Signing failed with exit code $LASTEXITCODE"
}
