set-location -path HKLM:\SYSTEM\CurrentControlSet\Enum\FTDIBUS


$running =  TRUE


<# Device loop #>
while($running){

}

$files = get-childitem
$constNameLatencyTimer='LatencyTimer'

<# example
Set-Itemproperty -path 'VID_0403+PID_6001+AXZ001MIA\0000\Device Parameters' -Name LatencyTimer -value '0x00000001'
#>

Start-Sleep -Seconds 1.5
$latencytimerEntry = ''
