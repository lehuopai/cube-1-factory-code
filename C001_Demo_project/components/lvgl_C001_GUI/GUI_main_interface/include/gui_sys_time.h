#ifndef _GUI_SYS_TIME_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
  * @brief  设置系统时间
  * @param  时间值
  * @retval 成功返回0， 失败返回-1
  */
int8_t GUI_Set_Sys_Time(uint32_t year, uint32_t month, uint32_t day, 
                    uint32_t hour, uint32_t min, uint32_t sec);

/**
  * @brief  获取系统时间
  * @param  时间值的指针
  * @retval void
  */
void GUI_Get_Sys_Time(uint32_t* year, uint32_t* month, uint32_t* day, 
                    uint32_t* hour, uint32_t* min, uint32_t* sec);

#endif // _GUI_SYS_TIME_H