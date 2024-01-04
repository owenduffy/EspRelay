
if #%PORT%==# set PORT=COM5

echo COM: %PORT%
rem set ESPTOOL=D:\Users\owen\AppData\Local\Programs\Python\Python39\scripts\esptool.exe
set ESPTOOL=esptool.exe
echo ESPTOOL: %ESPTOOL%
set SPEED=921600
set CHIP=ESP8266

awk "BEGIN{FS=\"\\\"\"} /^^ *#define *VERSION *\".*\"/{printf(\"%%s\n\",$2)>VERSION.txt;exit}" src\main.cpp
FOR /F %%V IN (VERSION.TXT) do SET VERSION=%%V
ECHO VERSION=%VERSION%

exit /b
