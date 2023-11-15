#include "dht12_driver.h"

/**
  * @brief  DHT12采集一次数据
  * @param  Temprature:指针返回温度数据
  * @param  Humi:指针返回湿度数据
  * @retval 校验成功返回0，失败返回1
  */
uint8_t DHT12_Get_Val(float *Temprature, float *Humi)
{
    uint8_t data[5] = {0};
    uint8_t temp = 0;
    uint8_t ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DHT12_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, DHT12_BEGIN_MEASURE, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (DHT12_ADDR << 1)|READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data, 5, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(DHT12_I2C_MASTER_NUM, cmd, 1000/portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // I2c_Check_Err(ret);

    temp = (uint8_t)(data[0]+data[1]+data[2]+data[3]);       //只取低8位
    if(data[4] == temp) {   //如果校验成功，往下运行
        *Humi=data[0]*10+data[1]; //湿度

        if(data[3]&0X80){	//为负温度
            *Temprature =0-(data[2]*10+((data[3]&0x7F)));
        }else{              //为正温度
            *Temprature = data[2]*10+data[3];
        } 	
        //判断温湿度是否超过测量范围（温度：-20℃~60摄氏度；湿度：20%RH~95%RH）		
        if(*Humi>950){
            *Humi = 950;
        }
        if(*Humi<200){
            *Humi = 200;
        }
        if(*Temprature>600){
            *Temprature = 600;
        }
        if(*Temprature<-200){
            *Temprature = -200;
        }
        *Temprature = *Temprature/10;   //计算为温度值
        *Humi = *Humi/10;               //计算为湿度值	   
    }else{                  //校验失败
        printf("CRC Error !!\r\n");
        *Temprature = 0;
        *Humi = 0;
        return 1;
    }
    return 0;
}