
call vars.bat

set CHIP=%1

if #%1==#ESP8266 goto ESP8266
if #%1==#ESP32 goto ESP32
goto end

:ESP32
set BIN=.pio\build\esp32dev\firmware.bin

echo Version:
%ESPTOOL% version

if #%2==#A goto ESP32writeAll
if #%2==#U goto ESP32writeUpdate
if #%2==#M goto ESP32Merge
goto end

:ESP32Merge
echo Merge:
rem %ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% merge_bin -o %BIN% 0xe000 C:\Users\owen\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.2/tools/partitions/boot_app0.bin 0x1000 EspRelay.ino.bootloader.bin 0x10000 EspRelay.ino.esp32.bin 0x8000 EspRelay.ino.partitions.bin 
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% merge_bin -o %BIN% 0xe000 %BOOTAPPPATH%boot_app0.bin 0x1000 EspRelay.ino.bootloader.bin 0x10000 EspRelay.ino.esp32.bin 0x8000 EspRelay.ino.partitions.bin 
%ESPTOOL% -c %CHIP% merge_bin -o EspRelay_esp32dev_merged.bin 0x1000 %USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\bin\bootloader_dio_40m.bin 0x8000 .pio\build\esp32dev\partitions.bin 0xe000 %USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin  0x10000 %BIN%
goto end

:ESP32WriteAll
set BIN=EspRelay.ino.esp32.merged.bin
echo Write:
@echo on
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% write_flash 0x00000 %BIN%
@echo Verify:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% verify_flash 0x00000 %BIN%
@echo off
goto end

:ESP32WriteUpdate
echo Write:
@echo on
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% write_flash 0x10000 %BIN%
@echo Verify:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% verify_flash 0x10000 %BIN%
@echo off
goto end

:ESP8266
set BIN=.pio\build\esp01_1m\firmware.bin

echo Version:
%ESPTOOL% version

echo Write:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% write_flash 0x00000 %BIN%
echo Verify:
%ESPTOOL% -c %CHIP% -b %SPEED% -p %PORT% verify_flash 0x00000 %BIN%
goto end

:end
exit /b

