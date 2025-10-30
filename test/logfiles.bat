@echo off
setlocal
for /f "delims=" %%L in ('curl -sX POST http://192.168.50.65/files/logs') do (
  set "line=%%L"
  setlocal EnableDelayedExpansion
  rem replace all " with spaces, so:  "name":"XYZ"  ->  name: XYZ
  set "work=!line:"= !"
  set "work=!work::= !"
  set "want="
  for %%T in (!work!) do (
    if defined want (
      if not "%%T"=="isDir" (
      set file=!file!%%T 
      ) else (
      echo !file!
      set "want=" )
    ) else if /I "%%T"=="name" (
      set "want=1"
    )
  )
  endlocal
)
endlocal
