:loop
for /f "tokens=3 delims=/" %%N in ('
  curl -si http://internetrecoverydev.local/index ^| findstr /bi "Location:"
') do set "id=%%N"
curl -i http://internetrecoverydev.local/index/%id%
timeout /t 1 /nobreak
curl -i http://internetrecoverydev.local/Settings
timeout /t 1 /nobreak
curl -X DELETE http://internetrecoverydev.local/api/files/wwwroot/temp/history.htm
timeout /t 1 /nobreak
curl -i http://internetrecoverydev.local/History
timeout /t 1 /nobreak
curl -i http://internetrecoverydev.local/History
timeout /t 1 /nobreak
curl -i http://internetrecoverydev.local/files
timeout /t 1 /nobreak
goto loop
