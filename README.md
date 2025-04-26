Files to remote control a motor to open and close access to a dog door

Boards managers:
was: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
changed to: https://dl.espressif.com/dl/package_esp32_index.json

Tools / Board manager / search for 'esp32' and make sure ESP32 by Expressif is installed

Tools / Boards / esp32 / UM TinyPICO

Still failing:
Waiting for gdb server to start...[2024-12-31T04:58:21.839Z] SERVER CONSOLE DEBUG: onBackendConnect: gdb-server session connected. You can switch to "DEBUG CONSOLE" to see GDB interactions.
"C:\\Users\\clare\\AppData\\Local\\Arduino15\\packages\\esp32\\tools\\openocd-esp32\\v0.12.0-esp32-20241016/bin/openocd" -c "gdb_port 50000" -c "tcl_port 50001" -c "telnet_port 50002" -s "c:\\Users\\clare\\OneDrive\\Documents\\DogDoor_Arduino\\SimpleWebServer\\SimpleWebServer" -f "C:/Program Files/Arduino IDE/resources/app/plugins/cortex-debug/extension/support/openocd-helpers.tcl" -f board/esp32-wrover-kit-3.3v.cfg
Open On-Chip Debugger v0.12.0-esp32-20241016 (2024-10-16-14:17)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
DEPRECATED! use 'gdb port', not 'gdb_port'
DEPRECATED! use 'tcl port' not 'tcl_port'
CDRTOSConfigure
Info : Listening on port 50001 for tcl connections
Info : Listening on port 50002 for telnet connections
Error: unable to open ftdi device with description '*', serial '*' at bus location '*'

C:/Users/clare/AppData/Local/Arduino15/packages/esp32/tools/openocd-esp32/v0.12.0-esp32-20241016/bin/../share/openocd/scripts/target/esp_common.cfg:9: 
Error: at file "C:/Users/clare/AppData/Local/Arduino15/packages/esp32/tools/openocd-esp32/v0.12.0-esp32-20241016/bin/../share/openocd/scripts/target/esp_common.cfg", line 9

[2024-12-31T04:58:22.088Z] SERVER CONSOLE DEBUG: onBackendConnect: gdb-server session closed
GDB server session ended. This terminal will be reused, waiting for next session to start...
