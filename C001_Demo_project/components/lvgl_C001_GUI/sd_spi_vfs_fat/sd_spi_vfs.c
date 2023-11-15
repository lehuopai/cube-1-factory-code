#include "sd_spi_vfs.h"

static const char *TAG = "sd_vfs";          //打印标签

/**
  * @brief  SD卡使用SPI初始化，搭载vfs/fat文件系统
  * @param  mount_path：挂载路径名，前面要加'/'
  * @param  card_formatted：卡是否要格式化，格式化后更容易挂在成功
  * @retval ESP_OK或ESP_ERR
  * @note   也可以使用mmc通讯，具体见官方历程sd_card例程
  */
esp_err_t SD_SPI_VFS_Init(char* mount_path, bool card_formatted)
{
    ESP_LOGI(TAG, "SD card SPI VFS init.");

    //配置sd卡spi驱动的基础设置
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck  = PIN_NUM_CLK;
    slot_config.gpio_cs   = PIN_NUM_CS;
    slot_config.dma_channel = 2;

    //配置文件系统的选项。
    //如果format_if_mount_failed设置为true, SD卡将被分区
    //格式化，以防挂载失败。
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = card_formatted,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    //使用上面定义的设置初始化SD卡和挂载FAT文件系统。
    //注意:esp_vfs_fat_sdmmc_mount是一个集功能于一身的便利函数。
    //开发时请检查其源代码并实现错误恢复
    //生产应用程序。
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_path, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    //SD卡初始化成功，打印其属性
    // sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "SD card SPI VFS init OK.");
    return ESP_OK;
}