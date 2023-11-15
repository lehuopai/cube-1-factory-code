#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GUI_C001_sys.h"
#include "gui_main_interface.h"
#include "gui_sys_time.h"
#include "gui_setting_interface.h"
#include "gui_folder_interface.h"
#include "gui_rgbled_interface.h"
#include "gui_calendar_interface.h"
#include "gui_gyroscop_interface.h"
#include "gui_i2cdevice_interface.h"

#include "sk6812_driver.h"
#include "ip5306_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../lvgl_esp32_drivers/lvgl_touch/ft6x36.h"


//图片资源声明
LV_IMG_DECLARE(setting_icon);
LV_IMG_DECLARE(folder_icon);
// LV_IMG_DECLARE(alarm_clock_icon);
LV_IMG_DECLARE(calendar_icon);
LV_IMG_DECLARE(gyroscope_icon);
LV_IMG_DECLARE(sun_icon);
LV_IMG_DECLARE(moon_icon);
LV_IMG_DECLARE(touch_icon);
LV_IMG_DECLARE(led_icon);
LV_IMG_DECLARE(i2c_icon);

//字体资源声明
LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(BAUHS93_60_font);

enum {
    MAIN_TABVIEW_INTER_LEVEL = 0,
    USE_MESSAG_INTER_LEVEL = 1,
};

static interface_level_t interface_level = MAIN_TABVIEW_INTER_LEVEL;


static lv_style_t state_bar_style;                 //状态栏风格
static lv_style_t dialPlate_style;                 //表盘风格
static lv_style_t digital_time_style;              //表盘时间风格
static lv_style_t digital_date_style;              //表盘日期风格
static lv_style_t app_btn_style;                   //软件按钮风格

static lv_obj_t* gui_main_obj;         //主界面对象

static lv_obj_t* state_bar;                //状态栏对象(SB)

static lv_obj_t* SB_function_label;            //状态栏功能标签
static lv_obj_t* SB_battery_label;             //状态栏电池标签

static lv_obj_t* menu_tabview;             //主界面选项卡(MT)

static lv_obj_t* MT_main_page;                 //主菜单界面选项卡页面(MT_MP)

static lv_obj_t* MT_MP_dialPlate_cnt;              //主菜单界面表盘应用按键(MMP_MP_DB)
static lv_obj_t* MT_MP_setting_imgbtn;             //主菜单界面设置应用图片按键
static lv_obj_t* MT_MP_folder_imgbtn;              //主菜单界面文件应用图片按键
static lv_obj_t* MT_MP_theme_imgbtn;               //主菜单界面主题应用图片按键

static lv_obj_t* MT_MP_DB_time_label;                  //主菜单表盘时间标签
static lv_obj_t* MT_MP_DB_date_label;                  //主菜单表盘日期标签

static lv_obj_t* MT_software_page;             //软件菜单界面选项卡页面(MT_SP)

static lv_obj_t* MT_SP_RGBled_imgbtn;              //软件菜单界面RGBled应用图片按键
// static lv_obj_t* MT_SP_alarmClock_imgbtn;          //软件菜单界面闹钟应用图片按键
static lv_obj_t* MT_SP_calendar_imgbtn;            //软件菜单界面日历应用图片按键
static lv_obj_t* MT_SP_gyroscope_imgbtn;           //软件菜单界面陀螺仪应用图片按键

static lv_obj_t* MT_module_page;               //模块菜单界面选项卡页面(MT_OP)

static lv_obj_t* MT_OP_I2Cdevice_imgbtn;            //模块菜单i2c设备图片按钮


static lv_style_t use_message_style;

static lv_obj_t* msgbox_back;                   //消息盒子背景
static lv_obj_t* use_message_msgbox;            //使用信息消息盒子


char state_bar_function[256] = LV_SYMBOL_HOME;

static char digital_time[9];
static char digital_date[25];

