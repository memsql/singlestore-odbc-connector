if ($env:DRIVER_TYPE -imatch "unicode")
{
    $env:TEST_SERVER=$ENV:HOST_WINDOWS_UNICODE
} else {
    $env:TEST_SERVER=$ENV:HOST_WINDOWS_ANSI
}