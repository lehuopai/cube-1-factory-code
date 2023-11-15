#ifndef LVGL_SPI_CONF_H
#define LVGL_SPI_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

// DISPLAY PINS
#define DISP_SPI_MOSI               23          //SPI MOSI脚
#define DISP_SPI_MISO               (-1)        //SPI MISO脚
#define DISP_SPI_INPUT_DELAY_NS     (0)         //SPI 输入延时

#define DISP_SPI_IO2                (-1)
#define DISP_SPI_IO3                (-1)

#define DISP_SPI_CLK                18
#define DISP_SPI_CS                 14


//选择TFT显示器附加到的SPI总线。
#define TFT_DISPLAY_SPI_HSPI        0
//CONFIG_LV_TFT_DISPLAY_SPI_HSPI
#if TFT_DISPLAY_SPI_HSPI
#define TFT_SPI_HOST HSPI_HOST
#else
#define TFT_SPI_HOST VSPI_HOST
#endif 

//为TFT显示选择SPI双工模式。
#define TFT_DISPLAY_SPI_HALF_DUPLEX  1
//CONFIG_LV_TFT_DISPLAY_SPI_HALF_DUPLEX
#if defined (TFT_DISPLAY_SPI_HALF_DUPLEX)
#define DISP_SPI_HALF_DUPLEX
#else
#define DISP_SPI_FULL_DUPLEX
#endif

//选择TFT显示的SPI SIO/DIO/QIO传输模式。
// #define DISP_SPI_TRANS_MODE_DIO
// #define DISP_SPI_TRANS_MODE_QIO
#define DISP_SPI_TRANS_MODE_SIO

/**********************
 *      TYPEDEFS
 **********************/

#define SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)

#define SPI_TFT_CLOCK_SPEED_HZ  (60*1000*1000)

#define SPI_TFT_SPI_MODE    (0)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVGL_SPI_CONF_H*/
