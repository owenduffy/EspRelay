
call vars.bat

mklittlefs -d 2 -c data -b 4096 -p 256 -s 0x10000 littlefs.bin

rem exit /b

rem 1MB (FS:64K OTA:~407KB)
"%ESPTOOL%" -c %CHIP% -p %COM% -b %SPEED% write_flash 0x0eb000 littlefs.bin
rem ??? "%ESPTOOL%" -p %COM% -b 921600 write_flash 0x200000 spiffs.bin

pause

exit /b


ESP01S:

[LittleFS] data    : D:\src\ESP8266-Relay01\data
[LittleFS] size    : 64
[LittleFS] page    : 256
[LittleFS] block   : 4096
/config.cfg
[LittleFS] upload  : C:\Users\owen\AppData\Local\Temp\arduino_build_419015/ESP8266-Relay01.mklittlefs.bin
[LittleFS] address : 0xEB000
[LittleFS] reset   : --before default_reset --after hard_reset
[LittleFS] port    : COM8
[LittleFS] speed   : 115200
[LittleFS] python   : C:\Users\owen\AppData\Local\Arduino15\packages\esp8266\tools\python3\3.7.2-post1\python3.exe
[LittleFS] uploader : C:\Users\owen\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.7.4\tools\upload.py

esptool.py v2.8
Serial port COM8
Connecting....
Chip is ESP8266EX
Features: WiFi
Crystal is 26MHz
MAC: a4:cf:12:b9:80:27
Uploading stub...
Running stub...
Stub running...
Configuring flash size...
Auto-detected Flash size: 1MB
Compressed 65536 bytes to 405...
Wrote 65536 bytes (405 compressed) at 0x000eb000 in 0.0 seconds (effective 11130.2 kbit/s)...
Hash of data verified.
