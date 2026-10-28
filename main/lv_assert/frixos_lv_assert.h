// LV_ASSERT_CUSTOM_INCLUDE (sdkconfig): included by every LVGL source via
// lv_conf_internal.h. Replaces LVGL's default "while(1);" assert handler.
#pragma once

void frixos_lv_assert_failed(const char *file, int line);

#define LV_ASSERT_HANDLER frixos_lv_assert_failed(__FILE__, __LINE__);
