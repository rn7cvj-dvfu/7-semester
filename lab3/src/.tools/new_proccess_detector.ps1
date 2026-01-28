$previous = Get-Process
while ($true) {
    Start-Sleep -Seconds 0.1
    $current = Get-Process
    $new = Compare-Object -ReferenceObject $previous -DifferenceObject $current -Property Id, ProcessName | Where-Object {$_.SideIndicator -eq '=>'}
    if ($new) {
        $new | ForEach-Object { Write-Host "NEW: PID=$($_.Id) Name=$($_.ProcessName)" -ForegroundColor Green }
    }
    $previous = $current
}