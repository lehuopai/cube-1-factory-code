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
#include "wifi_driver.h"
#include "esp_smartconfig.h"
#include "GUI_C001_sys.h"

static char TAG[]="WIFI message";               //打印标签

static int s_retry_num = 0;

static esp_netif_t *netif_ap = NULL;
static esp_netif_t *netif_sta = NULL;

static bool sta_connect = false;

/**
  * @brief  wifi事件处理回调函数
  * @param  arg:传入参数，此处为NULL
  * @param  event_base:指向公开事件的子系统的唯一指针
  * @param  event_id:事件ID，查看esp_wifi_types.h
  * @param  event_data:事件数据
  * @retval void
  * @note   根据wifi的连接事件设置事件组标志位
  */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {               //AP-设备连接事件
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);    //设置连接成功事件标志位
    }else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {      //AP-设备断开事件
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);  //清除连接成功事件标志位
    }else if (event_id == WIFI_EVENT_STA_START) {               //STA-开始连接
        esp_wifi_connect();
    }else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {        //STA-断开连接
        if (s_retry_num < 5) {                                  //错误重连
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {                                                        //超出次数退出
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);      //设置错误连接标志位
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);//清除连接成功标志位
        }
    }else if (event_id == WIFI_EVENT_SCAN_DONE){
        xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);     //发送扫描完成标志位
    }
}

/**
  * @brief  ip事件处理回调函数
  * @param  arg:传入参数，此处为NULL
  * @param  event_base:指向公开事件的子系统的唯一指针
  * @param  event_id:事件ID，查看esp_wifi_types.h
  * @param  event_data:事件数据
  * @retval void
  * @note   根据wifi的连接事件设置事件组标志位
  */
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    if (event_id == IP_EVENT_STA_GOT_IP) {               //STA-连接成功
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        sta_connect = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);     //设置连接成功标志位
    }
}

/**
  * @brief  smart config事件处理回调函数
  * @param  arg:传入参数，此处为NULL
  * @param  event_base:指向公开事件的子系统的唯一指针
  * @param  event_id:事件ID
  * @param  event_data:事件数据
  * @retval void
  * @note   打印sc的过程信息
  */
static void sc_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    if(event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE){                       //sc扫描成功
        ESP_LOGI(TAG, "SC  Scan done");
    }else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL){            //sc找到通道
        ESP_LOGI(TAG, "SC  Found channel");
    }else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD){            //sc获得密码
        ESP_LOGI(TAG, "SC  Got SSID and password");
        //提取或得到的sc账号密码信息
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);

        //更新wifi-sta信息
        bzero(sys_wifi_sta_ssid, 24);
        memcpy(sys_wifi_sta_ssid, ssid, strlen((char*)ssid));
        bzero(sys_wifi_sta_psss, 24);
        memcpy(sys_wifi_sta_psss, password, strlen((char*)password));

        ESP_ERROR_CHECK(esp_wifi_disconnect());                                     //wifi断开连接
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));        //配置成目标ap信息的STA模式
        ESP_ERROR_CHECK(esp_wifi_connect());                                        //重新连接
    }else if(event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE){         //sc设置完成发送ack到手机
        xEventGroupSetBits(s_wifi_event_group, WIFI_ESPTOUCH_DONE_BIT);
    }
}

/**
  * @brief  初始化wifi并打开
  * @param  void
  * @retval void
  */
void WIFI_Init()
{
    s_wifi_event_group = xEventGroupCreate();               //获得事件标志组
    ESP_ERROR_CHECK(esp_netif_init());                      //初始化底层TCP/IP堆栈。       
    ESP_ERROR_CHECK(esp_event_loop_create_default() );      //创建默认的事件循环。
    netif_ap = esp_netif_create_default_wifi_ap();          //创建默认的WIFI AP。在任何init错误的情况下，这个API中止。
    assert(netif_ap);
    netif_sta = esp_netif_create_default_wifi_sta();        //创建默认的WIFI STA。在任何init错误的情况下，这个API中止。
    assert(netif_sta);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();    //WiFi默认堆栈配置参数传递到esp_wifi_init调用。
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                   //为WiFi驱动程序Init WiFi Alloc资源，如WiFi控制结构、RX/TX缓冲区、WiFi NVS结构等，此WiFi也启动WiFi任务。
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));       //向系统事件注册所有wifi事件
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));           //向系统事件注册所有ip事件
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL) );          //向系统事件注册所有sc事件
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));//设置WiFi API配置存储类型
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));     //先不设置成任何模式
    sta_connect = false;
}

