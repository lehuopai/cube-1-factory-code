#ifndef _MPU6886_DRIVER_H
#define _MPU6886_DRIVER_H

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MPU6886_I2C_SCL_IO            22
#define MPU6886_I2C_SDA_IO            21
#define MPU6886_I2C_MASTER_NUM        I2C_NUM_0

#define MPU6886_ADDRESS           0x68 

#define MPU6886_WHOAMI            0x75
#define MPU6886_ACCEL_INTEL_CTRL  0x69
#define MPU6886_SMPLRT_DIV        0x19
#define MPU6886_INT_PIN_CFG       0x37
#define MPU6886_INT_ENABLE        0x38
#define MPU6886_ACCEL_XOUT_H      0x3B
#define MPU6886_ACCEL_XOUT_L      0x3C
#define MPU6886_ACCEL_YOUT_H      0x3D
#define MPU6886_ACCEL_YOUT_L      0x3E
#define MPU6886_ACCEL_ZOUT_H      0x3F
#define MPU6886_ACCEL_ZOUT_L      0x40

#define MPU6886_TEMP_OUT_H        0x41
#define MPU6886_TEMP_OUT_L        0x42

#define MPU6886_GYRO_XOUT_H       0x43
#define MPU6886_GYRO_XOUT_L       0x44
#define MPU6886_GYRO_YOUT_H       0x45
#define MPU6886_GYRO_YOUT_L       0x46
#define MPU6886_GYRO_ZOUT_H       0x47
#define MPU6886_GYRO_ZOUT_L       0x48

#define MPU6886_USER_CTRL         0x6A
#define MPU6886_PWR_MGMT_1        0x6B
#define MPU6886_PWR_MGMT_2        0x6C
#define MPU6886_CONFIG            0x1A
#define MPU6886_GYRO_CONFIG       0x1B
#define MPU6886_ACCEL_CONFIG      0x1C
#define MPU6886_ACCEL_CONFIG2     0x1D
#define MPU6886_FIFO_EN           0x23

#define MPU6886_FIFO_ENABLE		  0x23
#define MPU6886_FIFO_CONUTH		  0x72
#define MPU6886_FIFO_CONUTL		  0x73
#define MPU6886_FIFO_R_W		  0x74

//#define G (9.8)
#define RtA     57.324841
#define AtR    	0.0174533	
#define Gyro_Gr	0.0010653

enum Ascale {
	AFS_2G = 0,
	AFS_4G,
	AFS_8G,
	AFS_16G
};

enum Gscale {
	GFS_250DPS = 0,
	GFS_500DPS,
	GFS_1000DPS,
	GFS_2000DPS
};

extern enum Gscale Gyscale;
extern enum Ascale Acscale;

int MPU6886Init(void);
void MPU6886getAccelAdc(int16_t* ax, int16_t* ay, int16_t* az);
void MPU6886getGyroAdc(int16_t* gx, int16_t* gy, int16_t* gz);
void MPU6886getTempAdc(int16_t *t);

void MPU6886getAccelData(float* ax, float* ay, float* az);
void MPU6886getGyroData(float* gx, float* gy, float* gz);
void MPU6886getTempData(float *t);

void MPU6886SetGyroFsr(enum Gscale scale);
void MPU6886SetAccelFsr(enum Ascale scale);

void MPU6886getAhrsData(float *pitch,float *roll,float *yaw);
void MPU6886setFIFOEnable( bool );
uint8_t MPU6886ReadFIFO();
uint16_t MPU6886ReadFIFOCount();
void MPU6886ReadFIFOBuff( uint8_t *DataBuff ,uint16_t Length );

float aRes, gRes;

void MPU6886getGres();
void MPU6886getAres();

#endif /* _MPU6886_DRIVER_H */