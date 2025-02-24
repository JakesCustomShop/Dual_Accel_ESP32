ESP32 Code is: main.cpp.  Upload it to the ESP32 using PlatformIO on VScode.  It can be copied to a Arduino .ini file if that is easier for you. 

The ESP32 has 2 MPU6050s connected that each send XYZ and rotational data back to the PC where it can be recored.
MPU6050 #1 has it's A0 pin connect to ground
MPU6050 #2 has it's A1 pin pulled to 3.3V via 1k resostor.  This sets the serial address.

The Python code is a PC GUI to monitor and save the data to a .csv.  Download and run the .exe in /dist folder to collect data.
