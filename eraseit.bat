
call vars.bat

"%ESPTOOL%" -p %COM% -b %SPEED% erase_flash

exit /b