static const char main_menu_msg_info[] = "main menu\n  This is the beginning of the whole interface, and from any interface you can just press the touch button in the middle to get there.";
static const char software_menu_msg_info[] = "software menu\n  It only needs functions that can be used by the host, no modules need to be attached.";
static const char module_menu_msg_info[] = "module menu\n  Functions that can be used after the module is specified are placed under this menu.";
static const char setting_msg_info[] = "setting\n  I In Settings, you can configure time, wifi, volume and other basic Rubik’s Cube-1 core features.";
static const char folder_msg_info[] = "SD file\n  After the TF card is inserted, the files in the card can be managed simply.";
static const char theme_msg_info[] = "theme\n  Press the button to switch to light/dark style, you can also refresh the page by switching themes.";
static const char RGB_msg_info[] = "RGB led\n  Set the color of RGBled.";
static const char Calendar_msg_info[] = "Calendar\n  To view the calendar information, note that you need to set the time first.";
static const char Gyroscop_msg_info[] = "Gyroscop\n  The machine has an electronic gyroscope onboard. This is a simple demonstration.";
static const char I2Cdevice_msg_info[] = "I2C device\n  I2C device monitor, automatic detection after insertion of I2C device.The device needs to be supported by the board.";

static TaskHandle_t StateBarTaskHandle;             //状态栏刷新任务句柄
static TaskHandle_t TouchKeyTaskHandler;            //触摸按键任务句柄
static TaskHandle_t DialPlatTaskHandler;            //表盘刷新任务句柄

static void Create_Use_Message(const char* msg_info);
static void Create_Main_Page(void);
static void Create_Software_Page(void);
static void Create_Module_Page(void);
static void Gui_Main_Delete(void);

static void Btn_Event_Handle(lv_obj_t* obj, lv_event_t event);

static void State_Bar_Task(void* arg);
static void Touch_Key_Task(void* arg);
static void DialPlat_Task(void* arg);

/**
  * @brief  创建主界面
  * @param  void
  * @retval void
  * @note   这里是gui所有界面的开始，创建分别创建了状态栏和菜单选项卡。
  */
