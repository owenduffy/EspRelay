
call vars.bat

"%ESPTOOL%" -c %CHIP% -p %COM% -b %SPEED% erase_flash

pause

exit /b
