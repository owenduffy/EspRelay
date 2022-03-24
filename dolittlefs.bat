
if #%1==#ESP8266 goto ESP8266
if #%1==#ESP32 goto ESP32
goto end

call vars.bat
set CHIP=%1

rem 10000 170000  Writing at 0x00290000
:ESP8266
echo Processing ESP8266
set FSSIZE=0x10000
set FSLOC=0x0eb000
goto makeit

:ESP32
echo Processing ESP32
set FSSIZE=0x170000
set FSLOC=0x029000
goto makeit

:makeit
mklittlefs -d 2 -c data -b 4096 -p 256 -s %FSSIZE% littlefs.%1.bin

rem exit /b
rem goto end

"%ESPTOOL%" -c %CHIP% -p %COM% -b %SPEED% write_flash %FSLOC% littlefs.%1.bin

:end

exit /b
