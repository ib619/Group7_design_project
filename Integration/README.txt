List of additional libraries needed for final rover:
1. INA219_WE (Download from Arduino library manager)
2. Arduino -> ESP32 API (https://github.com/espressif/arduino-esp32)
3. ESP32 -> Arduino API
4. AsyncTCP (https://github.com/me-no-dev/AsyncTCP)
5. ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)


Tools needed to configure final rover:
1. ESP32 Uploader (https://github.com/me-no-dev/arduino-esp32fs-plugin)


Known problems with rover setup:
1. Arduino will not accept sketch uploads if connected to an unpowered rover
  - Either power rover or unattach arduino to upload script
