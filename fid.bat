
call vars.bat

echo %ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% flash_id
%ESPTOOL% -c %CHIP% -b %SPEED% -p %COM% flash_id

pause

exit /b
