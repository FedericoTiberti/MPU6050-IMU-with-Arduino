#include "math.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)
//#define readAngles() (imu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz))

MPU6050 imu;

int16_t ax,ay,az,gx,gy,gz;
float accX,accY,accZ,gyroX,gyroY,gyroZ;
int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;
float phiXa, phiYa, phiX=0, phiY=0, phiZ=0;

//time parameters
int16_t t0,t1;
float dt;

//normalization parameters
float accRes = 2.0; //defaul 2g per l'acc.
float divider = 32767.0;
float gyroRes = 250; //default 250°/s per il gyro

//complementary filter parameters
float alphaCF = 0;
float tau = 0.075;

void setup(){
  Wire.begin(); 
  Serial.begin(115200);
  imu.initialize();
  // da MPU6050_calibration.ino ottengo i seguenti offset:

  ax_offset = 828; ay_offset = -2621; az_offset = 718;
  gx_offset = 17; gy_offset = -14; gz_offset = 28;

  imu.setXAccelOffset(ax_offset);
  imu.setYAccelOffset(ay_offset);
  imu.setZAccelOffset(az_offset);
  imu.setXGyroOffset(gx_offset);
  imu.setYGyroOffset(gy_offset);
  imu.setZGyroOffset(gz_offset);
  
}

void readAngles(){
  imu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
  t1 = millis();
  dt = (t1-t0) / 1000.0; 
  t0 = t1;

  //NORMALIZE ACC
  accX = ax*accRes/divider;
  accY = ay*accRes/divider;
  accZ = az*accRes/divider;

  //NORMALIZE GYRO (velocità angolari normalizzate)
  gyroX = degToRad(gx*gyroRes/divider);
  gyroY = degToRad(gy*gyroRes/divider);
  gyroZ = degToRad(gz*gyroRes/divider);

  gyroX = (gyroX + sin(phiX)*tan(phiZ)*gyroY + cos(phiX)*tan(phiZ)*gyroZ);
  gyroY = (cos(phiX)*gyroY - sin(phiZ)*gyroZ);
  gyroZ = ((sin(phiX)/cos(phiZ))*gyroY + (cos(phiX)/cos(phiZ))*gyroZ);

  //calcolo del roll dall'acc. (phiXa)
  phiXa = atan2(cos(accY)*sin(accX), cos(accY)*cos(accX));

  //calcolo del pitch dall'acc (phiYa)
  phiYa = atan2(sin(accY), sqrt(pow(cos(accX)*sin(accX),2) + pow(cos(accY)*cos(accX),2)));
}

void compFilter(){
  alphaCF = tau/(tau+dt);

  //VEDERE LA LIBRERIA QUI: https://github.com/ElectronicCats/mpu6050/blob/master/src/MPU6050.h
  
  phiX = alphaCF * (phiX + gyroX*dt) + (1-alphaCF)*phiXa;
  phiY = alphaCF * (phiY + gyroY*dt) + (1-alphaCF)*phiYa;
  phiZ = alphaCF * (phiZ + gyroZ*dt);  //no correction because no magnetometer
}

void loop(){
  readAngles();
  compFilter();
  Serial.print("Roll: "); Serial.print(radToDeg(phiX)); Serial.print(", ");
  Serial.print("Pitch: "); Serial.print(radToDeg(phiY)); Serial.print(", ");
  Serial.print("Yaw: "); Serial.print(radToDeg(phiZ)); Serial.print(", ");
  Serial.print("Loop time: "); Serial.print(dt*1000.0); Serial.println("ms.");
}
