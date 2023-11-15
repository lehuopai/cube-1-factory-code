#include "gui_setting_interface.h"
#include "gui_main_interface.h"
#include "GUI_C001_sys.h"
#include "gui_sys_time.h"

#include "wifi_driver.h"
#include "backlight_pwm.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../lvgl_esp32_drivers/lvgl_touch/ft6x36.h"

enum {
    SETTING_TABVIEW_INTER_LEVEL = 0,
    USE_MESSAG_INTER_LEVEL = 1,
};
static interface_level_t interface_level = SETTING_TABVIEW_INTER_LEVEL;

static lv_obj_t* gui_setting_obj;      //设置界面对象

static lv_obj_t* setting_tabview;          //设置选项卡(ST)

static lv_obj_t* ST_menu_page;                 //选项卡菜单页面(ST_MP)

static lv_obj_t* ST_MP_menu_list;                  //选项卡菜单页面选项列表(ST_MP_ML)

static lv_obj_t* ST_MP_ML_timeset_btn;                 //选项列表时间设置按钮
static lv_obj_t* ST_MP_ML_lightset_btn;                //选项列表背光设置按钮
static lv_obj_t* ST_MP_ML_wifista_btn;                 //选项列表wifista设置按钮
static lv_obj_t* ST_MP_ML_wifiap_btn;                  //选项列表wifiap设置按钮
static lv_obj_t* ST_MP_ML_wifisc_btn;                  //选项列表WIFIsmartconfig设置按钮

static lv_obj_t* ST_timeset_page;               //选项卡时间设置页面(ST_TP)

static lv_obj_t* ST_TP_year_roller;                //选项卡时间设置页面年选择滚轮
static lv_obj_t* ST_TP_month_roller;               //选项卡时间设置页面月选择滚轮
static lv_obj_t* ST_TP_day_roller;                 //选项卡时间设置页面日选择滚轮
static lv_obj_t* ST_TP_hour_roller;                //选项卡时间设置页面时选择滚轮
static lv_obj_t* ST_TP_min_roller;                 //选项卡时间设置页面分选择滚轮
static lv_obj_t* ST_TP_sec_roller;                 //选项卡时间设置页面秒选择滚轮
static lv_obj_t* ST_TP_timeset_btn;                //选项卡时间设置页面时间设置按钮

static lv_obj_t* ST_backlight_page;             //选项卡背光设置页面(ST_BL)
static lv_obj_t* ST_BL_light_slider;                //选项卡背光页面调节滑块
static lv_obj_t* ST_BL_lightnum_label;              //选项卡背光页面滑块值

static lv_obj_t* ST_wifista_page;               //选项卡wifi—sta设置界面(ST_SP)

static lv_obj_t* ST_SP_scan_switch;                 //wifi sta界面开关
static lv_obj_t* ST_SP_ap_list;                     //wifi sta界面下wifiap列表(ST_SP_AL)

static lv_obj_t* ST_wifiap_page;                //选项卡wifi-ap设置界面(ST_AP)

static lv_obj_t* ST_AP_ap_switch;                   //wifi ap界面ap开关
static lv_obj_t* ST_AP_ssid_textarea;               //wifi ap界面SSID文本域
static lv_obj_t* ST_AP_pass_textarea;               //wifi ap界面password文本域

static lv_obj_t* ST_wifisc_page;                //选项卡wifi-sc设置界面(ST_SP)    
static lv_obj_t* ST_SP_sc_switch;                   //wifi sc界面sc开关
static lv_obj_t* ST_SP_ssid_textarea;               //wifi sc界面SSID文本域
static lv_obj_t* ST_SP_pass_textarea;               //wifi sc界面password文本域
static lv_obj_t* ST_SP_connect_label;               //wifi 连接信息标签

static lv_obj_t** ST_SP_AL_wifista_list_btn = NULL;     //wifi sta 设置ap列表里的按钮列表

static void Create_Menu_Page(void);
static void Create_Timeset_Page(void);
static void Create_BackLightSet_Page(void);
static void Create_WIFIsta_Page(void);
static void Create_WIFIap_Page(void);
static void Create_WIFIsc_Page(void);

static void List_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event);
static void Timeset_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event);
static void Backlightset_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event);
static void WIFIsta_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event);
static void WIFIap_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event);
static void WIFIsc_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event);

static TaskHandle_t TouchKeyTaskHandler;            //触摸按键任务句柄
static void Touch_Key_Task(void* arg);

/**
  * @brief  创建设置主界面
  * @param  void
  * @retval void
  * @note   由一个选项卡构成，每个设置内容都是一页。
  */
