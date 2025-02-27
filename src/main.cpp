#include <Arduino.h>


/*
02/09/2022
Jake Lehotsky
BISE Lab Engineer
lehotsky001@gannon.edu
Jake@JakesCustomShop.com

Github: https://github.com/JakesCustomShop/Dual_Accel_ESP32
Based loosly on: https://randomnerdtutorials.com/arduino-mpu-6050-accelerometer-gyroscope/

When time stamps are turned on, Serial Monitor data can be copy and pasted into a spreadsheet.
Rotation data is commented out.



*/
#include "Adafruit_MPU6050.h"
#include "Adafruit_Sensor.h"
#include <Wire.h>
Adafruit_MPU6050 mpu1;
Adafruit_MPU6050 mpu2; // Second MPU6050 instance

short T = 0;                //Serial Monitor data output period in miliseconds.  Increase T to decrease data points.
void setup() {

  //SETUP Accelorometers
  Serial.begin(19200);     //Set Baud rate to 115200
  while (!Serial)
    delay(10);


  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
if (!mpu1.begin(0x68)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Try to initialize the second MPU6050!
  if (!mpu2.begin(0x69)) { // Specify the I2C address for the second MPU6050
    Serial.println("Failed to find second MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Second MPU6050 Found!");

  mpu1.setAccelerometerRange(MPU6050_RANGE_16_G);       //Set the range of the accelorometer readings 2,4,8,16g
  Serial.print("Accelerometer range set to: ");       //1g = 9.8 m/s^2 ie the acceleration of gravity
  switch (mpu1.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu1.setGyroRange(MPU6050_RANGE_250_DEG);            //Set the range of the Gyro 250,500,1000,2000 degrees/second
  Serial.print("Gyro range set to: ");
  switch (mpu1.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu1.setFilterBandwidth(MPU6050_BAND_21_HZ);           //Set the bandwidth of the samples.  5,10,21,44,94,184,260 HZ.
  Serial.print("Filter bandwidth set to: ");            //Higher Bandwidth --> More samples per second --> more data points
  switch (mpu1.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }

  Serial.println("");
  delay(100);



  //Build table
  Serial.print("\t");
  Serial.print("Accel1_X [m/s^2]\t");
  Serial.print("Accel1_Y [m/s^2]\t");
  Serial.print("Accel1_Z [m/s^2]\t");
  Serial.print("Rotation1_X [rad/s]\t");
  Serial.print("Rotation1_Y [rad/s]\t");
  Serial.print("Rotation1_Z [rad/s]\t");
  Serial.print("Accel2_X [m/s^2]\t");
  Serial.print("Accel2_Y [m/s^2]\t");
  Serial.print("Accel2_Z [m/s^2]\t");
  Serial.print("Rotation2_X [rad/s]\t");
  Serial.print("Rotation2_Y [rad/s]\t");
  Serial.print("Rotation2_Z [rad/s]\n");

}


//---------------------------------------------------------------------//

//Initilize offest values to zero Acceleratrion due to gravity
float offset[] = {0,0,0,0,0,0,0,0,0,0,0,0}; //x,y,z
float accel[] = {0, 0, 0, 0,0,0};
float rot[] = {0, 0, 0, 0, 0, 0};    //Rotational Acceleration (degree/s)

int i = 0; //Determines first loop iteration to calibrate values.
char ch;    //Serial Read data
void loop() {
  // put your main code here, to run repeatedly:


  /* Get new sensor events with the readings from the first MPU6050 */
  sensors_event_t a, g, temp;
  mpu1.getEvent(&a, &g, &temp);
  
  /* Get new sensor events with the readings from the second MPU6050 */
  sensors_event_t a2, g2, temp2;
  mpu2.getEvent(&a2, &g2, &temp2);


  ch = Serial.read();
   
  if (ch == 'p') {
    Serial.println("Data Collection Paused.  Press send 's' to continue");
    while(ch!='s'){
      delay(100);
      ch = Serial.read();  
    }
  }


  if (ch == 'r' || i == 0) {
    //Serial.println("Reseting acceleration offset values.");
    //Serial.println("Do not move Accelerometer.");
    //Serial.println("\n");
    delay (500);
    offset[0] = a.acceleration.x;
    offset[1] = a.acceleration.y;
    offset[2] = a.acceleration.z;
    offset[3] = a2.acceleration.x;
    offset[4] = a2.acceleration.y;
    offset[5] = a2.acceleration.z;

    offset[6] = g.gyro.x*180/3.14159;           //Set the offsets for the gyro.
    offset[7] = g.gyro.y*180/3.14159;
    offset[8] = g.gyro.z*180/3.14159;
    offset[9] = g2.gyro.x*180/3.14159;           //Set the offsets for the gyro.
    offset[10] = g2.gyro.y*180/3.14159;
    offset[11] = g2.gyro.z*180/3.14159;

    //Check if accelerometer is steady
    delay (1000);
    if ((a.acceleration.x - offset[0]) < 0.1) {
      Serial.println("");
      Serial.println("Succesfully calibrated acceleromter offeset values!");
      Serial.println("\n");
      i = 1; //Ensure we don't reset the acceleration unless necessary
    }
    else
      Serial.println("Offset calibration failed.  Please try again.");
    Serial.println("\n");
    delay(5000);

  }

  accel[0] = a.acceleration.x - offset[0];
  accel[1] = a.acceleration.y - offset[1];
  accel[2] = a.acceleration.z - offset[2];
  accel[3] = a2.acceleration.x - offset[3];
  accel[4] = a2.acceleration.y - offset[4];
  accel[5] = a2.acceleration.z - offset[5];


  // rot[0] = g.gyro.x*180/3.14159 - offset[6];
  // rot[1] = g.gyro.y*180/3.14159 - offset[7];
  // rot[2] = g.gyro.z*180/3.14159 - offset[8];
  // rot[3] = g2.gyro.x*180/3.14159 - offset[9];
  // rot[4] = g2.gyro.y*180/3.14159 - offset[10];
  // rot[5] = g2.gyro.z*180/3.14159 - offset[11];

  /* Print out the values from the first MPU6050 */
  Serial.print("\t");
  Serial.print(accel[0]);
  Serial.print("\t");
  Serial.print(accel[1]);
  Serial.print("\t");
  Serial.print(accel[2]);
  Serial.print("\t");

  // Serial.print(rot[0]);
  // Serial.print("\t");
  // Serial.print(rot[1]);
  // Serial.print("\t");
  // Serial.print(rot[2]);
  // Serial.print("\t");

  Serial.print("\t\t\t");   //Placholder for not using rotational values.  Removing the serial prints speeds things up

  /* Print out the values from the second MPU6050 */
  Serial.print(accel[3]);
  Serial.print("\t");
  Serial.print(accel[4]);
  Serial.print("\t");
  Serial.print(accel[5]);


  // Serial.print("\t");
  // Serial.print(rot[3]);
  // Serial.print("\t");
  // Serial.print(rot[4]);
  // Serial.print("\t");
  // Serial.print(rot[5]);
  Serial.print("\t\t\t");   //Placholder for not using rotational values.  Removing the serial prints speeds things up


  Serial.print("\n");

  delay(T);


}