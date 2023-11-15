#include "gui_sys_time.h"
#include <time.h>
#include <sys/time.h>

/**
  * @brief  设置系统时间
  * @param  时间值
  * @retval 成功返回0， 失败返回-1
  */
int8_t GUI_Set_Sys_Time(uint32_t year, uint32_t month, uint32_t day, 
                    uint32_t hour, uint32_t min, uint32_t sec)
{
    struct tm s_tm;
    s_tm.tm_year = year - 1900;
    s_tm.tm_mon = month - 1;  
    s_tm.tm_mday = day; 
    s_tm.tm_hour = hour;  
    s_tm.tm_min = min; 
    s_tm.tm_sec = sec;  
     
    struct timeval tv;
    time_t timep;
    timep = mktime(&s_tm);  
    tv.tv_sec = timep;  
    tv.tv_usec = 0;
    if(settimeofday(&tv, NULL))
        return -1;
    return 0;
}

/**
  * @brief  获取系统时间
  * @param  时间值的指针
  * @retval void
  */
void GUI_Get_Sys_Time(uint32_t* year, uint32_t* month, uint32_t* day, 
                    uint32_t* hour, uint32_t* min, uint32_t* sec)
{
    time_t timer;   
    struct tm g_tm;   
    time(&timer);   
    localtime_r(&timer, &g_tm);  

    if(year != NULL)
      *year = g_tm.tm_year+1900;
    if(month != NULL)
      *month = g_tm.tm_mon+1;
    if(day != NULL)
      *day = g_tm.tm_mday;
    if(hour != NULL)  
      *hour = g_tm.tm_hour;
    if(min != NULL)
      *min = g_tm.tm_min;
    if(sec != NULL)
      *sec = g_tm.tm_sec;
}