void Gui_Setting_Create(void)
{
    lv_obj_t* screen = lv_scr_act();              //获取屏幕对象

    //创建设置对象
    gui_setting_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_setting_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建设置选项卡
    setting_tabview = lv_tabview_create(gui_setting_obj, NULL);
    lv_obj_set_size(setting_tabview, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_tabview_set_btns_pos(setting_tabview, LV_TABVIEW_TAB_POS_NONE);                    //隐藏按钮选项栏
    lv_tabview_set_anim_time(setting_tabview, 0);
    lv_obj_set_gesture_parent(setting_tabview, false);

    //添加菜单页面
    ST_menu_page = lv_tabview_add_tab(setting_tabview, "menu");
    Create_Menu_Page();

    ST_timeset_page = lv_tabview_add_tab(setting_tabview, "timeset");
    Create_Timeset_Page();

    ST_backlight_page = lv_tabview_add_tab(setting_tabview, "back light");
    Create_BackLightSet_Page();

    ST_wifista_page = lv_tabview_add_tab(setting_tabview, "WIFI-STA");
    Create_WIFIsta_Page();

    ST_wifiap_page = lv_tabview_add_tab(setting_tabview, "WIFI-AP");
    Create_WIFIap_Page();

    ST_wifisc_page = lv_tabview_add_tab(setting_tabview, "WIFI-SC");
    Create_WIFIsc_Page();

    //为ap列表数组分配最大所需内存
    ST_SP_AL_wifista_list_btn = malloc(sizeof(lv_obj_t*) * WIFI_AP_SCAN_MAX_NUM);

    //创建触摸按键处理任务
    xTaskCreate(Touch_Key_Task, "Touch_Key_Task", 2048, NULL, 15, &TouchKeyTaskHandler);

    //设置界面层级为设置菜单界面
    interface_level = SETTING_TABVIEW_INTER_LEVEL;
}

/**
  * @brief  创建设置菜单页面
  * @param  void
  * @retval void
  * @note   这里提供菜单列表，点击就跳转到相应的设置页面
  */
static void Create_Menu_Page(void)
{
    lv_page_set_scrollbar_mode(ST_menu_page, LV_SCROLLBAR_MODE_OFF);                    //设置隐藏滚动条

    lv_page_set_scrl_width(ST_menu_page, SCREEN_HEIGHT);
    lv_page_set_scrl_height(ST_menu_page, SCREEN_WIDTH);
    lv_page_set_scrollable_fit(ST_menu_page, LV_FIT_NONE);
    lv_obj_set_size(ST_menu_page, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建菜单页面选项列表 
    ST_MP_menu_list = lv_list_create(ST_menu_page, NULL);
    lv_obj_set_size(ST_MP_menu_list, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(ST_MP_menu_list, ST_menu_page, LV_ALIGN_IN_TOP_MID, 0, 0);
    //lv_list_set_scrollbar_mode(ST_MP_menu_list, LV_SCROLLBAR_MODE_OFF);               //设置隐藏滚动条

    //创建选项列表时间设置按钮
    ST_MP_ML_timeset_btn = lv_list_add_btn(ST_MP_menu_list, LV_SYMBOL_REFRESH, "Time setting");
    lv_obj_set_event_cb(ST_MP_ML_timeset_btn, List_Btn_Event_Handler);                     //为列表按钮添加回调

    //创建选项卡列表背光设置按钮
    ST_MP_ML_lightset_btn = lv_list_add_btn(ST_MP_menu_list, LV_SYMBOL_EYE_OPEN, "Screen brightness");
    lv_obj_set_event_cb(ST_MP_ML_lightset_btn, List_Btn_Event_Handler);                     //为列表按钮添加回调

    //创建选项列表wifi-sta设置按钮
    ST_MP_ML_wifista_btn = lv_list_add_btn(ST_MP_menu_list, LV_SYMBOL_WIFI, "WIFI-scan");
    lv_obj_set_event_cb(ST_MP_ML_wifista_btn, List_Btn_Event_Handler); 

    //创建选项列表wifi-ap设置按钮
    ST_MP_ML_wifiap_btn = lv_list_add_btn(ST_MP_menu_list, LV_SYMBOL_WIFI, "WIFI-AP");
    lv_obj_set_event_cb(ST_MP_ML_wifiap_btn, List_Btn_Event_Handler); 

    //创建选项列表wifi-sc设置按钮
    ST_MP_ML_wifisc_btn = lv_list_add_btn(ST_MP_menu_list, LV_SYMBOL_WIFI, "WIFI-SmartConfig");
    lv_obj_set_event_cb(ST_MP_ML_wifisc_btn, List_Btn_Event_Handler); 

}

/**
  * @brief  菜单页面选项按钮事件回调函数
  * @param  void
  * @retval void
  */
static void List_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event)
{
    if(obj == ST_MP_ML_timeset_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            lv_tabview_set_tab_act(setting_tabview, 1, LV_ANIM_ON);
        }
    }else if(obj == ST_MP_ML_lightset_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            lv_tabview_set_tab_act(setting_tabview, 2, LV_ANIM_ON);
        }
    }else if(obj == ST_MP_ML_wifista_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            lv_tabview_set_tab_act(setting_tabview, 3, LV_ANIM_ON);
        }
    }else if(obj == ST_MP_ML_wifiap_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            lv_tabview_set_tab_act(setting_tabview, 4, LV_ANIM_ON);
        }
    }else if(obj == ST_MP_ML_wifisc_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            lv_tabview_set_tab_act(setting_tabview, 5, LV_ANIM_ON);
        }
    }
}