/**
  * @brief  将ESP32 wifi 初始化成AP模式
  * @param  APSSID：softap ID
  * @param  APPASS：softap 密码，小于8位时会忽略
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Softap(const char* APSSID, const char* APPASS)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_config_t wifi_config = {                       //配置wifi设置
        .ap = {
            .ssid = "",                                         //wifi名称
            .ssid_len = 0,                                      //可选的SSID字段长度         
            .channel = ESP_WIFI_CHANNEL_AP,                     //wifi通道
            .password = "",                                     //wifi密码
            .max_connection = MAX_STA_CONN_AP,                  //允许连接的最多站数，默认4个，最多10个
            .authmode = WIFI_AUTH_WPA_WPA2_PSK                  //ESP32软ap的认证模式。
        },
    };
    strlcpy((char*)wifi_config.ap.ssid, APSSID, sizeof(wifi_config.ap.ssid));
    if(strlen(APPASS) < 8){                           //如果密码为空则更换模式    
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        ESP_LOGI(TAG, "password less than 8, The pattern is converted to auth open.");
    }else{
        strlcpy((char*)wifi_config.ap.password, APPASS, sizeof(wifi_config.ap.password));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));                   //设置wifi操作模式                
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config)); //根据参数把wifi配置成ap模式                            
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",      //打印wifi信息
             APSSID, APPASS, ESP_WIFI_CHANNEL_AP);
}

/**
  * @brief  ESP32 wifi扫描wifiap
  * @param  ap_list 指针返回扫描到的ap信息列表
  * @param  list_size 最大扫描ap数量
  * @retval 实际扫描到的ap数量
  * @note   需要查看扫描的更多信息，需要修改wifi_scan_list_t并修改代码，查看更多的信息
  * @note   这个函数会会自动打开和关闭wifi,使用前wifi最好是关闭状态
  */
uint16_t WIFI_Scan(wifi_scan_list_t *ap_list, uint8_t list_size)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    uint16_t number = list_size;
    wifi_ap_record_t ap_info[list_size];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    wifi_config_t wifi_config = {                       //配置wifi STA设置
        .sta = {
            .ssid = "",                                         //目标ID
            .password = "",                                     //目标密码
            .scan_method = WIFI_ALL_CHANNEL_SCAN,               //扫描模式
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));    //设置wifi
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, false));
    ESP_LOGI(TAG, "scan begin\n");
    xEventGroupWaitBits(s_wifi_event_group,             //等待扫描完成标志位
                        WIFI_SCAN_DONE_BIT, 
                        pdFALSE, 
                        pdFALSE, 
                        portMAX_DELAY);
    xEventGroupClearBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);        //清除扫描完成标志位
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    for (int i = 0; (i < list_size) && (i < ap_count); i++) {
        strcpy(ap_list[i].SSID, (char*)ap_info[i].ssid);
        ap_list[i].RSSI = ap_info[i].rssi;
    }
    WIFI_Off();
    return ap_count;
}

