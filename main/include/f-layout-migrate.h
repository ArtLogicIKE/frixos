#ifndef F_LAYOUT_MIGRATE_H
#define F_LAYOUT_MIGRATE_H

#include <stdint.h>
#include "frixos.h"
#include "f-text-widget.h"

// One-time conversion of layouts saved before the unified text widget (layout version 10 and
// older: digit displays, text labels and digit labels, plus the display schedule kept in NVS).

#define LEGACY_ELEM_COUNT 28
#define LEGACY_STATIC_TEXT_COUNT 8
#define LEGACY_STATIC_TEXT_LEN 96

// Profile as written by layout version 10 (1924 bytes).
typedef struct __attribute__((packed))
{
  screen_widget_t widget[LEGACY_ELEM_COUNT];
  char scroll_text[SCROLL_MSG_LENGTH];
  char static_text[LEGACY_STATIC_TEXT_COUNT][LEGACY_STATIC_TEXT_LEN];
  char digit_label_text[LEGACY_STATIC_TEXT_LEN];
  char digit_label_aux_text[LEGACY_STATIC_TEXT_LEN];
  screen_graph_cfg_t graph;
} legacy_profile_v10_t;

#define LEGACY_PROFILE_V10_SIZE 1924

// Fill `layout` and `tw` from the two legacy profiles. Reads the display schedule from NVS.
void screen_layout_migrate_v10(const legacy_profile_v10_t legacy[FRIXOS_SCREEN_LAYOUT_PROFILES],
                               screen_layout_t *layout, text_widgets_layout_t *tw);

// Convert the v11 message element into a scrolling text-widget instance.
void screen_layout_migrate_v11(screen_layout_t *layout, text_widgets_layout_t *tw);

// Delete the legacy schedule keys from NVS; call once the migrated layout is saved.
void screen_layout_migrate_v10_done(void);

#endif
