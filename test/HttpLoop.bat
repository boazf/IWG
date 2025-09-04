:loop
for /f "tokens=3 delims=/" %%N in ('
  curl -si http://192.168.50.240/index ^| findstr /bi "Location:"
') do set "id=%%N"
curl -i http://192.168.50.240/index/%id%
timeout /t 1 /nobreak
curl -i http://192.168.50.240/Settings
timeout /t 1 /nobreak
curl -X DELETE http://192.168.50.240/api/files/wwwroot/temp/history.htm
timeout /t 1 /nobreak
curl -i http://192.168.50.240/History
timeout /t 1 /nobreak
curl -i http://192.168.50.240/History
timeout /t 1 /nobreak
curl -i http://192.168.50.240/files
timeout /t 1 /nobreak
goto loop
