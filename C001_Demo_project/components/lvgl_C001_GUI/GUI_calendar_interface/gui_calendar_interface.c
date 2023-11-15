#include "gui_calendar_interface.h"
#include "GUI_C001_sys.h"
#include "gui_sys_time.h"
#include "gui_main_interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../lvgl_esp32_drivers/lvgl_touch/ft6x36.h"

lv_obj_t* gui_calendar_obj;         //日历界面对象

lv_obj_t* main_calendar;                //主日历对象


static TaskHandle_t TouchKeyTaskHandler;            //触摸按键任务句柄

static void Gui_Calendar_Delete(void);

static void Touch_Key_Task(void* arg);

/**
  * @brief  创建日历界面
  * @param  void
  * @retval void
  */
void Gui_Calendar_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建日历界面对象
    gui_calendar_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_calendar_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建主日历对象
    main_calendar = lv_calendar_create(screen, NULL);
    lv_obj_set_size(main_calendar, 300, 220);
    lv_obj_align(main_calendar, screen, LV_ALIGN_CENTER, 0, 0);

    /*Make the date number smaller to be sure they fit into their area*/
    lv_obj_set_style_local_text_font(main_calendar, LV_CALENDAR_PART_DATE, LV_STATE_DEFAULT, lv_theme_get_font_small());

    //设置今天的时间
    lv_calendar_date_t today;
    GUI_Get_Sys_Time(&today.year, &today.month, &today.day, NULL, NULL, NULL);
    lv_calendar_set_today_date(main_calendar, &today);
    lv_calendar_set_showed_date(main_calendar, &today);

    //创建触摸按键处理任务
    xTaskCreate(Touch_Key_Task, "Touch_Key_Task", 2048, NULL, 15, &TouchKeyTaskHandler);
}

/**
  * @brief  释放Calendar界面
  * @param  void
  * @retval void
  */
static void Gui_Calendar_Delete(void)
{
    lv_obj_del(gui_calendar_obj);
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
    while(1){
        tk_value = ft6x36_get_touch_key_value();

        if(1 == tk_value){
            Gui_Main_Create();
            Gui_Calendar_Delete();
        }else if(2 == tk_value){
            Gui_Main_Create();
            Gui_Calendar_Delete();
        }else if(3 == tk_value){

        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}