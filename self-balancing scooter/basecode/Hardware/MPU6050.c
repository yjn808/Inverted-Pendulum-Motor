#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "MPU6050_Reg.h"
#include "Delay.h"

#define MPU6050_ADDRESS		0xD0
#define MPU6050_GYRO_CALIBRATION_DELAY_MS 10

float GX_Offset=0;
float GY_Offset=0;
float GZ_Offset=0;

static void MPU6050_CalibrateGyro(uint16_t SampleCount);
static void MPU6050_ReadRegs(uint8_t RegAddress,uint8_t *DataArray,uint8_t Count);
static void MPU6050_GetRawData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	uint8_t Data[14];
	
	MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H,Data,14);
	*AccX = (Data[0] << 8) | Data[1];
	*AccY = (Data[2] << 8) | Data[3];
	*AccZ = (Data[4] << 8) | Data[5];
	
	*GyroX = (Data[8] << 8) | Data[9];
	*GyroY = (Data[10] << 8) | Data[11];
	*GyroZ = (Data[12] << 8) | Data[13];
	
	
}

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(Data);
	MyI2C_ReceiveAck();
	MyI2C_Stop();
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	Data = MyI2C_ReceiveByte();
	MyI2C_SendAck(1);
	MyI2C_Stop();
	
	return Data;
}


void MPU6050_ReadRegs(uint8_t RegAddress,uint8_t *DataArray,uint8_t Count)
{
	uint8_t i;
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	for(i=0;i<Count;i++)
	{
		DataArray[i] = MyI2C_ReceiveByte();
		if(i<Count-1)
		{
			MyI2C_SendAck(0);
		}
		else
		{
			MyI2C_SendAck(1);
		}
	}
	
	MyI2C_Stop();
}

void MPU6050_Init(void)
{
	MyI2C_Init();
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);
	GX_Offset=0;
	GY_Offset=0;
	GZ_Offset=0;
	Delay_ms(200);
	MPU6050_CalibrateGyro(1000);
}

uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

//void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
//						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
//{
//	uint8_t DataH, DataL;
//	
//	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
//	*AccX = (DataH << 8) | DataL;
//	
//	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
//	*AccY = (DataH << 8) | DataL;
//	
//	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
//	*AccZ = (DataH << 8) | DataL;
//	
//	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
//	*GyroX = (DataH << 8) | DataL;
//	
//	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
//	*GyroY = (DataH << 8) | DataL;
//	
//	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
//	*GyroZ = (DataH << 8) | DataL;
//}

void MPU6050_CalibrateGyro(uint16_t SampleCount)
{
	uint16_t i;
	int32_t GYroSumX=0;
	int32_t GYroSumY=0;
	int32_t GYroSumZ=0;
	int16_t AccX,AccY,AccZ,GyroX,GyroY,GyroZ;
	
	if(SampleCount == 0)
	{
		return ;
	}
	
	
	for(i=0;i<SampleCount;i++)
	{
		MPU6050_GetRawData(&AccX,&AccY,&AccZ,&GyroX,&GyroY,&GyroZ);
		GYroSumX += GyroX;
		GYroSumY += GyroY;
		GYroSumZ += GyroZ;
		Delay_ms(MPU6050_GYRO_CALIBRATION_DELAY_MS);
	}
	
	GX_Offset=GYroSumX/SampleCount;
	GY_Offset=GYroSumY/SampleCount;
	GZ_Offset=GYroSumZ/SampleCount;
}


void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	int16_t RawAccX,RawAccY,RawAccZ;
	int16_t RawGyroX,RawGyroY,RawGyroZ;
	
	MPU6050_GetRawData(&RawAccX,&RawAccY,&RawAccZ,&RawGyroX,&RawGyroY,&RawGyroZ);
	*AccX = RawAccX;
	*AccY = RawAccY;
	*AccZ = RawAccZ;
	
	*GyroX = RawGyroX-GX_Offset;
	*GyroY = RawGyroY-GY_Offset;
	*GyroZ = RawGyroZ-GZ_Offset;
	
	
}
