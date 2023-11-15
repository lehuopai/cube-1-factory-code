#ifndef __PAJ7620U2_DRIVER_H
#define __PAJ7620U2_DRIVER_H

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PAJ7620U2_I2C_SCL_IO            22
#define PAJ7620U2_I2C_SDA_IO            21
#define PAJ7620U2_I2C_MASTER_NUM        I2C_NUM_0

#define PAJ7620U2_ADDR                  0x73

#define PAJ_REGITER_BANK_SEL            0XEF    //BANK选择寄存器
#define PAJ_BANK0                       0X00    //BANK0
#define PAJ_BANK1                       0X01    //BANK1

//BANK0 寄存器组
#define PAJ_SUSPEND_CMD                 0X03 //设置设备挂起
#define PAJ_SET_INT_FLAG1               0X41 //设置手势检测中断寄存器1
#define PAJ_SET_INT_FLAG2               0X42 //设置手势检测中断寄存器2
#define PAJ_GET_INT_FLAG1               0X43 //获取手势检测中断标志寄存器1(获取手势结果)
#define PAJ_GET_INT_FLAG2               0X44 //获取手势检测中断标志寄存器2(获取手势结果)
#define PAJ_GET_STATE                   0X45 //获取手势检测工作状态
#define PAJ_SET_HIGH_THRESHOLD          0x69 //设置滞后高阀值（仅在接近检测模式下）
#define PAJ_SET_LOW_THRESEHOLD          0X6A //设置滞后低阀值
#define PAJ_GET_APPROACH_STATE          0X6B //获取接近状态 （1：PS data>= PS threshold ,0:PS data<= Low threshold）
#define PAJ_GET_GESTURE_DATA            0X6C //获取接近数据
#define PAJ_GET_OBJECT_BRIGHTNESS       0XB0 //获取被照物体亮度（最大255）
#define PAJ_GET_OBJECT_SIZE_1           0XB1 //获取被照物体大小低八位（bit7:0）(最大900)
#define PAJ_GET_OBJECT_SIZE_2           0XB2 //获取被照物体大小高四位（bit3:0）

//BANK1 寄存器组
#define PAJ_SET_PS_GAIN                 0X44 //设置检测增益大小 (0:1x gain 1:2x gain)
#define PAJ_SET_IDLE_S1_STEP_0          0x67 //设置S1的响应因子
#define PAJ_SET_IDLE_S1_STEP_1          0x68 
#define PAJ_SET_IDLE_S2_STEP_0          0X69 //设置S2的响应因子
#define PAJ_SET_IDLE_S2_STEP_1          0X6A 
#define PAJ_SET_OP_TO_S1_STEP_0         0X6B //设置OP到S1的过度时间
#define PAJ_SET_OP_TO_S1_STEP_1         0X6C
#define PAJ_SET_S1_TO_S2_STEP_0         0X6D //设置S1到S2的过度时间
#define PAJ_SET_S1_TO_S2_STEP_1         0X6E
#define PAJ_OPERATION_ENABLE            0X72 //设置PAJ7620U2使能寄存器

//手势识别效果
#define GETBIT(x)    1<<(x)

#define GESTURE_UP                      GETBIT(0) //向上
#define GESTURE_DOWM                    GETBIT(1) //向下
#define GESTURE_LEFT                    GETBIT(2) //向左
#define GESTURE_RIGHT                   GETBIT(3) //向右
#define GESTURE_FORWARD                 GETBIT(4) //向前
#define GESTURE_BACKWARD                GETBIT(5) //向后
#define GESTURE_CLOCKWISE               GETBIT(6) //顺时针
#define GESTURE_COUNT_CLOCKWISE         GETBIT(7) //逆时针
#define GESTURE_WAVE                    GETBIT(8) //挥动

//BANK寄存器枚举
typedef enum{
	  BANK0 = 0,      //BANK0寄存器
	  BANK1 = 1,      //BANK1寄存器
}bank_e;

/**
  * @brief  PAJ7620U2初始化
  * @param  void
  * @retval 成功：0 失败：1
  */
uint8_t PAJ7620U2_init(void);

/**
  * @brief  PAJ7620U2手势测试初始化
  * @param  void
  * @retval void
  */
void PAJ7620U2_Gesture_Init(void);

/**
  * @brief  PAJ7620U2获得手势状态
  * @param  gesture_status ： 指针返回手势状态
  * @retval void
  */
void PAJ7620U2_Get_Gesture(uint16_t *gesture_status);

/**
  * @brief  PAJ7620U2接近测试初始化
  * @param  void
  * @retval void
  */
void PAJ7620U2_Approach_Init(void);

/**
  * @brief  PAJ7620U2获得接近物体数据
  * @param  obj_brightness ： 指针返回物体亮度0-255
  * @param  obj_size : 指针返回物体大小
  * @retval void
  */
void PAJ7620U2_Get_Approach(uint8_t *obj_brightness, uint16_t *obj_size);


#endif /* __PAJ7620U2_DRIVER_H */