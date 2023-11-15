#ifndef __SD_SPI_VFS_H
#define __SD_SPI_VFS_H

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

//spi引脚配置
#define PIN_NUM_MISO        19
#define PIN_NUM_MOSI        23
#define PIN_NUM_CLK         18
#define PIN_NUM_CS          4

/**
  * @brief  SD卡使用SPI初始化，搭载vfs/fat文件系统
  * @param  mount_path：挂载路径名，前面要加'/'
  * @param  card_formatted：卡是否要格式化，格式化后更容易挂在成功
  * @retval ESP_OK或ESP_ERR
  * @note   也可以使用mmc通讯，具体见官方历程sd_card例程
  */
esp_err_t SD_SPI_VFS_Init(char* mount_path, bool card_formatted);

#endif /* __SD_SPI_VFS_H */