
call vars.bat

set SPEED=115200

%ESPTOOL% -v
rem exit /b
del wifisettings.bin

echo %ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% read_flash 0x3fe000 128 wifisettings.bin 
%ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% read_flash 0x3fe000 0x80 wifisettings.bin

pause
