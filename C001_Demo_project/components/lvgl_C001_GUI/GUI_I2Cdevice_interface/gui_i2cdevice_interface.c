#include "gui_i2cdevice_interface.h"
#include "GUI_C001_sys.h"
#include "gui_main_interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../lvgl_esp32_drivers/lvgl_touch/ft6x36.h"

#include "i2c_master.h"
#include "bmp280_driver.h"
#include "dht12_driver.h"
#include "bh1750fvi_driver.h"
#include "tcs34725_driver.h"
#include "paj7620u2_driver.h"
#include "sgp30_driver.h"

enum{
    I2CDEVICE_LIST_INTER_LEVEL = 0,
    BMP280_MONITOR_INTER_LEVEL = 1,
    DHT12_MONITOR_INTER_LEVEL = 2,
    BH1750FVI_MONITOR_INTER_LEVEL = 3,
    TCS34725_MONITOR_INTER_LEVEL = 4,
    PAJ7620U2_GESTURE_MONITOR_INTER_LEVEL = 5,
    PAJ7620U2_APPROACH_MONITOR_INTER_LEVEL = 6,
    SGP30_MONITOR_INTER_LEVEL = 7,
};
static interface_level_t interface_level = I2CDEVICE_LIST_INTER_LEVEL;

static lv_style_t load_back_style;

static lv_obj_t*  gui_i2cdevice_obj;        //I2Cdevice对象

static lv_obj_t* i2cdevice_list;                //I2Cdevice设备列表（IL）     

static lv_obj_t* IL_bmp280_btn;                     //list中bmp280按钮
static lv_obj_t* IL_dht12_btn;                      //list中dht12按钮
static lv_obj_t* IL_bh1750fvi_btn;                  //list中bh1750fvi按钮
static lv_obj_t* IL_tcs34725_btn;                   //list中tcs34725按钮
static lv_obj_t* IL_paj7620u2_gesture_btn;          //list中paj7620u2 gesture按钮
static lv_obj_t* IL_paj7620u2_approach_btn;         //list中paj7620u2 approach按钮
static lv_obj_t* IL_sgp30_btn;                      //list中sgp30按钮

static lv_obj_t* load_back;                     //“加载设备”动画对象


static TaskHandle_t DeviceLoadTaskHandler;          //加载设备任务句柄
static TaskHandle_t TouchKeyTaskHandler;            //触摸按键任务句柄

static void Device_Load_Task(void* arg);
static void Touch_Key_Task(void* arg);

static void List_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event);

static void Gui_BMP280_Create(void);
static void Gui_DHT12_Create(void);
static void Gui_BH1750FVI_Create(void);
static void Gui_TCS34725_Create(void);
static void Gui_PAJ7620U2_Gesture_Create(void);
static void Gui_PAJ7620U2_Approach_Create(void);
static void Gui_SGP30_Create(void);

/**
  * @brief  创建I2c设备界面
  * @param  void
  * @retval void
  */
