#include "gui_rgbled_interface.h"
#include "GUI_C001_sys.h"
#include "gui_main_interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../lvgl_esp32_drivers/lvgl_touch/ft6x36.h"

#include "sk6812_driver.h"

static lv_style_t black_btn_style;

static lv_obj_t*  gui_rgbled_obj;               //RGBled对象

static lv_obj_t* rgbled_color_cpicker;              //rgb颜色选择器
static lv_obj_t* rgbled_black_button;               //rgb黑色按钮

static TaskHandle_t TouchKeyTaskHandler;            //触摸按键任务句柄

static void Touch_Key_Task(void* arg);
static void Color_Changed_event_cb(lv_obj_t* obj, lv_event_t event);

/**
  * @brief  创建RGBled界面
  * @param  void
  * @retval void
  */
void Gui_Rgbled_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建RGBled对象
    gui_rgbled_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_rgbled_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建rgb颜色选择器lv_btn_create();
    rgbled_color_cpicker = lv_cpicker_create(gui_rgbled_obj, NULL);
    lv_obj_set_size(rgbled_color_cpicker, 200, 200);
    lv_obj_align(rgbled_color_cpicker, gui_rgbled_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(rgbled_color_cpicker, Color_Changed_event_cb);
    
    //创建黑色按键
    rgbled_black_button = lv_btn_create(gui_rgbled_obj, NULL);
    lv_obj_set_size(rgbled_black_button, 70, 70);
    lv_obj_align(rgbled_black_button, gui_rgbled_obj, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t* black_lable = lv_label_create(rgbled_black_button, NULL);
    lv_label_set_text(black_lable, "Black");
    lv_style_init(&black_btn_style);
    lv_style_set_bg_color(&black_btn_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_text_color(&black_btn_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(rgbled_black_button, LV_OBJ_PART_MAIN, &black_btn_style);
    lv_obj_add_style(black_lable, LV_OBJ_PART_MAIN, &black_btn_style);
    lv_obj_set_event_cb(rgbled_black_button, Color_Changed_event_cb);

    //创建触摸按键处理任务
    xTaskCreate(Touch_Key_Task, "Touch_Key_Task", 2048, NULL, 15, &TouchKeyTaskHandler);
}

/**
  * @brief  释放RGBled界面
  * @param  void
  * @retval void
  */
static void Gui_Rgbled_Delete(void)
{
    lv_obj_del(gui_rgbled_obj);
    vTaskDelete(TouchKeyTaskHandler);
}

/**
  * @brief  颜色选择器颜色时间回调函数
  * @param  obj ：对象
  * @param  event ：事件
  * @retval void
  */
static void Color_Changed_event_cb(lv_obj_t* obj, lv_event_t event)
{
    lv_color_t get_color;
    uint8_t red=0, green=0, blue=0;
    if(obj == rgbled_color_cpicker){
        if(event == LV_EVENT_VALUE_CHANGED){
            get_color = lv_cpicker_get_color(obj);
            red = (uint8_t)get_color.ch.red *8;
            green = (uint8_t)(get_color.ch.green_h<<3 | get_color.ch.green_l) *4;
            blue = (uint8_t)get_color.ch.blue *8;
            SK6812_Set_RGBColor(red, green, blue);
        }
    }else if(obj == rgbled_black_button){
        if(event == LV_EVENT_SHORT_CLICKED){
            SK6812_Set_RGBColor(0x00, 0x00, 0x00);
        }
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
    while(1){
        tk_value = ft6x36_get_touch_key_value();

        if(1 == tk_value){
            Gui_Main_Create();
            Gui_Rgbled_Delete();
        }else if(2 == tk_value){
            Gui_Main_Create();
            Gui_Rgbled_Delete();
        }else if(3 == tk_value){

        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}