
call vars.bat
set SPEED=115200

if #%1==#ESP8266 goto ESP8266
if #%1==#ESP32 goto ESP32
goto end

:ESP8266
echo Processing ESP8266
set WIFILOC=0xfe000
goto doit

:ESP32
echo Does not work on ESP32
goto end

:doit
%ESPTOOL% -v
rem exit /b
del wifisettings.bin

echo %ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% read_flash %WIFILOC% 128 wifisettings.%1.bin 
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% read_flash %WIFILOC% 0x80 wifisettings.%1.bin

:end
exit /b
