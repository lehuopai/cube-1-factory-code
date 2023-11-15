#ifndef _GUI_MAIN_INTERFACE_H
#define _GUI_MAIN_INTERFACE_H

#include "../../../lvgl/lvgl/lvgl.h"

/**
  * @brief  创建主界面
  * @param  void
  * @retval void
  * @note   这里是gui所有界面的开始，创建分别创建了状态栏和菜单选项卡。
  */
void Gui_Main_Create(void);

/**
  * @brief  显示开机界面logo
  * @param  void
  * @retval void
  */
void Gui_BootUp_Create(void);

#endif /* _GUI_MAIN_INTERFACE_H */
