List of additional libraries needed for final rover:
1. INA219_WE (Download from Arduino library manager)
2. Arduino -> ESP32 API (https://github.com/espressif/arduino-esp32) 
3. ESP32 -> Arduino API
4. AsyncTCP (https://github.com/me-no-dev/AsyncTCP)
5. ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
6. Orientation API (drive_subsystem)
7. Drive API (drive_subsystem)


Tools needed to configure final rover:
1. ESP32 Uploader (https://github.com/me-no-dev/arduino-esp32fs-plugin)
2. nios2 (https://www.intel.com/content/altera-www/global/en_us/index/support/support-resources/knowledge-base/tools/2019/why-does-the-nios--ii-not-installed-after-full-installation-of-t.html)
  - WSL for windows (https://docs.microsoft.com/en-us/windows/wsl/install-win10)
  - dox2unix for windows (https://docs.microsoft.com/en-us/windows/wsl/install-win10)


Known problems with rover setup:
1. Arduino will not accept sketch uploads if connected to an unpowered rover
  - Either power rover or unattach arduino to upload script
2. nios2 will not work with a windows version that is too new, need to download a special patch
  - https://www.intel.com/content/altera-www/global/en_us/index/support/support-resources/knowledge-base/embedded/2020/nios2-elf-gcc-exe--error--createprocess--no-such-file-or-directo.html
3. Ubuntu installation for windows might be unable to fetch packages with sudo-apt
  - run 'sudo apt-get update' to update lookup directories
  

ESP32 Settings for Arduino IDE:
  - Requires Arduino ESP32 API to work
  - Board Name/Type: DOIT ESP32 DEVKIT V1


Instructions for flashing .elf files:
1. Install nios2 and its dependancies
2. run the following command using powershell within the file's directory:
  C:\intelFPGA_lite\20.1\nios2eds/"nios II command shell.bat" nios2-download D8M_Cam_Testing.elf -g
