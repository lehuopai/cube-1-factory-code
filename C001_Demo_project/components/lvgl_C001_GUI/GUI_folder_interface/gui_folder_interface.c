#include "gui_folder_interface.h"
#include "GUI_C001_sys.h"
#include "gui_main_interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../lvgl_esp32_drivers/lvgl_touch/ft6x36.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "../sd_spi_vfs_fat/include/sd_spi_vfs.h"
#include "esp_err.h"

enum {
    INIT_FAIL_INTER_LEVEL = 0,
    FILE_LIST_INTER_LEVEL = 1,
};
static interface_level_t interface_level = 255;

static char* current_path;

static uint8_t current_dir_level = 0;
static lv_obj_t** Dir_List_obj = NULL;

static lv_style_t initfail_back_style;  //初始化失败背景风格

static lv_obj_t* gui_folder_obj;        //folder对象

static lv_obj_t* initfail_back;             //sd卡初始化错误对象(IB)

static lv_obj_t* IB_fail_msgbox;                //错误信息消息盒子

static TaskHandle_t TouchKeyTaskHandler;            //触摸按键任务句柄

static void Create_Fail_Message(void);
static void Create_File_List(char* dir_path);
static void Gui_Folder_Delete(void);
static void Recover_TFT_SPI(void);

static void Touch_Key_Task(void* arg);

/**
  * @brief  创建文件夹界面
  * @param  void
  * @retval void
  */
void Gui_folder_Create(void)
{
    lv_obj_t* screen = lv_scr_act();

    //创建folder对象
    gui_folder_obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(gui_folder_obj, SCREEN_WIDTH, SCREEN_HEIGHT);

    //初始化当前路径
    current_path = malloc(sizeof(char) * 256);
    memset(current_path, 0, sizeof(*current_path));
    strcpy(current_path, SD_CARD_PATH);

    //为目录列表数组分配最大所需内存
    Dir_List_obj = malloc(sizeof(lv_obj_t*) * SD_DIR_MAX_LEVEL);
    current_dir_level = 0;

    //创建根目录文件列表
    Create_File_List(current_path);

    //创建触摸按键处理任务
    xTaskCreate(Touch_Key_Task, "Touch_Key_Task", 2048, NULL, 15, &TouchKeyTaskHandler);
}

static const char fail_msg_info[] = "ERROR\n  SD/TF card detection failed. Make sure the card is installed correctly.";
static const char* use_message_btn[] = {"OK",""};
static void Use_Message_event_cb(lv_obj_t* obj, lv_event_t event);
/**
  * @brief  创建sd卡初始化失败提示框
  * @param  void
  * @retval void
  */
