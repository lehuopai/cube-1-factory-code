/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4.1 DriverLib
 * wifi_driver v2.0
 * ESP32 wifi 配置驱动程序
 * ->分离wifi初始化和模式初始化，使wifi可以动态的切换模式
 * ->新增wifi打开/关闭接口
 * ->新增wifi-scan扫描接口
 * ->新增wifi-apsta(桥接)接口
 * ->新增smart config 接口
 * ->修改wifi模式设置接口，动态修改ID和密码
 * 2020/12/7
***********************************************************************************************************/
#ifndef __WIFI_DRIVER_H
#define __WIFI_DRIVER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


/*-------------AP-------------*/
#define ESP_WIFI_CHANNEL_AP         1                   //AP wifi 通道
#define MAX_STA_CONN_AP             5                   //AP 最大连接数量

/*-------------STA------------*/
#define DEFAULT_SCAN_LIST_SIZE      15                  //最大扫描个数

#define WIFI_CONNECTED_BIT          BIT0                //连接成功事件标志位
#define WIFI_FAIL_BIT               BIT1                //STA连接失败事件标志位
#define WIFI_SCAN_DONE_BIT          BIT2                //wifi扫描完成标志位
#define WIFI_ESPTOUCH_DONE_BIT      BIT3                //wifiSC配置完成

//外部文件可以通过获取该事件标志组来获取wifi连接的成功与否状态
EventGroupHandle_t                  s_wifi_event_group;        //事件标志组

typedef struct {
    char SSID[33];
    int8_t RSSI;
}wifi_scan_list_t;                    //wifi扫描返回数据结构体

/**
  * @brief  初始化wifi并打开
  * @param  void
  * @retval void
  */
void WIFI_Init();

/**
  * @brief  将ESP32 wifi 初始化成AP模式
  * @param  APSSID：softap ID
  * @param  APPASS：softap 密码，小于8位时会忽略
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Softap(const char* APSSID, const char* APPASS);

/**
  * @brief  ESP32 wifi扫描wifiap
  * @param  ap_list 指针返回扫描到的ap信息列表
  * @param  list_size 最大扫描ap数量
  * @retval 实际扫描到的ap数量
  * @note   需要查看扫描的更多信息，需要修改wifi_scan_list_t并修改代码，查看更多的信息
  * @note   这个函数会会自动打开和关闭wifi,使用前wifi最好是关闭状态
  */
uint16_t WIFI_Scan(wifi_scan_list_t *ap_list, uint8_t list_size);

/**
  * @brief  将ESP32 wifi 初始化成STA模式
  * @param  STASSID sta目标ID
  * @param  STAPASS sta目标密码
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Sta(const char* STASSID, const char* STAPASS);

/**
  * @brief  将ESP32 wifi 初始化成APSTA（桥接）
  * @param  APSSID：softap ID
  * @param  APPASS：softap 密码，小于8位时会忽略
  * @param  STASSID sta目标ID
  * @param  STAPASS sta目标密码
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Apsta(const char* APSSID, const char* APPASS, const char* STASSID, const char* STAPASS);

/**
  * @brief  ESP32开始SmartConfig
  * @param  void
  * @retval void
  * @note   默认支持ESP TOUCH模式，配网成功后wifi处于sta联网状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Smart_Config(void);

/**
  * @brief  ESP32开启wifi
  * @param  void
  * @retval void
  * @note   在模式初始化函数中已经自动开启wifi，此函数非必须调用
  */
void WIFI_On(void);

/**
  * @brief  ESP32关闭wifi
  * @param  void
  * @retval void
  * @note   如果wifista已经连接则先断开
  */
void WIFI_Off(void);

#endif /* __WIFI_DRIVER_H */