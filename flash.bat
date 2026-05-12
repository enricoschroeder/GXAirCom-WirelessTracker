@echo off
echo =====================================================
echo  GXAirCom - Heltec Wireless Tracker V1.2 Flasher
echo =====================================================
echo.
echo Put the Wireless Tracker in bootloader mode:
echo   1. Hold the USER button
echo   2. Plug in USB
echo   3. Release after 2-3 seconds
echo.
pause

esptool.py --chip esp32s3 --baud 460800 ^
  --before default_reset --after hard_reset write_flash ^
  0x0000 bin\Wireless_Tracker\v1.0-beta\bootloader.bin ^
  0x8000 bin\Wireless_Tracker\v1.0-beta\partitions.bin ^
  0x10000 bin\Wireless_Tracker\v1.0-beta\firmware.bin

echo.
echo Done! Unplug and replug USB normally.
pause