
call vars.bat
call bootapppath.bat

set CHIP=%1

if #%1==#ESP8266 goto ESP8266
if #%1==#ESP32 goto ESP32
goto end

:ESP32
set BIN=EspRelay.ino.esp32.merged.bin

echo Version:
%ESPTOOL% version

if #%2==# goto ESP32write

rem use arduinobuilt to collect the files into current directory

echo Merge:
rem %ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% merge_bin -o %BIN% 0xe000 C:\Users\owen\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.2/tools/partitions/boot_app0.bin 0x1000 EspRelay.ino.bootloader.bin 0x10000 EspRelay.ino.esp32.bin 0x8000 EspRelay.ino.partitions.bin 
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% merge_bin -o %BIN% 0xe000 %BOOTAPPPATH%boot_app0.bin 0x1000 EspRelay.ino.bootloader.bin 0x10000 EspRelay.ino.esp32.bin 0x8000 EspRelay.ino.partitions.bin 
goto end

:ESP32write

echo Write:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% write_flash 0x00000 %BIN%
echo Verify:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% verify_flash 0x00000 %BIN%
goto end

:ESP8266
set BIN=EspRelay.ino.generic.bin

echo Version:
%ESPTOOL% version

echo Write:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% write_flash 0x00000 %BIN%
echo Verify:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% verify_flash 0x00000 %BIN%
goto end

:end
exit /b

