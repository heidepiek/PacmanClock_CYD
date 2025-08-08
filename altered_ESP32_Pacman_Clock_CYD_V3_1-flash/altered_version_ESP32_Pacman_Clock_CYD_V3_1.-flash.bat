@echo off
echo.
echo ESP32 Flash Utility using globally installed esptool
echo.

REM Check if esptool is available
where esptool >nul 2>&1
if errorlevel 1 (
  echo FOUT: esptool is niet gevonden. Installeer het met:
  echo     pip install esptool
  echo Of zorg dat esptool.exe in PATH staat.
  pause
  exit /b
)

echo Beschikbare seriële poorten:
for /f "tokens=4" %%A in ('mode^|findstr "COM[0-9]*:"') do echo %%A
echo.
set /p port="Typ de COM-poort van de ESP32 (bijv. COM6): "
pause

REM Flash bootloader
esptool --chip esp32 --port %port% --baud 921600 write_flash 0x1000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.ino.bootloader.bin

REM Flash partition table
esptool --chip esp32 --port %port% --baud 921600 write_flash 0x8000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.ino.partitions.bin

REM Flash main firmware
esptool --chip esp32 --port %port% --baud 921600 write_flash 0x10000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.ino.bin

REM Flash SPIFFS
esptool --chip esp32 --port %port% --baud 921600 write_flash 0x210000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.spiffs.bin

pause