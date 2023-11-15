#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_freertos_hooks.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "GUI_C001_sys.h"
#include "gui_sys_time.h"
#include "gui_main_interface.h"

#include "sk6812_driver.h"
#include "i2c_master.h"
#include "ip5306_driver.h"
#include "wifi_driver.h"
#include "speaker_driver.h"
#include "backlight_pwm.h"

/*********************
 *      DEFINES
 *********************/
#define TAG "C001"
#define LV_TICK_PERIOD_MS 1

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void C001_WIFI_Init(void);

Sys_Theme_Num Current_Theme = 0;  
uint8_t Cullent_Backlight = 0;

bool sys_wifi_ap_open_flag = false;             //wifi-ap开关标志
char *sys_wifi_ap_ssid = NULL;                  //wifi-apSSID
char *sys_wifi_ap_psss = NULL;                  //wifi-ap密码

bool sys_wifi_sta_open_flag = false;            //wifi-sta开关标志
char *sys_wifi_sta_ssid = NULL;                 //wifi-staSSID
char *sys_wifi_sta_psss = NULL;                 //wifi-sta密码

/**********************
 *   APPLICATION MAIN
 **********************/
void app_main() 
{
    //初始化NVS
    esp_err_t ret = nvs_flash_init();               //初始化默认的NVS分区
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {     //判断是否初始化成功
      ESP_ERROR_CHECK(nvs_flash_erase());           //如果失败擦除默认的NVS分区，重新初始化      
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //初始化C001wifi信息
    C001_WIFI_Init();

    //播放开机音效
    Speaker_Play_Seartup();

    //让gpio25不发出电流声
    gpio_pad_select_gpio(25);
    gpio_set_direction(25, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(25, GPIO_PULLDOWN_ONLY);
    // gpio_set_level(25, 0);

    /* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to core 0 
     * 如果你想使用一个任务来创建图形，你需要创建一个固定任务
     * 否则可能会出现内存损坏等问题。
     * 注意:当不使用wi-fi和蓝牙可以设置guiTask使用核心0*/
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
}

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! 
 * 创建一个信号量来处理对lvgl的并发调用
 * 如果你想调用任何lvgl函数从其他线程/任务
 * 您应该锁定完全相同的信号量!*/
SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter) 
{
    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers 
       初始化驱动程序使用的SPI或I2C总线*/
    lvgl_driver_init();

    // static lv_color_t buf1[DISP_BUF_SIZE];
    // static lv_color_t buf2[DISP_BUF_SIZE];

    uint8_t lv_color_t_size = sizeof(lv_color_t);
    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE*lv_color_t_size, MALLOC_CAP_DMA);
    lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE*lv_color_t_size, MALLOC_CAP_DMA);

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. 
     * 初始化工作缓冲区根据所选的显示。
     * 注意:使用单色显示时buf2 == NULL*/
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device when enabled on the menuconfig 
     * 在menuconfig上启用时注册输入设备*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
    
    /* Create and start a periodic timer interrupt to call lv_tick_inc 
     * 创建并启动一个定期计时器中断来调用lv_tick_inc*/
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    //初始化背光调节
    Backlight_Pwm_Init();
    Cullent_Backlight = 30;
    Change_Backlight_pwm(Cullent_Backlight);

    //初始化当前主题
    Current_Theme = Light_Theme;

    //初始化系统时间
    GUI_Set_Sys_Time(1970, 1, 1, 0, 0, 0);     

    //初始化rgb灯珠sk6812
    SK6812_Init();      

    //初始化iic1，ip5306用
    I2c_Master_Init(IP5306_I2C_MASTER_NUM, IP5306_I2C_SDA_IO, IP5306_I2C_SCL_IO);

    //显示开机logo
    Gui_BootUp_Create();  

    //创建主界面                          
    Gui_Main_Create();                              
    
    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms 
         * 延迟1滴答假设FreeRTOS滴答是10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success 
         * 尝试获取信号量，成功时调用lvgl相关函数*/
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }
    /* A task should NEVER return */
    vTaskDelete(NULL);
}

static void lv_tick_task(void *arg) 
{
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}


/**
  * @brief  初始化C001wifi信息
  * @param  void
  * @retval void
  */
static void C001_WIFI_Init(void)
{
    //初始化wifi设置
    WIFI_Init();
    sys_wifi_ap_open_flag = false;
    sys_wifi_sta_open_flag = false;

    //初始化默认wifi-ap账号密码
    sys_wifi_ap_ssid = malloc(sizeof(char) * 24);
    bzero(sys_wifi_ap_ssid, 24);
    memcpy(sys_wifi_ap_ssid, "Rubik's Cube-1", 14);
    sys_wifi_ap_psss = malloc(sizeof(char) * 24);
    bzero(sys_wifi_ap_psss, 24);
    memcpy(sys_wifi_ap_psss, "12345678", 8);

    //初始化默认wifi-sta账号密码
    sys_wifi_sta_ssid = malloc(sizeof(char) * 24);
    bzero(sys_wifi_sta_ssid, 24);
    memcpy(sys_wifi_sta_ssid, "------", 6);
    sys_wifi_sta_psss = malloc(sizeof(char) * 24);
    bzero(sys_wifi_sta_psss, 24);
    memcpy(sys_wifi_sta_psss, "------", 6);
}