void Gui_I2Cdevice_Create(void)
{
    lv_obj_t* screen = lv_scr_act();              //获取屏幕对象

    //创建I2Cdevice对象
    gui_i2cdevice_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_i2cdevice_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建I2Cdevice设备列表 
    i2cdevice_list = lv_list_create(gui_i2cdevice_obj, NULL);
    lv_obj_set_size(i2cdevice_list, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(i2cdevice_list, gui_i2cdevice_obj, LV_ALIGN_IN_TOP_MID, 0, 0);

    //显示“加载设备”动画
    load_back = lv_obj_create(gui_i2cdevice_obj, NULL);
    lv_obj_set_size(load_back, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_style_init(&load_back_style);
    lv_style_set_bg_color(&load_back_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&load_back_style, LV_STATE_DEFAULT, 100);
    lv_obj_add_style(load_back, LV_OBJ_PART_MAIN, &load_back_style);
    lv_obj_t * load_preload = lv_spinner_create(load_back, NULL);
    lv_obj_set_size(load_preload, 100, 100);
    lv_obj_align(load_preload, load_back, LV_ALIGN_CENTER, 0, -20);
    lv_obj_t* load_label = lv_label_create(load_back, NULL);
    lv_label_set_text(load_label, "load device");
    lv_obj_align(load_label, load_preload, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_task_handler();

    //创建设备加载任务
    xTaskCreate(Device_Load_Task, "Device_Load_Task", 2048, NULL, 14, &DeviceLoadTaskHandler);
}

/**
  * @brief  加载I2C设备界面
  * @param  void
  * @retval void
  */
static void Device_Load_Task(void* arg)
{
    //开始检测i2c设备并创建按钮
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if(!I2C_ALIVE(BMP280_ADDR, 0xFF)){
        IL_bmp280_btn = lv_list_add_btn(i2cdevice_list, LV_SYMBOL_RIGHT, "BMP280");
        lv_obj_set_event_cb(IL_bmp280_btn, List_Btn_Event_Handler);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if(!I2C_ALIVE(DHT12_ADDR, 0xFF)){
        IL_dht12_btn = lv_list_add_btn(i2cdevice_list, LV_SYMBOL_RIGHT, "DHT12");
        lv_obj_set_event_cb(IL_dht12_btn, List_Btn_Event_Handler);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if(!I2C_ALIVE(BH1750FVI_ADDR, 0xFF)){
        IL_bh1750fvi_btn = lv_list_add_btn(i2cdevice_list, LV_SYMBOL_RIGHT, "BH1750FVI");
        lv_obj_set_event_cb(IL_bh1750fvi_btn, List_Btn_Event_Handler);
    } 
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if(!I2C_ALIVE(TCS34725_ADDR, 0xFF)){
        IL_tcs34725_btn = lv_list_add_btn(i2cdevice_list, LV_SYMBOL_RIGHT, "TCS34725");
        lv_obj_set_event_cb(IL_tcs34725_btn, List_Btn_Event_Handler);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if(!I2C_ALIVE(PAJ7620U2_ADDR, 0xFF)){
        IL_paj7620u2_gesture_btn = lv_list_add_btn(i2cdevice_list, LV_SYMBOL_RIGHT, "PAJ7620U2-gesture");
        lv_obj_set_event_cb(IL_paj7620u2_gesture_btn, List_Btn_Event_Handler);
        IL_paj7620u2_approach_btn = lv_list_add_btn(i2cdevice_list, LV_SYMBOL_RIGHT, "PAJ7620U2-approach");
        lv_obj_set_event_cb(IL_paj7620u2_approach_btn, List_Btn_Event_Handler);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if(!I2C_ALIVE(SGP30_ADDR, 0xFF)){
        IL_sgp30_btn = lv_list_add_btn(i2cdevice_list, LV_SYMBOL_RIGHT, "SGP30");
        lv_obj_set_event_cb(IL_sgp30_btn, List_Btn_Event_Handler);
    }

    //删除“加载对象”动画对象
    lv_obj_del(load_back);

    //创建触摸按键处理任务
    xTaskCreate(Touch_Key_Task, "Touch_Key_Task", 2048, NULL, 15, &TouchKeyTaskHandler);

    //设置界面层级为I2C设备列表
    interface_level = I2CDEVICE_LIST_INTER_LEVEL;

    //结束任务自身
    vTaskDelete(NULL);
}

/**
  * @brief  释放I2C设备界面
  * @param  void
  * @retval void
  */
static void Gui_I2Cdevice_Delete(void)
{
    lv_obj_del(gui_i2cdevice_obj);
    vTaskDelete(TouchKeyTaskHandler);
}

/**
  * @brief  I2C设备按钮回调函数
  * @param  obj ： 对象
  * @param  event : 事件类型
  * @retval void
  */
static void List_Btn_Event_Handler(lv_obj_t* obj, lv_event_t event)
{
    if(obj == IL_bmp280_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_BMP280_Create();
            interface_level = BMP280_MONITOR_INTER_LEVEL;
        }
    }else if(obj == IL_dht12_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_DHT12_Create();
            interface_level = DHT12_MONITOR_INTER_LEVEL;
        }
    }else if(obj == IL_bh1750fvi_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_BH1750FVI_Create();
            interface_level = BH1750FVI_MONITOR_INTER_LEVEL;
        }
    }else if(obj == IL_tcs34725_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_TCS34725_Create();
            interface_level = TCS34725_MONITOR_INTER_LEVEL;
        }
    }else if(obj == IL_paj7620u2_gesture_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_PAJ7620U2_Gesture_Create();
            interface_level = PAJ7620U2_GESTURE_MONITOR_INTER_LEVEL;
        }
    }else if(obj == IL_paj7620u2_approach_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_PAJ7620U2_Approach_Create();
            interface_level = PAJ7620U2_APPROACH_MONITOR_INTER_LEVEL;
        }
    }else if(obj == IL_sgp30_btn){
        if(event == LV_EVENT_SHORT_CLICKED){
            Gui_SGP30_Create();
            interface_level = SGP30_MONITOR_INTER_LEVEL;
        }
    }
}


static lv_obj_t* gui_bmp280_obj;                    //bmp280对象
static lv_obj_t* bmp280_state_value_label;             //状态数据标签   
static lv_obj_t* bmp280_temperature_value_label;       //温度数据标签 
static lv_obj_t* bmp280_barometric_value_label;        //气压数据标签      
static lv_obj_t* bmp280_elevation_value_label;         //海拔数据标签    

static TaskHandle_t BMP280CollectionTaskHandler;        //BMP280数据采集任务句柄
static void BMP280_Collection_Task(void* arg);

/**
  * @brief  BMP280监视器创建页面
  * @param  void
  * @retval void
  */
static void Gui_BMP280_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建BMP280对象
    gui_bmp280_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_bmp280_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建界面标签
    lv_obj_t* title_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(title_label, "BMP280");
    lv_obj_align(title_label, gui_bmp280_obj, LV_ALIGN_IN_TOP_MID, 0, 10);

    lv_obj_t* state_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(state_label, "state");
    lv_obj_align(state_label, gui_bmp280_obj, LV_ALIGN_IN_TOP_LEFT, 10, 40);
    bmp280_state_value_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(bmp280_state_value_label, "Initialization");
    lv_obj_align(bmp280_state_value_label, gui_bmp280_obj, LV_ALIGN_IN_TOP_MID, 50, 40);

    lv_obj_t* temperature_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(temperature_label, "temperature");
    lv_obj_align(temperature_label, state_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    bmp280_temperature_value_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(bmp280_temperature_value_label, "0.0 C");
    lv_obj_align(bmp280_temperature_value_label, bmp280_state_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* barometric_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(barometric_label, "barometric");
    lv_obj_align(barometric_label, temperature_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    bmp280_barometric_value_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(bmp280_barometric_value_label, "0.0 hPa");
    lv_obj_align(bmp280_barometric_value_label, bmp280_temperature_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* elevation_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(elevation_label, "elevation");
    lv_obj_align(elevation_label, barometric_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    bmp280_elevation_value_label = lv_label_create(gui_bmp280_obj, NULL);
    lv_label_set_text(bmp280_elevation_value_label, "0.0 m");
    lv_obj_align(bmp280_elevation_value_label, bmp280_barometric_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建BMP280数据采集任务
    xTaskCreate(BMP280_Collection_Task, "BMP280_Collection_Task", 2048, NULL, 13, &BMP280CollectionTaskHandler);
}

/**
  * @brief  BMP280采集处理函数
  * @param  void
  * @retval void
  */
static void BMP280_Collection_Task(void* arg)
{
    float P=0,T=0,ALT=0;
    char str_val[4][20];
    Bmp280_Init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(str_val[0], "2s/Collect");
    lv_label_set_text(bmp280_state_value_label, str_val[0]);
    while(1){
        Bmp280GetData(&P, &T, &ALT);
        sprintf(str_val[1], "%0.2f C", T);
        sprintf(str_val[2], "%0.4f hPa", P);
        sprintf(str_val[3], "%0.2f m", ALT);
        lv_label_set_text(bmp280_temperature_value_label, str_val[1]);
        lv_label_set_text(bmp280_barometric_value_label, str_val[2]);
        lv_label_set_text(bmp280_elevation_value_label, str_val[3]);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static lv_obj_t* gui_dht12_obj;                 //dht12对象
static lv_obj_t* dht12_state_value_label;           //状态数据标签
static lv_obj_t* dht12_temperature_value_label;     //温度数据标签 
static lv_obj_t* dht12_humidity_value_label;        //适度数据标签

static TaskHandle_t DHT12CollectionTaskHandler;        //BMP280数据采集任务句柄
static void DHT12_Collection_Task(void* arg);

/**
  * @brief  DHT12监视器页面创建
  * @param  void
  * @retval void
  */
static void Gui_DHT12_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建BMP280对象
    gui_dht12_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_dht12_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建界面标签
    lv_obj_t* title_label = lv_label_create(gui_dht12_obj, NULL);
    lv_label_set_text(title_label, "DHT12");
    lv_obj_align(title_label, gui_dht12_obj, LV_ALIGN_IN_TOP_MID, 0, 10);

    lv_obj_t* state_label = lv_label_create(gui_dht12_obj, NULL);
    lv_label_set_text(state_label, "state");
    lv_obj_align(state_label, gui_dht12_obj, LV_ALIGN_IN_TOP_LEFT, 10, 40);
    dht12_state_value_label = lv_label_create(gui_dht12_obj, NULL);
    lv_label_set_text(dht12_state_value_label, "Initialization");
    lv_obj_align(dht12_state_value_label, gui_dht12_obj, LV_ALIGN_IN_TOP_MID, 50, 40);

    lv_obj_t* temperature_label = lv_label_create(gui_dht12_obj, NULL);
    lv_label_set_text(temperature_label, "temperature");
    lv_obj_align(temperature_label, state_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    dht12_temperature_value_label = lv_label_create(gui_dht12_obj, NULL);
    lv_label_set_text(dht12_temperature_value_label, "0.0 C");
    lv_obj_align(dht12_temperature_value_label, dht12_state_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* humidity_label = lv_label_create(gui_dht12_obj, NULL);
    lv_label_set_text(humidity_label, "humidity");
    lv_obj_align(humidity_label, temperature_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    dht12_humidity_value_label = lv_label_create(gui_dht12_obj, NULL);
    lv_label_set_text(dht12_humidity_value_label, "0.0 RH");
    lv_obj_align(dht12_humidity_value_label, dht12_temperature_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建DHT12数据采集任务
    xTaskCreate(DHT12_Collection_Task, "DHT12_Collection_Task", 2048, NULL, 13, &DHT12CollectionTaskHandler);
}

/**
  * @brief  DHT12采集处理任务
  * @param  void
  * @retval void
  */
static void DHT12_Collection_Task(void* arg)
{
    float Temperature=0,Humi=0;
    char str_val[3][20];
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(str_val[0], "1s/Collect");
    lv_label_set_text(dht12_state_value_label, str_val[0]);
    while(1){
        DHT12_Get_Val(&Temperature, &Humi);
        sprintf(str_val[1], "%0.1f C", Temperature);
        sprintf(str_val[2], "%0.1f RH", Humi);
        lv_label_set_text(dht12_temperature_value_label, str_val[1]);
        lv_label_set_text(dht12_humidity_value_label, str_val[2]);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static lv_obj_t* gui_bh1750fvi_obj;             //bh1750fvi对象
static lv_obj_t* bh1750fvi_state_value_label;       //状态数据标签
static lv_obj_t* bh1750fvi_light_value_label;       //bh1750fvi光照强度数值

static TaskHandle_t BH1750FVICollectionTaskHandler;        //BH1750FVI数据采集任务句柄
static void BH1750FVI_Collection_Task(void* arg);

/**
  * @brief  BH1750FVI监视器页面创建
  * @param  void
  * @retval void
  */
static void Gui_BH1750FVI_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建BH1750FVI对象
    gui_bh1750fvi_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_bh1750fvi_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建界面标签
    lv_obj_t* title_label = lv_label_create(gui_bh1750fvi_obj, NULL);
    lv_label_set_text(title_label, "BH1750FVI");
    lv_obj_align(title_label, gui_bh1750fvi_obj, LV_ALIGN_IN_TOP_MID, 0, 10);

    lv_obj_t* state_label = lv_label_create(gui_bh1750fvi_obj, NULL);
    lv_label_set_text(state_label, "state");
    lv_obj_align(state_label, gui_bh1750fvi_obj, LV_ALIGN_IN_TOP_LEFT, 10, 40);
    bh1750fvi_state_value_label = lv_label_create(gui_bh1750fvi_obj, NULL);
    lv_label_set_text(bh1750fvi_state_value_label, "Initialization");
    lv_obj_align(bh1750fvi_state_value_label, gui_bh1750fvi_obj, LV_ALIGN_IN_TOP_MID, 50, 40);

    lv_obj_t* light_label = lv_label_create(gui_bh1750fvi_obj, NULL);
    lv_label_set_text(light_label, "light");
    lv_obj_align(light_label, state_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    bh1750fvi_light_value_label = lv_label_create(gui_bh1750fvi_obj, NULL);
    lv_label_set_text(bh1750fvi_light_value_label, "0.0 lx");
    lv_obj_align(bh1750fvi_light_value_label, bh1750fvi_state_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建BH1750FVI数据采集任务
    xTaskCreate(BH1750FVI_Collection_Task, "BH1750FVI_Collection_Task", 2048, NULL, 13, &BH1750FVICollectionTaskHandler);
}

/**
  * @brief  BH1750FVI采集处理任务
  * @param  void
  * @retval void
  */
static void BH1750FVI_Collection_Task(void* arg)
{
    float lx_data;
    char str_val[2][20];
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(str_val[0], "1s/Collect");
    lv_label_set_text(bh1750fvi_state_value_label, str_val[0]);
    while(1){
        lx_data = BH1750FVI_Read_Data();
        sprintf(str_val[1], "%0.1f lx", lx_data);
        lv_label_set_text(bh1750fvi_light_value_label, str_val[1]);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static lv_style_t tcs34725_color_block_style;

static lv_obj_t* gui_tcs34725_obj;             //tcs34725对象
static lv_obj_t* tcs34725_state_value_label;        //状态数据标签
static lv_obj_t* tcs34725_red_value_label;          //tcs34725红色数值标签
static lv_obj_t* tcs34725_green_value_label;        //tcs34725绿色数值标签
static lv_obj_t* tcs34725_blue_value_label;         //tcs34725蓝色数值标签
static lv_obj_t* tcs34725_clear_value_label;        //tcs34725透明度数值标签
static lv_obj_t* tcs34725_color_block;              //tcs34725颜色块

static TaskHandle_t TCS34725CollectionTaskHandler;        //TCS34725数据采集任务句柄
static void TCS34725_Collection_Task(void* arg);

/**
  * @brief  TCS34725监视器页面创建
  * @param  void
  * @retval void
  */
static void Gui_TCS34725_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建TCS34725对象
    gui_tcs34725_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_tcs34725_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建界面标签
    lv_obj_t* title_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(title_label, "TCS34725");
    lv_obj_align(title_label, gui_tcs34725_obj, LV_ALIGN_IN_TOP_MID, 0, 10);

    lv_obj_t* state_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(state_label, "state");
    lv_obj_align(state_label, gui_tcs34725_obj, LV_ALIGN_IN_TOP_LEFT, 10, 40);
    tcs34725_state_value_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(tcs34725_state_value_label, "Initialization");
    lv_obj_align(tcs34725_state_value_label, gui_tcs34725_obj, LV_ALIGN_IN_TOP_MID, 50, 40);

    lv_obj_t* red_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(red_label, "red");
    lv_obj_align(red_label, state_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    tcs34725_red_value_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(tcs34725_red_value_label, "0");
    lv_obj_align(tcs34725_red_value_label, tcs34725_state_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* green_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(green_label, "green");
    lv_obj_align(green_label, red_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    tcs34725_green_value_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(tcs34725_green_value_label, "0");
    lv_obj_align(tcs34725_green_value_label, tcs34725_red_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* blue_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(blue_label, "blue");
    lv_obj_align(blue_label, green_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    tcs34725_blue_value_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(tcs34725_blue_value_label, "0");
    lv_obj_align(tcs34725_blue_value_label, tcs34725_green_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* clear_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(clear_label, "clear");
    lv_obj_align(clear_label, blue_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    tcs34725_clear_value_label = lv_label_create(gui_tcs34725_obj, NULL);
    lv_label_set_text(tcs34725_clear_value_label, "0");
    lv_obj_align(tcs34725_clear_value_label, tcs34725_blue_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    tcs34725_color_block = lv_obj_create(gui_tcs34725_obj, NULL);
    lv_obj_set_size(tcs34725_color_block, 280, 50);
    lv_obj_align(tcs34725_color_block, gui_tcs34725_obj, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_style_init(&tcs34725_color_block_style);
    lv_style_set_bg_color(&tcs34725_color_block_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(tcs34725_color_block, LV_OBJ_PART_MAIN, &tcs34725_color_block_style);

    //创建TCS34725数据采集任务
    xTaskCreate(TCS34725_Collection_Task, "TCS34725_Collection_Task", 2048, NULL, 13, &TCS34725CollectionTaskHandler);
}

/**
  * @brief  TCS34725采集处理任务
  * @param  void
  * @retval void
  */
static void TCS34725_Collection_Task(void* arg)
{
    uint16_t red=0, green=0, blue=0, clear=0;
    char str_val[6][20];
    uint32_t block_color = 0;
    
    //初始化TCS34725
    TCS34725_SetIntegrationTime(TCS34725_INTEGRATIONTIME_240MS);
    TCS34725_SetGain(TCS34725_GAIN_4X);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    sprintf(str_val[0], "1s/Collect");
    lv_label_set_text(tcs34725_state_value_label, str_val[0]);
    while(1){
        if(TCS34725_GetRawData(&red, &green, &blue, &clear)){
            sprintf(str_val[1], "%d", red);
            sprintf(str_val[2], "%d", green);
            sprintf(str_val[3], "%d", blue);
            sprintf(str_val[4], "%d", clear);
        }
        lv_label_set_text(tcs34725_red_value_label, str_val[1]);
        lv_label_set_text(tcs34725_green_value_label, str_val[2]);
        lv_label_set_text(tcs34725_blue_value_label, str_val[3]);
        lv_label_set_text(tcs34725_clear_value_label, str_val[4]);
        
        //颜色增强算法
        if(red >= green && red >= blue){
            red *= 1.5;
            if(green >= blue){
                green *= 1.2;
                blue *= 0.8;
            }else{
                blue *= 1.2;
                green *= 0.8;  
            }
        }else if(green > red && green > blue){
            green *= 1.5;
            if(red > blue){
                red *= 1.2;
                blue *= 0.8;
            }else{
                blue *= 1.2;
                red *= 0.8;
            }
        }else if(blue > red && blue > green){
            blue *= 1.5;
            if(red > green){
                red *= 1.2;
                green *= 0.8;
            }else{
                green *= 1.2;
                red *= 0.8;
            }
        }
        block_color = (red/256) * 2;
        block_color = ((block_color << 8) + green/256) * 2;
        block_color = ((block_color << 8) + blue/256) * 2;

        lv_style_set_bg_color(&tcs34725_color_block_style, LV_STATE_DEFAULT, lv_color_hex(block_color));
        lv_obj_refresh_style(tcs34725_color_block, LV_OBJ_PART_ALL, LV_STYLE_PROP_ALL);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static lv_obj_t* gui_paj7620u2_gesture_obj;         //paj7620u2 gesture对象
static lv_obj_t* paj7620u2_gesture_state_value_label;   //状态数据标签
static lv_obj_t* paj7620u2_gesture_gesture_value_label; //paj7620u2 gesture手势状态数据标签

static TaskHandle_t PAJ7620U2GestureCollectionTaskHandler;        //paj7620u2 gesture数据采集任务句柄
static void PAJ7620U2_Gesture_Collection_Task(void* arg);

/**
  * @brief  PAJ7620U2-gesture监视器页面创建
  * @param  void
  * @retval void
  */
static void Gui_PAJ7620U2_Gesture_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建PAJ7620U2 gesture对象
    gui_paj7620u2_gesture_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_paj7620u2_gesture_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建界面标签
    lv_obj_t* title_label = lv_label_create(gui_paj7620u2_gesture_obj, NULL);
    lv_label_set_text(title_label, "PAJ7620U2-gesture");
    lv_obj_align(title_label, gui_paj7620u2_gesture_obj, LV_ALIGN_IN_TOP_MID, 0, 10);

    lv_obj_t* state_label = lv_label_create(gui_paj7620u2_gesture_obj, NULL);
    lv_label_set_text(state_label, "state");
    lv_obj_align(state_label, gui_paj7620u2_gesture_obj, LV_ALIGN_IN_TOP_LEFT, 10, 40);
    paj7620u2_gesture_state_value_label = lv_label_create(gui_paj7620u2_gesture_obj, NULL);
    lv_label_set_text(paj7620u2_gesture_state_value_label, "Initialization");
    lv_obj_align(paj7620u2_gesture_state_value_label, gui_paj7620u2_gesture_obj, LV_ALIGN_IN_TOP_MID, 50, 40);

    lv_obj_t* gesture_label = lv_label_create(gui_paj7620u2_gesture_obj, NULL);
    lv_label_set_text(gesture_label, "gesture");
    lv_obj_align(gesture_label, state_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    paj7620u2_gesture_gesture_value_label = lv_label_create(gui_paj7620u2_gesture_obj, NULL);
    lv_label_set_text(paj7620u2_gesture_gesture_value_label, "---");
    lv_obj_align(paj7620u2_gesture_gesture_value_label, paj7620u2_gesture_state_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建PAJ7620U2 gesture数据采集任务
    xTaskCreate(PAJ7620U2_Gesture_Collection_Task, "PAJ7620U2_Gesture_Collection_Task", 2048, NULL, 13, &PAJ7620U2GestureCollectionTaskHandler);
}

/**
  * @brief  PAJ7620U2-gesture采集处理任务
  * @param  void
  * @retval void
  */
static void PAJ7620U2_Gesture_Collection_Task(void* arg)
{
    uint16_t gesture_status=0;
    char str_val[2][20];

    PAJ7620U2_init();
    PAJ7620U2_Gesture_Init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(str_val[0], "500ms/Collect");
    lv_label_set_text(paj7620u2_gesture_state_value_label, str_val[0]);
    while(1){
        PAJ7620U2_Get_Gesture(&gesture_status);
         switch(gesture_status){
            case GESTURE_UP:                            //向上
                sprintf(str_val[1], "Up");
                break; 
			case GESTURE_DOWM:                          //向下
                sprintf(str_val[1], "Dowm");
                break; 
			case GESTURE_LEFT:                          //向左
                sprintf(str_val[1], "Left");
                break; 
			case GESTURE_RIGHT:                         //向右 
                sprintf(str_val[1], "Right");
                break; 
			case GESTURE_FORWARD:                       //向前 
                sprintf(str_val[1], "Forward");
                break; 
	    	case GESTURE_BACKWARD:                      //向后
                sprintf(str_val[1], "Backward");
                  break; 
	    	case GESTURE_CLOCKWISE:                     //顺时针
                sprintf(str_val[1], "Clockwise");
                break; 
			case GESTURE_COUNT_CLOCKWISE:               //逆时针
                sprintf(str_val[1], "AntiClockwise");
                break; 
	    	case GESTURE_WAVE:                          //挥动
                sprintf(str_val[1], "Wave");
                break; 
			default:
                break; 
        }
        lv_label_set_text(paj7620u2_gesture_gesture_value_label, str_val[1]);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

static lv_obj_t* gui_paj7620u2_approach_obj;            //paj7620u2 approach对象
static lv_obj_t* paj7620u2_approach_state_value_label;      //状态数据标签
static lv_obj_t* paj7620u2_approach_brightness_value_label; //paj7620u2 approach物体亮度数据
static lv_obj_t* paj7620u2_approach_size_value_label;       //paj7620u2 approach物体大小数据

static TaskHandle_t PAJ7620U2ApproachCollectionTaskHandler;        //paj7620u2 approach数据采集任务句柄
static void PAJ7620U2_Approach_Collection_Task(void* arg);

/**
  * @brief  PAJ7620U2-approach监视器页面创建
  * @param  void
  * @retval void
  */
static void Gui_PAJ7620U2_Approach_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建PAJ7620U2 approach对象
    gui_paj7620u2_approach_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_paj7620u2_approach_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建界面标签
    lv_obj_t* title_label = lv_label_create(gui_paj7620u2_approach_obj, NULL);
    lv_label_set_text(title_label, "PAJ7620U2-approach");
    lv_obj_align(title_label, gui_paj7620u2_approach_obj, LV_ALIGN_IN_TOP_MID, 0, 10);

    lv_obj_t* state_label = lv_label_create(gui_paj7620u2_approach_obj, NULL);
    lv_label_set_text(state_label, "state");
    lv_obj_align(state_label, gui_paj7620u2_approach_obj, LV_ALIGN_IN_TOP_LEFT, 10, 40);
    paj7620u2_approach_state_value_label = lv_label_create(gui_paj7620u2_approach_obj, NULL);
    lv_label_set_text(paj7620u2_approach_state_value_label, "Initialization");
    lv_obj_align(paj7620u2_approach_state_value_label, gui_paj7620u2_approach_obj, LV_ALIGN_IN_TOP_MID, 50, 40);

    lv_obj_t* brightness_label = lv_label_create(gui_paj7620u2_approach_obj, NULL);
    lv_label_set_text(brightness_label, "obj brightness");
    lv_obj_align(brightness_label, state_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    paj7620u2_approach_brightness_value_label = lv_label_create(gui_paj7620u2_approach_obj, NULL);
    lv_label_set_text(paj7620u2_approach_brightness_value_label, "0");
    lv_obj_align(paj7620u2_approach_brightness_value_label, paj7620u2_approach_state_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* size_label = lv_label_create(gui_paj7620u2_approach_obj, NULL);
    lv_label_set_text(size_label, "obj size");
    lv_obj_align(size_label, brightness_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    paj7620u2_approach_size_value_label = lv_label_create(gui_paj7620u2_approach_obj, NULL);
    lv_label_set_text(paj7620u2_approach_size_value_label, "0");
    lv_obj_align(paj7620u2_approach_size_value_label, paj7620u2_approach_brightness_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建PAJ7620U2 approach数据采集任务
    xTaskCreate(PAJ7620U2_Approach_Collection_Task, "PAJ7620U2_Approach_Collection_Task", 2048, NULL, 13, &PAJ7620U2ApproachCollectionTaskHandler);
}

/**
  * @brief  PAJ7620U2-approach采集处理任务
  * @param  void
  * @retval void
  */
static void PAJ7620U2_Approach_Collection_Task(void* arg)
{
    uint8_t obj_brightness=0;
    uint16_t obj_size=0;
    char str_val[3][20];

    PAJ7620U2_init();
    PAJ7620U2_Approach_Init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(str_val[0], "500ms/Collect");
    lv_label_set_text(paj7620u2_approach_state_value_label, str_val[0]);
    while(1){
        PAJ7620U2_Get_Approach(&obj_brightness, &obj_size);
        sprintf(str_val[1], "%d", obj_brightness);
        lv_label_set_text(paj7620u2_approach_brightness_value_label, str_val[1]);
        sprintf(str_val[2], "%d", obj_size);
        lv_label_set_text(paj7620u2_approach_size_value_label, str_val[2]);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

static lv_obj_t* gui_sgp30_obj;                 //sgp30对象
static lv_obj_t* sgp30_state_value_label;           //状态数据标签
static lv_obj_t* sgp30_co2_value_label;             //二氧化碳数据标签 
static lv_obj_t* sgp30_tvoc_value_label;            //甲醛数据标签

static TaskHandle_t SGP30CollectionTaskHandler;        //SGP30数据采集任务句柄
static void SGP30_Collection_Task(void* arg);

/**
  * @brief  SGP30监视器页面创建
  * @param  void
  * @retval void
  */
static void Gui_SGP30_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建SGP30对象
    gui_sgp30_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_sgp30_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //创建界面标签
    lv_obj_t* title_label = lv_label_create(gui_sgp30_obj, NULL);
    lv_label_set_text(title_label, "SGP30");
    lv_obj_align(title_label, gui_sgp30_obj, LV_ALIGN_IN_TOP_MID, 0, 10);

    lv_obj_t* state_label = lv_label_create(gui_sgp30_obj, NULL);
    lv_label_set_text(state_label, "state");
    lv_obj_align(state_label, gui_sgp30_obj, LV_ALIGN_IN_TOP_LEFT, 10, 40);
    sgp30_state_value_label = lv_label_create(gui_sgp30_obj, NULL);
    lv_label_set_text(sgp30_state_value_label, "Initialization");
    lv_obj_align(sgp30_state_value_label, gui_sgp30_obj, LV_ALIGN_IN_TOP_MID, 50, 40);

    lv_obj_t* co2_label = lv_label_create(gui_sgp30_obj, NULL);
    lv_label_set_text(co2_label, "CO2");
    lv_obj_align(co2_label, state_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    sgp30_co2_value_label = lv_label_create(gui_sgp30_obj, NULL);
    lv_label_set_text(sgp30_co2_value_label, "0 ppm");
    lv_obj_align(sgp30_co2_value_label, sgp30_state_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t* tvoc_label = lv_label_create(gui_sgp30_obj, NULL);
    lv_label_set_text(tvoc_label, "tvoc");
    lv_obj_align(tvoc_label, co2_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    sgp30_tvoc_value_label = lv_label_create(gui_sgp30_obj, NULL);
    lv_label_set_text(sgp30_tvoc_value_label, "0 ppd");
    lv_obj_align(sgp30_tvoc_value_label, sgp30_co2_value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    //创建SGP30数据采集任务
    xTaskCreate(SGP30_Collection_Task, "SGP30_Collection_Task", 2048, NULL, 13, &SGP30CollectionTaskHandler);
}

/**
  * @brief  sgp30采集处理任务
  * @param  void
  * @retval void
  */
static void SGP30_Collection_Task(void* arg)
{
    uint16_t co2=0,tvoc=0;
    char str_val[3][20];

    SGP30_Init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sprintf(str_val[0], "1s/Collect");
    lv_label_set_text(sgp30_state_value_label, str_val[0]);
    while(1){
        SGP30_Get_Value(&co2, &tvoc);
        sprintf(str_val[1], "%d ppm", co2);
        lv_label_set_text(sgp30_co2_value_label, str_val[1]);
        sprintf(str_val[2], "%d ppd", tvoc);
        lv_label_set_text(sgp30_tvoc_value_label, str_val[2]);
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
    while(1){
        tk_value = ft6x36_get_touch_key_value();

        if(interface_level == I2CDEVICE_LIST_INTER_LEVEL){
            if(1 == tk_value){
                Gui_Main_Create();
                Gui_I2Cdevice_Delete();
            }else if(2 == tk_value){
                Gui_Main_Create();
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }else if(interface_level == BMP280_MONITOR_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(gui_bmp280_obj);
                vTaskDelete(BMP280CollectionTaskHandler);
                interface_level = I2CDEVICE_LIST_INTER_LEVEL;
            }else if(2 == tk_value){
                Gui_Main_Create();
                vTaskDelete(BMP280CollectionTaskHandler);
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }else if(interface_level == DHT12_MONITOR_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(gui_dht12_obj);
                vTaskDelete(DHT12CollectionTaskHandler);
                interface_level = I2CDEVICE_LIST_INTER_LEVEL;
            }else if(2 == tk_value){
                Gui_Main_Create();
                vTaskDelete(DHT12CollectionTaskHandler);
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }else if(interface_level == BH1750FVI_MONITOR_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(gui_bh1750fvi_obj);
                vTaskDelete(BH1750FVICollectionTaskHandler);
                interface_level = I2CDEVICE_LIST_INTER_LEVEL;
            }else if(2 == tk_value){
                Gui_Main_Create();
                vTaskDelete(BH1750FVICollectionTaskHandler);
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }else if(interface_level == TCS34725_MONITOR_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(gui_tcs34725_obj);
                vTaskDelete(TCS34725CollectionTaskHandler);
                interface_level = I2CDEVICE_LIST_INTER_LEVEL;
            }else if(2 == tk_value){
                Gui_Main_Create();
                vTaskDelete(TCS34725CollectionTaskHandler);
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }else if(interface_level == PAJ7620U2_GESTURE_MONITOR_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(gui_paj7620u2_gesture_obj);
                vTaskDelete(PAJ7620U2GestureCollectionTaskHandler);
                interface_level = I2CDEVICE_LIST_INTER_LEVEL;
            }else if(2 == tk_value){
                Gui_Main_Create();
                vTaskDelete(PAJ7620U2GestureCollectionTaskHandler);
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }else if(interface_level == PAJ7620U2_APPROACH_MONITOR_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(gui_paj7620u2_approach_obj);
                vTaskDelete(PAJ7620U2ApproachCollectionTaskHandler);
                interface_level = I2CDEVICE_LIST_INTER_LEVEL;
            }else if(2 == tk_value){
                Gui_Main_Create();
                vTaskDelete(PAJ7620U2ApproachCollectionTaskHandler);
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }else if(interface_level == SGP30_MONITOR_INTER_LEVEL){
            if(1 == tk_value){
                lv_obj_del(gui_sgp30_obj);
                vTaskDelete(SGP30CollectionTaskHandler);
                interface_level = I2CDEVICE_LIST_INTER_LEVEL;
            }else if(2 == tk_value){
                Gui_Main_Create();
                vTaskDelete(SGP30CollectionTaskHandler);
                Gui_I2Cdevice_Delete();
            }else if(3 == tk_value){

            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}




