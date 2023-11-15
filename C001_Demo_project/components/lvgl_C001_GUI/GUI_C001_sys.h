#ifndef _GUI_C001_SYS_H
#define _GUI_C001_SYS_H
#include "../lvgl/lvgl/lvgl.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH        320                 //屏幕宽度
#define SCREEN_HEIGHT       240                 //屏幕高度

#define SD_CARD_PATH        "/SD"               //文件夹根目录名称
#define SD_LIST_MAX_FILE    64                  //文件夹列表一次性最多支持显示的文件数量
#define SD_DIR_MAX_LEVEL    8                   //文件夹目录最多递归层数

#define WIFI_AP_SCAN_MAX_NUM    15              //wifi-scan最大搜索数

extern bool sys_wifi_ap_open_flag;              //wifi-ap开关标志
extern char* sys_wifi_ap_ssid;                  //wifi-apSSID
extern char* sys_wifi_ap_psss;                  //wifi-ap密码

extern bool sys_wifi_sta_open_flag;             //wifi-sta开关标志
extern char *sys_wifi_sta_ssid;                 //wifi-staSSID
extern char *sys_wifi_sta_psss;                 //wifi-sta密码


typedef uint8_t interface_level_t;              //界面级别

lv_style_t sys_theme_style;


enum {
    Light_Theme = 0,
    Dark_Theme = 1,
};
typedef uint8_t Sys_Theme_Num;                  //主题编号

extern Sys_Theme_Num Current_Theme;             //当前主题标志位
extern uint8_t Cullent_Backlight;               //当前背光亮度

#endif // !_GUI_C001_SYS_H