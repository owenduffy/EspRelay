
call vars.bat

set BIN=EspRelay.ino.generic.bin

%ESPTOOL% -v
rem exit /b

%ESPTOOL% -c esp8266 -b %SPEED% -p %COM% write_flash 0x00000 %BIN% 
%ESPTOOL% -c esp8266 -b %SPEED% -p %COM%verify_flash 0x00000 %BIN%
