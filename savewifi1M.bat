
call vars.bat

set SPEED=115200

%ESPTOOL% -v
rem exit /b
del wifisettings.bin

echo %ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% read_flash 0xfe000 128 wifisettings.bin 
%ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% read_flash 0xfe000 0x80 wifisettings.bin

pause
