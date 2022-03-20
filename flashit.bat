
call vars.bat

set BIN=EspRelay.ino.generic.bin

%ESPTOOL% -v
rem exit /b

%ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% write_flash 0x00000 %BIN% 
%ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% verify_flash 0x00000 %BIN%

pause
