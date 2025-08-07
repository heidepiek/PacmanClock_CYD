@echo off
echo.
echo ESP32 Flash Utility based on esptool.exe (nieuwste versie)
echo.
echo Available serial ports:
for /f "tokens=4" %%A in ('mode^|findstr "COM[0-9]*:"') do echo %%A
echo.
set /p port="Type in COM Port of the ESP32 (e.g. COM6): "
pause

REM Flash bootloader
esptool.exe --chip esp32 --port %port% --baud 921600 write_flash 0x1000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.ino.bootloader.bin

REM Flash partition table
esptool.exe --chip esp32 --port %port% --baud 921600 write_flash 0x8000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.ino.partitions.bin

REM Flash main firmware
esptool.exe --chip esp32 --port %port% --baud 921600 write_flash 0x10000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.ino.bin

REM Flash SPIFFS - aangepast adres voor 2MB SPIFFS
esptool.exe --chip esp32 --port %port% --baud 921600 write_flash 0x210000 ./bin/altered_version_ESP32_Pacman_Clock_CYD_V3_1.spiffs.bin

pause