void Gui_Main_Create(void)
{
    lv_obj_t* screen = lv_scr_act();              //获取屏幕对象

    //创建注主界面对象
    gui_main_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_main_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    lv_style_init(&sys_theme_style);
    lv_style_set_bg_color(&sys_theme_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_radius(&sys_theme_style, LV_STATE_DEFAULT, 0);
    lv_obj_add_style(gui_main_obj, LV_OBJ_PART_MAIN, &sys_theme_style);

    //创建状态栏
    state_bar = lv_obj_create(gui_main_obj, NULL);
    lv_obj_set_size(state_bar, SCREEN_WIDTH, 20);

    lv_style_init(&state_bar_style);
    //lv_style_copy(&state_bar_style, &sys_theme_style);
    //lv_style_set_bg_color(&state_bar_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_radius(&state_bar_style, LV_STATE_DEFAULT, 0);
    lv_style_set_border_side(&state_bar_style, LV_STATE_FOCUSED, LV_BORDER_SIDE_NONE);
    lv_style_set_border_side(&state_bar_style, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);
    lv_obj_add_style(state_bar, LV_OBJ_PART_MAIN, &state_bar_style);

    //创建状态栏应用标签
    SB_function_label = lv_label_create(state_bar, NULL);
    lv_label_set_text(SB_function_label, state_bar_function);
    lv_label_set_long_mode(SB_function_label, LV_LABEL_LONG_SROLL);
    lv_obj_set_width(SB_function_label, 220);
    lv_obj_align(SB_function_label, state_bar, LV_ALIGN_IN_LEFT_MID, 10,0);

    //创建状态栏电池标签
    SB_battery_label = lv_label_create(state_bar, NULL); 
    lv_label_set_text(SB_battery_label, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(SB_battery_label, state_bar, LV_ALIGN_IN_RIGHT_MID, -10,0);

    //创建状态栏刷新任务
    xTaskCreate(State_Bar_Task, "State_Bar_Task", 2048, NULL, 13, &StateBarTaskHandle);

    //创建主界面选项卡
    menu_tabview = lv_tabview_create(gui_main_obj, NULL);
    lv_obj_set_size(menu_tabview, SCREEN_WIDTH, 220);
    lv_obj_set_pos(menu_tabview, 0, 20);
    lv_tabview_set_btns_pos(menu_tabview, LV_TABVIEW_TAB_POS_NONE);                    //隐藏按钮选项栏
    lv_tabview_set_anim_time(menu_tabview, 100);

    //添加创建主菜单页面
    MT_main_page = lv_tabview_add_tab(menu_tabview, "main page");
    Create_Main_Page();
    
    //添加创建软件菜单页面
    MT_software_page = lv_tabview_add_tab(menu_tabview, "software page");
    Create_Software_Page();

    //添加创建模块菜单页面
    MT_module_page = lv_tabview_add_tab(menu_tabview, "module page");
    Create_Module_Page();

    //创建触摸按键处理任务
    xTaskCreate(Touch_Key_Task, "Touch_Key_Task", 2048, NULL, 15, &TouchKeyTaskHandler);

    //设置界面层级为主界面
    interface_level = MAIN_TABVIEW_INTER_LEVEL;
}

/**
  * @brief  创建主界面下主菜单
  * @param  void
  * @retval void
  * @note   创建主菜单下的表盘以及三个基础功能。
  */
static void Create_Main_Page(void)
{
    //创建主菜单页面
    lv_page_set_scrl_layout(MT_main_page, LV_LAYOUT_PRETTY_MID);
    lv_page_set_scrollbar_mode(MT_main_page, LV_SCROLLBAR_MODE_OFF);

    //创建表盘
    MT_MP_dialPlate_cnt = lv_cont_create(MT_main_page, NULL);               
    lv_obj_set_size(MT_MP_dialPlate_cnt, 300, 100);
    lv_obj_set_drag_parent(MT_MP_dialPlate_cnt,true);
    lv_style_init(&dialPlate_style);
    lv_style_set_border_width(&dialPlate_style, LV_STATE_DEFAULT, 2);
    lv_style_set_border_color(&dialPlate_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_add_style(MT_MP_dialPlate_cnt, LV_OBJ_PART_MAIN, &dialPlate_style);

    //创建日期显示标签
    lv_style_init(&digital_date_style);
    lv_style_set_text_font(&digital_date_style, LV_STATE_DEFAULT, &lv_font_montserrat_20);
    MT_MP_DB_date_label = lv_label_create(MT_MP_dialPlate_cnt, NULL);          
    lv_obj_set_size(MT_MP_DB_date_label, 280, 20);
    lv_obj_align(MT_MP_DB_date_label, MT_MP_dialPlate_cnt, LV_ALIGN_IN_TOP_LEFT, 70, 10);
    lv_obj_add_style(MT_MP_DB_date_label, LV_OBJ_PART_MAIN, &digital_date_style);
    lv_label_set_text(MT_MP_DB_date_label, digital_date);

    //创建时间显示标签
    lv_style_init(&digital_time_style);
    lv_style_set_text_font(&digital_time_style, LV_STATE_DEFAULT, &BAUHS93_60_font);
    MT_MP_DB_time_label = lv_label_create(MT_MP_dialPlate_cnt, NULL);           
    lv_obj_set_size(MT_MP_DB_time_label, 280, 60);
    lv_obj_align(MT_MP_DB_time_label, MT_MP_DB_date_label, LV_ALIGN_OUT_BOTTOM_LEFT, -45, 0);
    lv_obj_add_style(MT_MP_DB_time_label, LV_OBJ_PART_MAIN, &digital_time_style);
    lv_label_set_text(MT_MP_DB_time_label, digital_time);

    //创建表盘刷新任务
    xTaskCreate(DialPlat_Task, "DialPlat_Task", 2048, NULL, 14, &DialPlatTaskHandler);

    //创建设置图片按钮
    lv_style_init(&app_btn_style);
    lv_style_set_clip_corner(&app_btn_style, LV_STATE_DEFAULT, true);
    lv_style_set_radius(&app_btn_style, LV_STATE_DEFAULT, 20);

    MT_MP_setting_imgbtn = lv_imgbtn_create(MT_main_page, NULL);
    lv_obj_add_style(MT_MP_setting_imgbtn, LV_IMGBTN_PART_MAIN, &app_btn_style);
    lv_imgbtn_set_src(MT_MP_setting_imgbtn, LV_BTN_STATE_RELEASED, &setting_icon);
    lv_imgbtn_set_src(MT_MP_setting_imgbtn, LV_BTN_STATE_PRESSED, &touch_icon);
    lv_obj_set_drag_parent(MT_MP_setting_imgbtn, true);
    lv_obj_set_event_cb(MT_MP_setting_imgbtn, Btn_Event_Handle);
    
    //创建文件夹图片按钮
    MT_MP_folder_imgbtn = lv_imgbtn_create(MT_main_page, MT_MP_setting_imgbtn);
    lv_imgbtn_set_src(MT_MP_folder_imgbtn, LV_STATE_DEFAULT, &folder_icon);

    //创建主题切换图片按钮
    MT_MP_theme_imgbtn = lv_imgbtn_create(MT_main_page, MT_MP_setting_imgbtn);
    lv_imgbtn_set_src(MT_MP_theme_imgbtn, LV_BTN_STATE_RELEASED, &moon_icon);
    lv_imgbtn_set_src(MT_MP_theme_imgbtn, LV_BTN_STATE_PRESSED, &touch_icon);
    lv_imgbtn_set_src(MT_MP_theme_imgbtn, LV_BTN_STATE_CHECKED_RELEASED, &sun_icon);
    lv_imgbtn_set_src(MT_MP_theme_imgbtn, LV_BTN_STATE_CHECKED_PRESSED, &touch_icon);
    if(Current_Theme == Light_Theme){
        lv_imgbtn_set_state(MT_MP_theme_imgbtn, LV_BTN_STATE_RELEASED);
    }else if(Current_Theme == Dark_Theme){
        lv_imgbtn_set_state(MT_MP_theme_imgbtn, LV_BTN_STATE_CHECKED_RELEASED);
    }
    lv_imgbtn_set_checkable(MT_MP_theme_imgbtn, true);
}

/**
  * @brief  创建主界面下软件菜单
  * @param  void
  * @retval void
  * @note   此菜单下存放的是板载功能，只需要主机就可以独立运行。
  */
static void Create_Software_Page(void)
{
    lv_page_set_scrl_layout(MT_software_page, LV_LAYOUT_PRETTY_MID);
    lv_page_set_scrollbar_mode(MT_software_page, LV_SCROLLBAR_MODE_OFF );
    // lv_page_set_edge_flash(MT_software_page, true);

    MT_SP_RGBled_imgbtn = lv_imgbtn_create(MT_software_page, MT_MP_setting_imgbtn);
    lv_imgbtn_set_src(MT_SP_RGBled_imgbtn, LV_STATE_DEFAULT, &led_icon);

    MT_SP_calendar_imgbtn = lv_imgbtn_create(MT_software_page, MT_MP_setting_imgbtn);
    lv_imgbtn_set_src(MT_SP_calendar_imgbtn, LV_STATE_DEFAULT, &calendar_icon);

    // MT_SP_alarmClock_imgbtn = lv_imgbtn_create(MT_software_page, MT_MP_setting_imgbtn);
    // lv_imgbtn_set_src(MT_SP_alarmClock_imgbtn, LV_STATE_DEFAULT, &alarm_clock_icon);

    MT_SP_gyroscope_imgbtn = lv_imgbtn_create(MT_software_page, MT_MP_setting_imgbtn);
    lv_imgbtn_set_src(MT_SP_gyroscope_imgbtn, LV_STATE_DEFAULT, &gyroscope_icon);
}

/**
  * @brief  创建主界面下模块菜单
  * @param  void
  * @retval void
  * @note   此菜单下存放的功能都要外接模块才能使用。
  */
static void Create_Module_Page(void)
{
    lv_page_set_scrl_layout(MT_module_page, LV_LAYOUT_PRETTY_MID);
    lv_page_set_scrollbar_mode(MT_module_page, LV_SCROLLBAR_MODE_OFF);
    // lv_page_set_edge_flash(MT_module_page, true);

    MT_OP_I2Cdevice_imgbtn = lv_imgbtn_create(MT_module_page, MT_MP_setting_imgbtn);
    lv_imgbtn_set_src(MT_OP_I2Cdevice_imgbtn, LV_STATE_DEFAULT, &i2c_icon);
}

/**
  * @brief  释放主界面
  * @param  void
  * @retval void
  */
static void Gui_Main_Delete(void)
{
    vTaskDelete(DialPlatTaskHandler);
    vTaskDelete(StateBarTaskHandle);
    lv_obj_del(gui_main_obj);
    vTaskDelete(TouchKeyTaskHandler);
}

/**
  * @brief  主界面下所有图片按键的事件回调函数
  * @param  obj：lv对象
  * @param  event：事件类型
  * @retval void
  */
static void Btn_Event_Handle(lv_obj_t* obj, lv_event_t event)
{
    if(obj == MT_MP_setting_imgbtn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_Main_Delete();
            Gui_Setting_Create();
        }else if(event == LV_EVENT_LONG_PRESSED){
            Create_Use_Message(setting_msg_info);
        }
    }else if(obj == MT_MP_folder_imgbtn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_Main_Delete();
            Gui_folder_Create();
        }else if(event == LV_EVENT_LONG_PRESSED){
            Create_Use_Message(folder_msg_info);
        }
    }else if(obj == MT_MP_theme_imgbtn){
        if(event == LV_EVENT_VALUE_CHANGED) {
            uint32_t flag = 0;
            lv_btn_state_t style_btn_state = lv_imgbtn_get_state(obj);
            if(LV_BTN_STATE_RELEASED == style_btn_state){
                flag = LV_THEME_MATERIAL_FLAG_LIGHT;
                Current_Theme = Light_Theme;
            }else if(LV_BTN_STATE_CHECKED_RELEASED == style_btn_state){
                flag = LV_THEME_MATERIAL_FLAG_DARK;
                Current_Theme = Dark_Theme;
            }
            LV_THEME_DEFAULT_INIT(lv_theme_get_color_primary(), lv_theme_get_color_secondary(),
                flag, lv_theme_get_font_small(), lv_theme_get_font_normal(), 
                lv_theme_get_font_subtitle(), lv_theme_get_font_title());
        }else if(event == LV_EVENT_LONG_PRESSED){
            Create_Use_Message(theme_msg_info);
        }
    }else if(obj == MT_SP_RGBled_imgbtn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_Main_Delete();
            Gui_Rgbled_Create();
        }else if(event == LV_EVENT_LONG_PRESSED){
            Create_Use_Message(RGB_msg_info);
        }
    }else if(obj == MT_SP_calendar_imgbtn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_Main_Delete();
            Gui_Calendar_Create();
        }else if(event == LV_EVENT_LONG_PRESSED){
            Create_Use_Message(Calendar_msg_info);
        }
    }else if(obj == MT_SP_gyroscope_imgbtn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_Main_Delete();
            Gui_Gyroscop_Create();
        }else if(event == LV_EVENT_LONG_PRESSED){
            Create_Use_Message(Gyroscop_msg_info);
        }
    }else if(obj == MT_OP_I2Cdevice_imgbtn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_Main_Delete();
            Gui_I2Cdevice_Create();
        }else if(event == LV_EVENT_LONG_PRESSED){
            Create_Use_Message(I2Cdevice_msg_info);
        }
    }
}

