# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
 list(APPEND EXTRA_COMPONENT_DIRS  components/lvgl/
				components/lvgl/lvgl
				components/lvgl_esp32_drivers 
				components/lvgl_esp32_drivers/lvgl_touch 
				components/lvgl_esp32_drivers/lvgl_tft
				)
project(C001_Project)