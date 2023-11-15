file(GLOB SOURCES *.c)
file(GLOB SOURCES_font GUI_font/*.c)
file(GLOB SOURCES_image GUI_image/*.c)
file(GLOB SOURCES_main_interface GUI_main_interface/*.c)
file(GLOB SOURCES_setting_interface GUI_setting_interface/*.c)
file(GLOB SOURCES_folder_interface GUI_folder_interface/*.c)
file(GLOB SOURCES_RGBled_interface GUI_RGBled_interface/*.c)
file(GLOB SOURCES_I2Cdevice_interface GUI_I2Cdevice_interface/*.c)
file(GLOB SOURCES_calendar_interface GUI_calendar_interface/*.c)
file(GLOB SOURCES_gyroscop_interface GUI_gyroscop_interface/*.c)

file(GLOB SOURCES_I2Cdriver GUI_I2Cdriver/*.c)
file(GLOB SOURCES_SD_FAT sd_spi_vfs_fat/*.c)


idf_component_register(SRCS ${SOURCES}
		     SRCS ${SOURCES_font}
		     SRCS ${SOURCES_image}
		     SRCS ${SOURCES_main_interface}
		     SRCS ${SOURCES_setting_interface}
		     SRCS ${SOURCES_folder_interface}
		     SRCS ${SOURCES_RGBled_interface}
		     SRCS ${SOURCES_calendar_interface}
		     SRCS ${SOURCES_gyroscop_interface}
		     SRCS ${SOURCES_I2Cdevice_interface}
		     SRCS ${SOURCES_I2Cdriver}
		     SRCS ${SOURCES_SD_FAT}
                       INCLUDE_DIRS  .
			GUI_main_interface/include
		    	GUI_setting_interface/include
			GUI_folder_interface/include
				sd_spi_vfs_fat/include 
				"${IDF_PATH}/components/fatfs/vfs"
				"${IDF_PATH}/components/fatfs/src"
				"${IDF_PATH}/components/wear_levelling/include"
				"${IDF_PATH}/components/spi_flash/include"
				"${IDF_PATH}/components/sdmmc/include"
			GUI_RGBled_interface/include 
			GUI_calendar_interface/include
			GUI_gyroscop_interface/include
			GUI_I2Cdevice_interface/include 
			GUI_I2Cdriver/include
)