/**
  * @brief  状态栏刷新任务
  * @param  arg：NULL
  * @retval void
  */
static void State_Bar_Task(void* arg)
{   
    char battery_str[5] = {0};
    uint8_t power = 0;
    while(1){
        if(Ip5306_Check_Charge() == 1){
            sprintf(battery_str, LV_SYMBOL_CHARGE);
        }else{
            power = Ip5306_Check_Power();
            if(power == 100)
                sprintf(battery_str, LV_SYMBOL_BATTERY_FULL);
            else if(power == 75)
                sprintf(battery_str, LV_SYMBOL_BATTERY_3);
            else if(power == 50)
                sprintf(battery_str, LV_SYMBOL_BATTERY_2);
            else if(power == 25)
                sprintf(battery_str, LV_SYMBOL_BATTERY_1);
            else
                sprintf(battery_str, LV_SYMBOL_BATTERY_EMPTY);
        }
        lv_label_set_text(SB_battery_label, battery_str);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
  * @brief  触摸按键任务处理函数
  * @param  arg：NULL
  * @retval void
  */
static void Touch_Key_Task(void* arg)
{
    int8_t tk_value = 0;
    uint16_t tabview_num = 0;
    while(1){
        tk_value = ft6x36_get_touch_key_value();

        if(interface_level == MAIN_TABVIEW_INTER_LEVEL){
            if(1 == tk_value){
                lv_tabview_set_tab_act(menu_tabview, 0, LV_ANIM_ON);
            }else if(2 == tk_value){
                lv_tabview_set_tab_act(menu_tabview, 0, LV_ANIM_ON);
            }else if(3 == tk_value){
                tabview_num = lv_tabview_get_tab_act(menu_tabview);
                if(0 == tabview_num)
                    Create_Use_Message(main_menu_msg_info);
                else if(1 == tabview_num)
                    Create_Use_Message(software_menu_msg_info);
                else if(2 == tabview_num)
                    Create_Use_Message(module_menu_msg_info);
            }
        }else if(interface_level == USE_MESSAG_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(msgbox_back);
            }else if(2 == tk_value){
                lv_obj_del(msgbox_back);
                vTaskDelay(30 / portTICK_PERIOD_MS);
                lv_tabview_set_tab_act(menu_tabview, 0, LV_ANIM_ON);
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/**
  * @brief  主界面下表盘的刷新任务
  * @param  arg：NULL
  * @retval void
  */
static void DialPlat_Task(void* arg)
{
    uint32_t year=0, month=0, day=0, hour=0, min=0, sec=0;
    char month_name[10];
    while(1){
        GUI_Get_Sys_Time(&year, &month, &day, &hour, &min, &sec);
        sprintf(digital_time, "%02d:%02d:%02d", hour, min, sec);
        lv_label_set_text(MT_MP_DB_time_label, digital_time);

        
        if(sec%10 == 1){        //限定每10秒更新一次日期
            if(month == 1 && day == 1 && year == 1970){
                sprintf(digital_date, "       run time");
            }else{
                switch(month){
                    case 1: strcpy(month_name, "January");
                            break;
                    case 2: strcpy(month_name, "February");
                            break;
                    case 3: strcpy(month_name, "March");
                            break;
                    case 4: strcpy(month_name, "April");
                            break;
                    case 5: strcpy(month_name, "May");
                            break;
                    case 6: strcpy(month_name, "June");
                            break;
                    case 7: strcpy(month_name, "July");
                            break;
                    case 8: strcpy(month_name, "August");
                            break;
                    case 9: strcpy(month_name, "September");
                            break;
                    case 10: strcpy(month_name, "October");
                            break;
                    case 11: strcpy(month_name, "November");
                            break;
                    case 12: strcpy(month_name, "December");
                            break;
                }
                sprintf(digital_date, "%9s  %d  %d", month_name, day, year);
            }   
            lv_label_set_text(MT_MP_DB_date_label, digital_date);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static const char* use_message_btn[] = {"OK",""};

static void Use_Message_event_cb(lv_obj_t* obj, lv_event_t event);

/**
  * @brief  创建使用信息提示框
  * @param  msg_info：具体要传递的信息
  * @retval void
  */
static void Create_Use_Message(const char* msg_info)
{
    lv_style_init(&use_message_style);
    lv_style_set_bg_color(&use_message_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&use_message_style, LV_STATE_DEFAULT, 200);

    msgbox_back = lv_obj_create(gui_main_obj, NULL);
    lv_obj_set_size(msgbox_back, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_add_style(msgbox_back, LV_OBJ_PART_MAIN, &use_message_style);

    use_message_msgbox = lv_msgbox_create(msgbox_back, NULL);
    lv_obj_set_width(use_message_msgbox,SCREEN_HEIGHT);                                     
    lv_msgbox_set_text(use_message_msgbox, msg_info);
    lv_msgbox_add_btns(use_message_msgbox, use_message_btn);
    lv_obj_align(use_message_msgbox, msgbox_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(use_message_msgbox, Use_Message_event_cb);

    interface_level = USE_MESSAG_INTER_LEVEL;
}

/**
  * @brief  使用信息消息框的事件回调函数
  * @param  obj：lv对象
  * @param  event：事件类型
  * @retval void
  */
static void Use_Message_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if(obj == use_message_msgbox){
        if(event == LV_EVENT_DELETE){
            lv_obj_del_async(msgbox_back);
            msgbox_back = NULL;
            interface_level = MAIN_TABVIEW_INTER_LEVEL;
        }else if(event == LV_EVENT_VALUE_CHANGED){
            lv_msgbox_start_auto_close(use_message_msgbox, 0);
        }
    }
}

LV_IMG_DECLARE(C001_logo_icon);         //开机logo图片声明

/**
  * @brief  显示开机界面logo
  * @param  void
  * @retval void
  */
void Gui_BootUp_Create(void)
{
    lv_obj_t* screen = lv_scr_act(); 
    lv_obj_t* bootup_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(bootup_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    lv_obj_t* bootup_img = lv_img_create(bootup_obj, NULL);
    lv_img_set_src(bootup_img, &C001_logo_icon);
    lv_obj_align(bootup_img, bootup_obj, LV_ALIGN_CENTER, 0, 0);
    lv_task_handler();

    SK6812_Set_RGBColor(0xff, 0x00, 0x00);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    SK6812_Set_RGBColor(0x00, 0xff, 0x00);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    SK6812_Set_RGBColor(0x00, 0x00, 0xff);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    SK6812_Set_RGBColor(0x00, 0x00, 0x00);

    lv_obj_del(bootup_obj);
}