/**
  * @brief  将ESP32 wifi 初始化成STA模式
  * @param  STASSID sta目标ID
  * @param  STAPASS sta目标密码
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Sta(const char* STASSID, const char* STAPASS)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_config_t wifi_config = {                       //配置wifi STA设置
        .sta = {
            .ssid = "",                                         //目标ID
            .password = "",                                     //目标密码
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,           //ESP32软ap的认证模式。
            .pmf_cfg = {                                        //受保护管理框架的配置结构
                .capable = true,
                .required = false
            },
        },
    };
    strlcpy((char*)wifi_config.sta.ssid, STASSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, STAPASS, sizeof(wifi_config.sta.password));

    s_retry_num = 0;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                      //设置模式
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));    //设置wifi
    ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI(TAG, "wifi init sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,              //等待事件标志组
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, 
                                           pdFALSE, 
                                           pdFALSE, 
                                           portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT){                                     //连接成功
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 STASSID, STAPASS);
        sta_connect = true;
    }else if(bits & WIFI_FAIL_BIT){                                     //连接失败
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 STASSID, STAPASS);
        sta_connect = false;
    }else{                                                              //意外错误
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        sta_connect = false;
    }
}

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
void WIFI_Init_Apsta(const char* APSSID, const char* APPASS, const char* STASSID, const char* STAPASS)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_config_t wifi_config = {                       //配置wifi设置
        .ap = {
            .ssid = "",                                             //wifi名称
            .ssid_len = 0,                                          //可选的SSID字段长度         
            .channel = ESP_WIFI_CHANNEL_AP,                         //wifi通道
            .password = "",                                         //wifi密码
            .max_connection = MAX_STA_CONN_AP,                      //允许连接的最多站数，默认4个，最多10个
            .authmode = WIFI_AUTH_WPA_WPA2_PSK                      //ESP32软ap的认证模式。
        },
        .sta = {
            .ssid = "",                                             //目标ID
            .password = "",                                         //目标密码
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,               //ESP32软ap的认证模式。
            .pmf_cfg = {                                            //受保护管理框架的配置结构
                .capable = true,
                .required = false
            }
        }
    };

    strlcpy((char*)wifi_config.ap.ssid, APSSID, sizeof(wifi_config.ap.ssid));
    if(strlen(APPASS) < 8){                                         //如果密码为空则更换模式    
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        ESP_LOGI(TAG, "password less than 8, The pattern is converted to auth open.");
    }else{
        strlcpy((char*)wifi_config.ap.password, APPASS, sizeof(wifi_config.ap.password));
    }
    strlcpy((char*)wifi_config.sta.ssid, STASSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, STAPASS, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));                    //设置模式
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));    //设置wifi
    ESP_ERROR_CHECK(esp_wifi_connect());
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,              //等待事件标志组
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, 
                                           pdFALSE, 
                                           pdFALSE, 
                                           portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT){                                         //连接成功
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 STASSID, STAPASS);
        sta_connect = true;
    }else if(bits & WIFI_FAIL_BIT){                                         //连接失败
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 STASSID, STAPASS);
        sta_connect = false;
    }else{                                                                  //意外错误
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

/**
  * @brief  ESP32开始SmartConfig
  * @param  void
  * @retval void
  * @note   默认支持ESP TOUCH模式，配网成功后wifi处于sta联网状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Smart_Config(void)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                      //设置模式
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));            //设置SmartConfig的协议类型。
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT(); //使smartconfig日志（暂时只有一个）。
    ESP_ERROR_CHECK(esp_smartconfig_start(&sc_cfg));                        //启动SmartConfig, config ESP设备连接AP，需要通过手机APP广播信息
    while(1){                                                               //等待事件标志组
        uxBits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);    
        if(uxBits & WIFI_CONNECTED_BIT){                        //连接ap成功
            ESP_LOGI(TAG, "WiFi Connected to ap");
            sta_connect = true;
        }
        if(uxBits & WIFI_ESPTOUCH_DONE_BIT){                    //sc配置完成
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();                             //停止sc
            break;
        }
    }
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    xEventGroupClearBits(s_wifi_event_group, WIFI_ESPTOUCH_DONE_BIT);
}

/**
  * @brief  ESP32开启wifi
  * @param  void
  * @retval void
  * @note   在模式初始化函数中已经自动开启wifi，此函数非必须调用
  */
void WIFI_On(void)
{
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
  * @brief  ESP32关闭wifi
  * @param  void
  * @retval void
  * @note   如果wifista已经连接则先断开
  */
void WIFI_Off(void)
{   
    if(true == sta_connect){
        s_retry_num = 5;
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        xEventGroupWaitBits(s_wifi_event_group,                         //等待断开连接事件标志组
                            WIFI_FAIL_BIT, 
                            pdFALSE, 
                            pdFALSE, 
                            portMAX_DELAY);
        xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);        //清除连接失败标志位
        sta_connect = false;
        ESP_LOGI(TAG, "sta disconnect\n");
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
}