static void Create_Fail_Message(void)
{   
    interface_level = INIT_FAIL_INTER_LEVEL;
    //初始化错误信息消息盒子背景风格
    lv_style_init(&initfail_back_style);
    lv_style_set_bg_color(&initfail_back_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&initfail_back_style, LV_STATE_DEFAULT, 200);
    
    //创建错误信息消息盒子背景
    initfail_back = lv_obj_create(gui_folder_obj, NULL);
    lv_obj_set_size(initfail_back, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_add_style(initfail_back, LV_OBJ_PART_MAIN, &initfail_back_style);

    //创建错误信息消息盒子
    IB_fail_msgbox = lv_msgbox_create(initfail_back, NULL);
    lv_obj_set_width(IB_fail_msgbox,SCREEN_HEIGHT);                                     
    lv_msgbox_set_text(IB_fail_msgbox, fail_msg_info);
    lv_msgbox_add_btns(IB_fail_msgbox,use_message_btn);
    lv_obj_align(IB_fail_msgbox, initfail_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(IB_fail_msgbox, Use_Message_event_cb);
}

/**
  * @brief  错误信息消息框的事件回调函数
  * @param  obj：lv对象
  * @param  event：事件类型
  * @retval void
  */
static void Use_Message_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if(obj == IB_fail_msgbox){
        if(event == LV_EVENT_DELETE){
            lv_obj_del_async(initfail_back);
        }else if(event == LV_EVENT_VALUE_CHANGED){
            lv_msgbox_start_auto_close(IB_fail_msgbox, 0);
        }
    }
}


static void Open_Dir_event_cb(lv_obj_t* obj, lv_event_t event);
/**
  * @brief  创建文件列表
  * @param  dir_path : 目录路径
  * @retval void
  */
static void Create_File_List(char* dir_path)
{
    DIR* dir = NULL;
    uint8_t dir_num = 0;
    struct dirent* ptr = NULL;
    lv_obj_t*  file_btn[SD_LIST_MAX_FILE];
    
    interface_level = FILE_LIST_INTER_LEVEL;

    //初始化sd卡
    if(ESP_FAIL == SD_SPI_VFS_Init(SD_CARD_PATH, false)){
        Recover_TFT_SPI();
        Create_Fail_Message();
        return;
    }

    //创建目录对象
    Dir_List_obj[current_dir_level] = lv_obj_create(gui_folder_obj, NULL);
    lv_obj_set_size(Dir_List_obj[current_dir_level], SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(Dir_List_obj[current_dir_level], NULL, LV_ALIGN_CENTER, 0, 0);
    
    //创建路径标签
    lv_obj_t* path_label = lv_label_create(Dir_List_obj[current_dir_level], NULL);
    lv_label_set_long_mode(path_label, LV_LABEL_LONG_SROLL);
    lv_obj_set_width(path_label, SCREEN_WIDTH-40);
    lv_obj_align(path_label, Dir_List_obj[current_dir_level], LV_ALIGN_IN_TOP_LEFT, 20, 12);
    lv_label_set_text(path_label, dir_path);
    
    //创建文件目录列表
    lv_obj_t* directory_list = lv_list_create(Dir_List_obj[current_dir_level], NULL);
    lv_obj_set_size(directory_list, SCREEN_WIDTH, 200);
    lv_obj_align(directory_list, Dir_List_obj[current_dir_level], LV_ALIGN_IN_TOP_MID, 0, 40);

    //创建文件目录列表
    dir = opendir(current_path);
    while((ptr = readdir(dir)) != NULL && dir_num <= SD_LIST_MAX_FILE-1){
        if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..") == 0 || strcmp(ptr->d_name,"SYSTEM~1") == 0){   
            continue;
        }
        if(ptr->d_type == DT_DIR){
            file_btn[dir_num] = lv_list_add_btn(directory_list, LV_SYMBOL_RIGHT, ptr->d_name);
            lv_obj_set_event_cb(file_btn[dir_num], Open_Dir_event_cb);
        }else{
            file_btn[dir_num] = lv_list_add_btn(directory_list, LV_SYMBOL_FILE, ptr->d_name);
        }
        dir_num++;
    }
    closedir(dir);

    //卸载sd卡
    esp_vfs_fat_sdmmc_unmount();
    
    //恢复TFTspi
    Recover_TFT_SPI();

    //目录层级加1
    current_dir_level++;
}

/**
  * @brief  打开目录文件事件回调函数
  * @param  obj ：对象
  * @param  event ：事件类型
  * @retval void
  */
static void Open_Dir_event_cb(lv_obj_t* obj, lv_event_t event)
{
    if(event == LV_EVENT_SHORT_CLICKED){
        if(current_dir_level >= SD_DIR_MAX_LEVEL)
            return;
        strcat(current_path, "/");
        strcat(current_path, lv_list_get_btn_text(obj));
        Create_File_List(current_path);
    }
}

/**
  * @brief  释放文件夹页面所有资源
  * @param  void
  * @retval void
  */
static void Gui_Folder_Delete(void)
{
    free(current_path);
    free(Dir_List_obj);
    lv_obj_del(gui_folder_obj);
    vTaskDelete(TouchKeyTaskHandler);
}

/**
  * @brief  获取上一级目录的菜单
  * @param  path : 路径
  * @retval void
  */
static void Get_Before_Path(char* path)
{
    uint32_t last = strlen(path) - 1; 
    while(path[last] != '/'){
        path[last] = '\0';
        last--;
    }
    path[last] = '\0';
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
        if(interface_level == INIT_FAIL_INTER_LEVEL){
            if(1 == tk_value){
                    Gui_Main_Create();
                    Gui_Folder_Delete();
            }else if(2 == tk_value){
                Gui_Main_Create();
                Gui_Folder_Delete();
            }
        }else if(interface_level == FILE_LIST_INTER_LEVEL){
            if(1 == tk_value){
                if(current_dir_level == 1){
                    Gui_Main_Create();
                    Gui_Folder_Delete();
                }
                lv_obj_del(Dir_List_obj[current_dir_level-1]);
                current_dir_level--;
                Get_Before_Path(current_path);
            }else if(2 == tk_value){ 
                Gui_Main_Create();
                Gui_Folder_Delete();
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/**
  * @brief  恢复液晶屏的spi
  * @param  void
  * @retval void
  */
static void Recover_TFT_SPI(void)
{
    // gpio_iomux_in(PIN_NUM_MOSI, 65);
    gpio_iomux_out(PIN_NUM_MOSI, 1, false);
    // gpio_iomux_in(PIN_NUM_CLK, 63);
    gpio_iomux_out(PIN_NUM_CLK, 1, false);
}