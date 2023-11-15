#include "gui_gyroscop_interface.h"
#include "GUI_C001_sys.h"
#include "gui_main_interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../lvgl_esp32_drivers/lvgl_touch/ft6x36.h"

#include "mpu6886_driver.h"

static lv_style_t gyroscop_obj_style;
static lv_style_t out_circle_style;
static lv_style_t in_dot_style;
static lv_style_t angle_circle_style;

static lv_obj_t*    gui_gyroscop_obj;               //gyroscop对象

static lv_obj_t*    angle_circle;                       //角度数据园

static TaskHandle_t AngleCircleMoveTaskHandle;      //圆心移动任务句柄
static void Angle_Circle_Move_Task(void* arg);

static TaskHandle_t TouchKeyTaskHandler;            //触摸按键任务句柄
static void Touch_Key_Task(void* arg);


/**
  * @brief  创建Gyroscop界面
  * @param  void
  * @retval void
  */
void Gui_Gyroscop_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建gyroscop对象
    gui_gyroscop_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_gyroscop_obj, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_style_init(&gyroscop_obj_style);
    lv_style_set_bg_color(&gyroscop_obj_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_border_width(&gyroscop_obj_style, LV_STATE_DEFAULT, 0);
    lv_style_set_radius(&gyroscop_obj_style, LV_STATE_DEFAULT, 0);
    lv_obj_add_style(gui_gyroscop_obj, LV_OBJ_PART_MAIN, &gyroscop_obj_style);

    lv_obj_t* out_circle = lv_obj_create(gui_gyroscop_obj, NULL);
    lv_obj_set_size(out_circle, SCREEN_HEIGHT, SCREEN_HEIGHT);
    lv_obj_align(out_circle, gui_gyroscop_obj, LV_ALIGN_CENTER, 0, 0);
    lv_style_init(&out_circle_style);
    lv_style_set_radius(&out_circle_style, LV_STATE_DEFAULT, 255);
    lv_style_set_bg_color(&out_circle_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(out_circle, LV_OBJ_PART_MAIN, &out_circle_style);

    lv_obj_t* in_dot = lv_obj_create(out_circle, NULL);
    lv_obj_set_size(in_dot, 10, 10);
    lv_obj_align(in_dot, out_circle, LV_ALIGN_CENTER, 0, 0);
    lv_style_init(&in_dot_style);
    lv_style_set_radius(&in_dot_style, LV_STATE_DEFAULT, 255);
    lv_style_set_bg_color(&in_dot_style, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_border_width(&in_dot_style, LV_STATE_DEFAULT, 0);
    lv_obj_add_style(in_dot, LV_OBJ_PART_MAIN, &in_dot_style);

    angle_circle = lv_obj_create(out_circle, NULL);
    lv_obj_set_size(angle_circle, 40, 40);
    // lv_obj_align(angle_circle, out_circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(angle_circle, ANGLE_CENTRE_X, ANGLE_CENTRE_Y);
    lv_style_init(&angle_circle_style);
    lv_style_set_radius(&angle_circle_style, LV_STATE_DEFAULT, 255);
    lv_style_set_border_color(&angle_circle_style, LV_STATE_DEFAULT, LV_COLOR_CYAN);
    lv_style_set_bg_color(&angle_circle_style, LV_STATE_DEFAULT, LV_COLOR_CYAN);
    lv_style_set_bg_opa(&angle_circle_style, LV_STATE_DEFAULT, LV_OPA_50);
    lv_obj_add_style(angle_circle, LV_OBJ_PART_MAIN, &angle_circle_style);
    
    //创建触摸按键处理任务
    xTaskCreate(Touch_Key_Task, "Touch_Key_Task", 2048, NULL, 15, &TouchKeyTaskHandler);

    //创建圆心移动处理任务
    xTaskCreate(Angle_Circle_Move_Task, "Angle_Circle_Move_Task", 2048, NULL, 14, &AngleCircleMoveTaskHandle);
}

/**
  * @brief  释放Gyroscop界面
  * @param  void
  * @retval void
  */
static void Gui_Gyroscop_Delete(void)
{
    vTaskDelete(AngleCircleMoveTaskHandle);
    lv_obj_del(gui_gyroscop_obj);
    vTaskDelete(TouchKeyTaskHandler);
}

/**
  * @brief  圆心移动任务
  * @param  void
  * @retval void
  */
static void Angle_Circle_Move_Task(void* arg)
{
    float accX=0,accY=0,accZ=0;

    MPU6886Init();
    while(1){
        MPU6886getAccelData(&accX, &accY, &accZ);
        lv_obj_set_pos(angle_circle, ANGLE_CENTRE_X+(accY*100), ANGLE_CENTRE_Y+(accX*100));
        if(accZ < 0){
            lv_style_set_bg_color(&gyroscop_obj_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_report_style_mod(&gyroscop_obj_style);
            lv_style_set_bg_color(&out_circle_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_report_style_mod(&out_circle_style);
        }else{
            lv_style_set_bg_color(&gyroscop_obj_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_report_style_mod(&gyroscop_obj_style);
            lv_style_set_bg_color(&out_circle_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_report_style_mod(&out_circle_style);
        }
        if((accY*100)<10 && (accY*100)>-10 && (accX*100)<10 && (accX*100)>-10){
            lv_style_set_bg_color(&angle_circle_style, LV_STATE_DEFAULT, LV_COLOR_RED);
        }else{
            lv_style_set_bg_color(&angle_circle_style, LV_STATE_DEFAULT, LV_COLOR_CYAN);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
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
            Gui_Gyroscop_Delete();
        }else if(2 == tk_value){
            Gui_Main_Create();
            Gui_Gyroscop_Delete();
        }else if(3 == tk_value){

        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}