static lv_style_t set_btn_style;

/**
  * @brief  创建时间设置页面
  * @param  void
  * @retval void
  */
static void Create_Timeset_Page(void)
{
    //设置页面参数
    lv_page_set_scrollbar_mode(ST_timeset_page, LV_SCROLLBAR_MODE_OFF);
    lv_page_set_scrl_width(ST_timeset_page, SCREEN_WIDTH);
    lv_page_set_scrollable_fit2(ST_timeset_page, LV_FIT_NONE, LV_FIT_MAX);
    lv_obj_set_size(ST_timeset_page, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建月选择滚轮
    ST_TP_month_roller = lv_roller_create(ST_timeset_page, NULL);
    lv_roller_set_options(ST_TP_month_roller, "January\n""February\n""March\n"
                                                "April\n""May\n""June\n""July\n"
                                                "August\n""September\n""October\n"
                                                "November\n""December",
                                                LV_ROLLER_MODE_INIFINITE);
    lv_obj_align(ST_TP_month_roller, ST_timeset_page, LV_ALIGN_IN_TOP_MID, 0, 40);
    lv_obj_t* month_roller_label = lv_label_create(ST_timeset_page, NULL);
    lv_label_set_text(month_roller_label, "Month");
    lv_obj_align(month_roller_label, ST_TP_month_roller, LV_ALIGN_OUT_TOP_MID, 0, -10);

    //创建年选择滚轮
    ST_TP_year_roller = lv_roller_create(ST_timeset_page, NULL);
    lv_roller_set_options(ST_TP_year_roller, "2020\n""2021\n""2022\n"
                                                "2023\n""2024\n""2025\n""2026\n"
                                                "2027\n""2028\n""2029\n"
                                                "2030",
                                                LV_ROLLER_MODE_INIFINITE);
    lv_obj_align(ST_TP_year_roller, ST_TP_month_roller, LV_ALIGN_OUT_LEFT_TOP, -27, 0);
    lv_roller_set_auto_fit(ST_TP_year_roller, false);
    lv_obj_set_width(ST_TP_year_roller, 90);
    lv_obj_t* year_roller_label = lv_label_create(ST_timeset_page, NULL);
    lv_label_set_text(year_roller_label, "Year");
    lv_obj_align(year_roller_label, ST_TP_year_roller, LV_ALIGN_OUT_TOP_MID, 0, -10);  

    //创建日选择滚轮
    ST_TP_day_roller = lv_roller_create(ST_timeset_page, NULL);
    lv_roller_set_options(ST_TP_day_roller, "1\n""2\n""3\n""4\n""5\n""6\n""7\n"
                                                "8\n""9\n""10\n""11\n""12\n""13\n"
                                                "14\n""15\n""16\n""17\n""18\n""19\n"
                                                "20\n""21\n""22\n""23\n""24\n""25\n"
                                                "26\n""27\n""28\n""29\n""30\n""31",
                                                LV_ROLLER_MODE_INIFINITE);
    lv_obj_align(ST_TP_day_roller, ST_TP_month_roller, LV_ALIGN_OUT_RIGHT_TOP, 5, 0);
    lv_roller_set_auto_fit(ST_TP_day_roller, false);
    lv_obj_set_width(ST_TP_day_roller, 90);
    lv_obj_t* day_roller_label = lv_label_create(ST_timeset_page, NULL);
    lv_label_set_text(day_roller_label, "Day");
    lv_obj_align(day_roller_label, ST_TP_day_roller, LV_ALIGN_OUT_TOP_MID, 0, -10);

    //创建分选择滚轮
    ST_TP_min_roller = lv_roller_create(ST_timeset_page, NULL);
    lv_roller_set_options(ST_TP_min_roller, "0\n""1\n""2\n""3\n""4\n""5\n""6\n""7\n"
                                                "8\n""9\n""10\n""11\n""12\n""13\n"
                                                "14\n""15\n""16\n""17\n""18\n""19\n"
                                                "20\n""21\n""22\n""23\n""24\n""25\n"
                                                "26\n""27\n""28\n""29\n""30\n""31\n"
                                                "32\n""33\n""34\n""35\n""36\n""37\n"
                                                "38\n""39\n""40\n""41\n""42\n""43\n"
                                                "44\n""45\n""46\n""47\n""48\n""49\n"
                                                "50\n""51\n""52\n""53\n""54\n""55\n"
                                                "56\n""57\n""58\n""59",
                                                LV_ROLLER_MODE_INIFINITE);
    lv_roller_set_auto_fit(ST_TP_min_roller, false);
    lv_obj_set_width(ST_TP_min_roller, 90);
    lv_obj_align(ST_TP_min_roller, ST_TP_month_roller, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
    lv_obj_t* min_roller_label = lv_label_create(ST_timeset_page, NULL);
    lv_label_set_text(min_roller_label, "Minute");
    lv_obj_align(min_roller_label, ST_TP_min_roller, LV_ALIGN_OUT_TOP_MID, 0, -10);
    
    //创建时选择滚轮
    ST_TP_hour_roller = lv_roller_create(ST_timeset_page, NULL);
    lv_roller_set_options(ST_TP_hour_roller, "0\n""1\n""2\n""3\n""4\n""5\n""6\n""7\n"
                                                "8\n""9\n""10\n""11\n""12\n""13\n"
                                                "14\n""15\n""16\n""17\n""18\n""19\n"
                                                "20\n""21\n""22\n""23",
                                                LV_ROLLER_MODE_INIFINITE);
    lv_roller_set_auto_fit(ST_TP_hour_roller, false);
    lv_obj_set_width(ST_TP_hour_roller, 90);
    lv_obj_align(ST_TP_hour_roller, ST_TP_min_roller, LV_ALIGN_OUT_LEFT_TOP, -10, 0);
    lv_obj_t* hour_roller_label = lv_label_create(ST_timeset_page, NULL);
    lv_label_set_text(hour_roller_label, "Hour");
    lv_obj_align(hour_roller_label, ST_TP_hour_roller, LV_ALIGN_OUT_TOP_MID, 0, -10);
    
    //创建秒选择滚轮
    ST_TP_sec_roller = lv_roller_create(ST_timeset_page, NULL);
    lv_roller_set_options(ST_TP_sec_roller, "0\n""1\n""2\n""3\n""4\n""5\n""6\n""7\n"
                                                "8\n""9\n""10\n""11\n""12\n""13\n"
                                                "14\n""15\n""16\n""17\n""18\n""19\n"
                                                "20\n""21\n""22\n""23\n""24\n""25\n"
                                                "26\n""27\n""28\n""29\n""30\n""31\n"
                                                "32\n""33\n""34\n""35\n""36\n""37\n"
                                                "38\n""39\n""40\n""41\n""42\n""43\n"
                                                "44\n""45\n""46\n""47\n""48\n""49\n"
                                                "50\n""51\n""52\n""53\n""54\n""55\n"
                                                "56\n""57\n""58\n""59",
                                                LV_ROLLER_MODE_INIFINITE);
    lv_roller_set_auto_fit(ST_TP_sec_roller, false);
    lv_obj_set_width(ST_TP_sec_roller, 90);
    lv_obj_align(ST_TP_sec_roller, ST_TP_min_roller, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_t* sec_roller_label = lv_label_create(ST_timeset_page, NULL);
    lv_label_set_text(sec_roller_label, "Sec");
    lv_obj_align(sec_roller_label, ST_TP_sec_roller, LV_ALIGN_OUT_TOP_MID, 0, -10);
    
    //创建确认设置按钮
    ST_TP_timeset_btn = lv_btn_create(ST_timeset_page, NULL);
    lv_obj_set_size(ST_TP_timeset_btn, 280, 60);
    lv_obj_align(ST_TP_timeset_btn, ST_TP_min_roller, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_t* timeset_btn_label = lv_label_create(ST_TP_timeset_btn, NULL);
    lv_label_set_text(timeset_btn_label, "Set");
    lv_style_init(&set_btn_style);
    lv_style_set_bg_opa(&set_btn_style, LV_STATE_PRESSED, LV_OPA_50);
    lv_style_set_bg_color(&set_btn_style, LV_STATE_PRESSED, LV_COLOR_CYAN);
    lv_obj_add_style(ST_TP_timeset_btn, LV_OBJ_PART_MAIN, &set_btn_style);
    lv_obj_set_event_cb(ST_TP_timeset_btn, Timeset_Btn_Event_Handler);
}

/**
  * @brief  时间设置页面事件回调函数
  * @param  void
  * @retval void
  * @note   按下设置按钮触发
  */
static void Timeset_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event)
{
    uint32_t year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
    if(obj == ST_TP_timeset_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            year = lv_roller_get_selected(ST_TP_year_roller) + 2020;
            month = lv_roller_get_selected(ST_TP_month_roller) + 1;
            day = lv_roller_get_selected(ST_TP_day_roller) + 1;
            hour = lv_roller_get_selected(ST_TP_hour_roller);
            min = lv_roller_get_selected(ST_TP_min_roller);
            sec = lv_roller_get_selected(ST_TP_sec_roller);
            GUI_Set_Sys_Time(year, month, day, hour, min, sec);          //设置系统时间
        }
    }
}

lv_style_t backlight_slider_style;

/**
  * @brief  创建背光设置页面
  * @param  void
  * @retval void
  */
static void Create_BackLightSet_Page(void)
{
    //设置页面参数
    lv_page_set_scrollbar_mode(ST_backlight_page, LV_SCROLLBAR_MODE_OFF);
    lv_page_set_scrollable_fit2(ST_backlight_page, LV_FIT_NONE, LV_FIT_MAX);
    lv_page_set_scrl_width(ST_backlight_page, SCREEN_WIDTH);

    //创建背光设置标题
    lv_obj_t* Backlight_title = lv_label_create(ST_backlight_page, NULL);
    lv_label_set_text(Backlight_title, "Back light");
    lv_obj_align(Backlight_title, ST_backlight_page, LV_ALIGN_IN_TOP_LEFT, 20, 20);

    //创建背光调节滑块
    ST_BL_light_slider = lv_slider_create(ST_backlight_page, NULL);
    lv_obj_set_size(ST_BL_light_slider, 260, 40);
    lv_obj_align(ST_BL_light_slider, ST_backlight_page, LV_ALIGN_IN_TOP_MID, 0, 60);
    lv_obj_set_event_cb(ST_BL_light_slider, Backlightset_Btn_Event_Handler);
    lv_slider_set_range(ST_BL_light_slider, 1, 100);
    lv_slider_set_value(ST_BL_light_slider, Cullent_Backlight, LV_ANIM_OFF);
    lv_style_init(&backlight_slider_style);
    lv_style_set_bg_color(&backlight_slider_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_add_style(ST_BL_light_slider, LV_SLIDER_PART_KNOB, &backlight_slider_style);
    lv_obj_add_style(ST_BL_light_slider, LV_SLIDER_PART_INDIC, &backlight_slider_style);

    //创建背光调节滑块数值
    ST_BL_lightnum_label = lv_label_create(ST_backlight_page, NULL);
    char str[5];
    sprintf(str, "%d", Cullent_Backlight);
    lv_label_set_text(ST_BL_lightnum_label, str);
    lv_obj_align(ST_BL_lightnum_label, ST_BL_light_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}


/**
  * @brief  背光设置页面事件回调函数
  * @param  void
  * @retval void
  */
static void Backlightset_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event)
{
    uint8_t light=0;
    char buf[4];
    if(obj == ST_BL_light_slider){
        if(event == LV_EVENT_VALUE_CHANGED){
            light = lv_slider_get_value(ST_BL_light_slider);
            snprintf(buf, 4, "%u", light);
            lv_label_set_text(ST_BL_lightnum_label, buf);
            Change_Backlight_pwm(light);
            Cullent_Backlight = light;
        }
    }
}

static TaskHandle_t WIFIScanTaskHandler;            //wifi扫描任务句柄
static void WIFIsta_Scan_Task(void* arg);

lv_style_t wifi_sta_switch_style;
/**
  * @brief  创建WIFI-sta设置页面
  * @param  void
  * @retval void
  */
static void Create_WIFIsta_Page(void)
{
    //设置页面参数
    lv_page_set_scrollbar_mode(ST_wifista_page, LV_SCROLLBAR_MODE_OFF);
    lv_page_set_scrollable_fit2(ST_wifista_page, LV_FIT_NONE, LV_FIT_MAX);
    lv_page_set_scrl_width(ST_wifista_page, SCREEN_WIDTH);

    //创建扫描标签
    lv_obj_t* WIFIsta_title_label = lv_label_create(ST_wifista_page, NULL);
    lv_label_set_text(WIFIsta_title_label, "WIFI-scan");
    lv_obj_align(WIFIsta_title_label, ST_wifista_page, LV_ALIGN_IN_TOP_LEFT, 20, 20);

    //创建扫描开关
    ST_SP_scan_switch = lv_switch_create(ST_wifista_page, NULL);
    lv_obj_align(ST_SP_scan_switch, ST_wifista_page, LV_ALIGN_IN_TOP_RIGHT, -50, 10);
    lv_obj_set_size(ST_SP_scan_switch, 80, 40);
    lv_style_init(&wifi_sta_switch_style);
    lv_style_set_bg_color(&wifi_sta_switch_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_add_style(ST_SP_scan_switch, LV_SWITCH_PART_INDIC, &wifi_sta_switch_style);
    lv_obj_set_event_cb(ST_SP_scan_switch, WIFIsta_Btn_Event_Handler);

    //创建ap列表
    ST_SP_ap_list = lv_list_create(ST_wifista_page, NULL);
    lv_obj_set_size(ST_SP_ap_list, SCREEN_WIDTH-20, SCREEN_HEIGHT-70);
    lv_obj_align(ST_SP_ap_list, ST_wifista_page, LV_ALIGN_IN_TOP_MID, 0, 60);
}

/**
  * @brief  WIFI-sta界面事件回调函数
  * @param  void
  * @retval void
  */
static void WIFIsta_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event)
{   
    if(obj == ST_SP_scan_switch){
        if(event == LV_EVENT_VALUE_CHANGED){
            if(true == lv_switch_get_state(ST_SP_scan_switch)){
                if(sys_wifi_ap_open_flag == true || sys_wifi_sta_open_flag == true){
                    // lv_switch_off(obj, LV_ANIM_ON);
                    return;
                }
                //创建wifi扫描任务
                xTaskCreatePinnedToCore(WIFIsta_Scan_Task, "WIFIsta_Scan_Task", 4096*4, NULL, 14, &WIFIScanTaskHandler, 0);
            }else{
                lv_list_clean(ST_SP_ap_list);
            }
        }
    }
}

lv_style_t sta_cover_style;
/**
  * @brief  WIFI-staAP扫描处理任务
  * @param  void
  * @retval void
  */
static void WIFIsta_Scan_Task(void* arg)
{
    wifi_scan_list_t ap_list[WIFI_AP_SCAN_MAX_NUM];
    uint8_t ap_count=0;
    uint8_t i=0;
    
    vTaskSuspend(TouchKeyTaskHandler);      //挂起触摸按键任务

    lv_obj_t* cover_obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(cover_obj, 320, 240);
    lv_style_init(&sta_cover_style);
    lv_style_set_bg_opa(&sta_cover_style, LV_STATE_DEFAULT, LV_OPA_80);
    lv_obj_add_style(cover_obj, LV_OBJ_PART_MAIN, &sta_cover_style);
    lv_obj_t* cover_label = lv_label_create(cover_obj, NULL);
    lv_label_set_text(cover_label, "Searching wifi AP ...");
    lv_obj_align(cover_label, cover_obj, LV_ALIGN_CENTER, 0, 0);

    ap_count = WIFI_Scan(ap_list, WIFI_AP_SCAN_MAX_NUM);
    for(i=0; i<ap_count; i++){
        ST_SP_AL_wifista_list_btn[i] = lv_list_add_btn(ST_SP_ap_list, LV_SYMBOL_WIFI, ap_list[i].SSID);
    }

    lv_obj_del(cover_obj);
    vTaskResume(TouchKeyTaskHandler);       //恢复
    vTaskDelete(NULL);
}

lv_style_t wifi_ap_switch_style;
/**
  * @brief  WIFI-ap设置界面创建函数
  * @param  void
  * @retval void
  */
static void Create_WIFIap_Page(void)
{
    //设置页面参数
    lv_page_set_scrollbar_mode(ST_wifiap_page, LV_SCROLLBAR_MODE_OFF);
    lv_page_set_scrollable_fit2(ST_wifiap_page, LV_FIT_NONE, LV_FIT_MAX);
    lv_page_set_scrl_width(ST_wifiap_page, SCREEN_WIDTH);

    //创建AP标签
    lv_obj_t* WIFIap_title_label = lv_label_create(ST_wifiap_page, NULL);
    lv_label_set_text(WIFIap_title_label, "WIFI-AP");
    lv_obj_align(WIFIap_title_label, ST_wifiap_page, LV_ALIGN_IN_TOP_LEFT, 20, 20);

    //创建AP开关
    ST_AP_ap_switch = lv_switch_create(ST_wifiap_page, NULL);
    lv_obj_align(ST_AP_ap_switch, ST_wifiap_page, LV_ALIGN_IN_TOP_RIGHT, -50, 10);
    lv_obj_set_size(ST_AP_ap_switch, 80, 40);
    if(sys_wifi_ap_open_flag == true){
        lv_switch_on(ST_AP_ap_switch, LV_ANIM_OFF);
    }
    lv_style_init(&wifi_ap_switch_style);
    lv_style_set_bg_color(&wifi_ap_switch_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_add_style(ST_AP_ap_switch, LV_SWITCH_PART_INDIC, &wifi_ap_switch_style);
    lv_obj_set_event_cb(ST_AP_ap_switch, WIFIap_Btn_Event_Handler);

    //创建ssid文本域标签
    lv_obj_t* ssid_label = lv_label_create(ST_wifiap_page, NULL);
    lv_label_set_text(ssid_label, "SSID:");
    lv_obj_align(ssid_label, ST_wifiap_page, LV_ALIGN_IN_TOP_LEFT, 20, 50);

    //创建ssid文本域
    ST_AP_ssid_textarea = lv_textarea_create(ST_wifiap_page, NULL);
    lv_obj_set_width(ST_AP_ssid_textarea, SCREEN_WIDTH-(20*2));
    lv_textarea_set_one_line(ST_AP_ssid_textarea, true);
    lv_textarea_set_cursor_hidden(ST_AP_ssid_textarea, true);
    lv_textarea_set_text(ST_AP_ssid_textarea, sys_wifi_ap_ssid);
    lv_obj_align(ST_AP_ssid_textarea, ssid_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建password文本域标签
    lv_obj_t* pass_label = lv_label_create(ST_wifiap_page, NULL);
    lv_label_set_text(pass_label, "password:");
    lv_obj_align(pass_label, ST_AP_ssid_textarea, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    //创建password文本域
    ST_AP_pass_textarea = lv_textarea_create(ST_wifiap_page, NULL);
    lv_obj_set_width(ST_AP_pass_textarea, SCREEN_WIDTH-(20*2));
    lv_textarea_set_one_line(ST_AP_pass_textarea, true);
    lv_textarea_set_cursor_hidden(ST_AP_pass_textarea, true);
    lv_textarea_set_text(ST_AP_pass_textarea, sys_wifi_ap_psss);
    lv_obj_align(ST_AP_pass_textarea, pass_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
}

/**
  * @brief  WIFI-ap界面事件回调函数
  * @param  void
  * @retval void
  */
static void WIFIap_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event)
{   
    if(obj == ST_AP_ap_switch){
        if(event == LV_EVENT_VALUE_CHANGED){
            if(true == lv_switch_get_state(ST_AP_ap_switch)){
                if(sys_wifi_ap_open_flag == true || sys_wifi_sta_open_flag == true){
                    // lv_switch_off(obj, LV_ANIM_ON);
                    return;
                }
                vTaskSuspend(TouchKeyTaskHandler);
                WIFI_Init_Softap(lv_textarea_get_text(ST_AP_ssid_textarea), 
                                lv_textarea_get_text(ST_AP_pass_textarea));
                sys_wifi_ap_open_flag = true;
                vTaskResume(TouchKeyTaskHandler);
            }else{
                if(sys_wifi_sta_open_flag == true){
                    return;
                }
                WIFI_Off();
                sys_wifi_ap_open_flag = false;
            }
        }
    }
}

static TaskHandle_t WIFIscConfigTaskHandler;            //wifisc任务句柄
static void WIFIsc_Config_Task(void* arg);
/**
  * @brief  WIFI-sc设置界面创建函数
  * @param  void
  * @retval void
  */
static void Create_WIFIsc_Page(void)
{
    //设置页面参数
    lv_page_set_scrollbar_mode(ST_wifisc_page, LV_SCROLLBAR_MODE_OFF);
    lv_page_set_scrollable_fit2(ST_wifisc_page, LV_FIT_NONE, LV_FIT_MAX);
    lv_page_set_scrl_width(ST_wifisc_page, SCREEN_WIDTH);

    //创建AP标签
    lv_obj_t* WIFIsc_title_label = lv_label_create(ST_wifisc_page, NULL);
    lv_label_set_text(WIFIsc_title_label, "WIFI-SmartConfig");
    lv_obj_align(WIFIsc_title_label, ST_wifisc_page, LV_ALIGN_IN_TOP_LEFT, 20, 20);

    //创建sc开关
    ST_SP_sc_switch = lv_switch_create(ST_wifisc_page, NULL);
    lv_obj_align(ST_SP_sc_switch, ST_wifisc_page, LV_ALIGN_IN_TOP_RIGHT, -50, 10);
    lv_obj_set_size(ST_SP_sc_switch, 80, 40);
    if(sys_wifi_sta_open_flag == true){
        lv_switch_on(ST_SP_sc_switch, LV_ANIM_OFF);
    }
    lv_style_init(&wifi_ap_switch_style);
    lv_style_set_bg_color(&wifi_ap_switch_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_add_style(ST_SP_sc_switch, LV_SWITCH_PART_INDIC, &wifi_ap_switch_style);
    lv_obj_set_event_cb(ST_SP_sc_switch, WIFIsc_Btn_Event_Handler);

    //创建ssid文本域标签
    lv_obj_t* ssid_label = lv_label_create(ST_wifisc_page, NULL);
    lv_label_set_text(ssid_label, "SSID:");
    lv_obj_align(ssid_label, ST_wifisc_page, LV_ALIGN_IN_TOP_LEFT, 20, 50);

    //创建ssid文本标签
    ST_SP_ssid_textarea = lv_textarea_create(ST_wifisc_page, NULL);
    lv_obj_set_width(ST_SP_ssid_textarea, SCREEN_WIDTH-(20*2));
    lv_textarea_set_one_line(ST_SP_ssid_textarea, true);
    lv_textarea_set_cursor_hidden(ST_SP_ssid_textarea, true);
    lv_textarea_set_text(ST_SP_ssid_textarea, sys_wifi_sta_ssid);
    lv_obj_align(ST_SP_ssid_textarea, ssid_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建password文本域标签
    lv_obj_t* pass_label = lv_label_create(ST_wifisc_page, NULL);
    lv_label_set_text(pass_label, "password:");
    lv_obj_align(pass_label, ST_SP_ssid_textarea, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    //创建password文本域
    ST_SP_pass_textarea = lv_textarea_create(ST_wifisc_page, NULL);
    lv_obj_set_width(ST_SP_pass_textarea, SCREEN_WIDTH-(20*2));
    lv_textarea_set_one_line(ST_SP_pass_textarea, true);
    lv_textarea_set_cursor_hidden(ST_SP_pass_textarea, true);
    lv_textarea_set_text(ST_SP_pass_textarea, sys_wifi_sta_psss);
    lv_obj_align(ST_SP_pass_textarea, pass_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建sc连接状态标签
    ST_SP_connect_label = lv_label_create(ST_wifisc_page, NULL);
    if(sys_wifi_sta_open_flag == true){
        lv_label_set_text(ST_SP_connect_label, "successful");
    }else{
        lv_label_set_text(ST_SP_connect_label, "no connect");
    }
    lv_obj_align(ST_SP_connect_label, ST_SP_pass_textarea, LV_ALIGN_OUT_BOTTOM_LEFT, 160, 10);
}

/**
  * @brief  WIFI-sc界面事件回调函数
  * @param  void
  * @retval void
  */
static void WIFIsc_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event)
{   
    if(obj == ST_SP_sc_switch){
        if(event == LV_EVENT_VALUE_CHANGED){
            if(true == lv_switch_get_state(ST_SP_sc_switch)){
                if(sys_wifi_ap_open_flag == true || sys_wifi_sta_open_flag == true){
                    // lv_switch_off(obj, LV_ANIM_ON);
                    return;
                }
                lv_label_set_text(ST_SP_connect_label, "connecting ...");

                // 创建sc任务并等待结束
                xTaskCreatePinnedToCore(WIFIsc_Config_Task, "WIFIsc_Config_Task", 4096*4, NULL, 14, &WIFIscConfigTaskHandler, 0);
                
            }else{
                if(sys_wifi_ap_open_flag == true){
                    return;
                }
                WIFI_Off();
                lv_label_set_text(ST_SP_connect_label, "no connect");
                bzero(sys_wifi_sta_ssid, 24);
                memcpy(sys_wifi_sta_ssid, "------", 6);
                bzero(sys_wifi_sta_psss, 24);
                memcpy(sys_wifi_sta_psss, "------", 6);
                lv_textarea_set_text(ST_SP_ssid_textarea, sys_wifi_sta_ssid);
                lv_textarea_set_text(ST_SP_pass_textarea, sys_wifi_sta_psss);
                sys_wifi_sta_open_flag = false;
            }
        }
    }
}

lv_style_t sc_cover_style;
/**
  * @brief  WIFI-sc连接任务处理函数
  * @param  void
  * @retval void
  */
static void WIFIsc_Config_Task(void* arg)
{
    vTaskSuspend(TouchKeyTaskHandler);

    lv_obj_t* cover_obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(cover_obj, 320, 240);
    lv_style_init(&sc_cover_style);
    lv_style_set_bg_opa(&sc_cover_style, LV_STATE_DEFAULT, LV_OPA_80);
    lv_obj_add_style(cover_obj, LV_OBJ_PART_MAIN, &sc_cover_style);
    lv_obj_t* cover_label = lv_label_create(cover_obj, NULL);
    lv_label_set_recolor(cover_label, true);
    lv_label_set_text(cover_label, "Turn on the phone and use \n#ff0000 EspTouch# to start network \nconfiguration.");
    lv_obj_align(cover_label, cover_obj, LV_ALIGN_CENTER, 0, 0);

    //开启sc完成后设置完成标志位
    WIFI_Smart_Config();

    lv_label_set_text(ST_SP_connect_label, "successful");
    lv_textarea_set_text(ST_SP_ssid_textarea, sys_wifi_sta_ssid);
    lv_textarea_set_text(ST_SP_pass_textarea, sys_wifi_sta_psss);
    sys_wifi_sta_open_flag = true;
    lv_obj_del(cover_obj);

    vTaskResume(TouchKeyTaskHandler);

    vTaskDelete(NULL);
}

/**
  * @brief  释放设置页面所有资源
  * @param  void
  * @retval void
  */
static void Gui_Setting_Delete(void)
{
    free(ST_SP_AL_wifista_list_btn);
    lv_obj_del(gui_setting_obj);
    vTaskDelete(TouchKeyTaskHandler);
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

        if(interface_level == SETTING_TABVIEW_INTER_LEVEL){
            if(1 == tk_value){
                tabview_num = lv_tabview_get_tab_act(setting_tabview);
                if(0 == tabview_num){
                    Gui_Main_Create();
                    Gui_Setting_Delete();
                }else{
                    lv_tabview_set_tab_act(setting_tabview, 0, LV_ANIM_ON);
                }
            }else if(2 == tk_value){
                Gui_Main_Create();
                Gui_Setting_Delete();
            }else if(3 == tk_value){

            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
