#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include "esp_sntp.h"
#include "frixos.h"
#include "f-time.h"
#include "f-wifi.h"
#include "moon.h"

// Language index: 0=en, 1=de, 2=fr, 3=it, 4=pt, 5=sv, 6=da, 7=pl, 8=es
static const char *ampm_suffix[9][2] = {
    {"am", "pm"},        // English
    {"vorm.", "nachm."}, // German
    {"mat.", "apr."},    // French
    {"matt.", "pom."},   // Italian
    {"man.", "tar."},    // Portuguese
    {"fm", "em"},        // Swedish
    {"fm", "em"},        // Danish
    {"AM", "PM"},        // Polish
    {"a.m.", "p.m."},    // Spanish
};

static uint8_t time_lang_index(void)
{
  uint8_t lang_index = eeprom_language;
  if (lang_index >= 9)
  {
    lang_index = 0;
  }
  return lang_index;
}

int time_hour12(const struct tm *timeinfo)
{
  int hour = timeinfo->tm_hour;
  if (hour == 0)
  {
    return 12;
  }
  if (hour > 12)
  {
    return hour - 12;
  }
  return hour;
}

const char *time_localized_ampm(const struct tm *timeinfo)
{
  if (!eeprom_12hour)
  {
    return "";
  }
  bool is_pm = timeinfo->tm_hour >= 12;
  return ampm_suffix[time_lang_index()][is_pm ? 1 : 0];
}

void format_local_time(char *buffer, size_t buffer_size, const struct tm *timeinfo)
{
  if (eeprom_12hour)
  {
    snprintf(buffer, buffer_size, "%d:%02d%s", time_hour12(timeinfo), timeinfo->tm_min,
             time_localized_ampm(timeinfo));
  }
  else
  {
    snprintf(buffer, buffer_size, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  }
}

static const char *TAG = "f-time";

time_t first_time_sync = 0;

void time_sync_notification_cb(struct timeval *tv)
{
  
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "NTP sync");
  time_valid = 1; // set time as valid once we receive the first NTP event
  if (time_just_validated == -1)
    time_just_validated = 1;
  // we might as well calculate the moon phase too
  // we do it here, with every weather, but also when NTP is synchronized
  moon_icon_index = get_moon_index();
  time_t now = tv->tv_sec;
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  time(&last_time_update); // update timestamp
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Moon phase: %d", moon_icon_index);
  if (first_time_sync == 0) {
    first_time_sync = now;
  }
}

void sync_time_with_ntp(void)
{
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "SNTP init");

  // First stop any existing SNTP instance
  esp_sntp_stop();

  // Set operating mode before initialization
  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");  // Default NTP server
  esp_sntp_setservername(1, "time.nist.gov"); // Backup NTP server
  esp_sntp_setservername(2, "time.google.com");

  esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  esp_sntp_init();

  // Set timezone - validate first; invalid strings (e.g. "GMT +2") can crash tzset/localtime
  const char *tz_to_use = (validate_timezone(my_timezone) && strlen(my_timezone) > 0) ? my_timezone : "UTC";
  setenv("TZ", tz_to_use, 1);
  tzset();
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Timezone: %s", tz_to_use);
}



// Function to get the current hour and minute
void get_current_time(int *hour, int *minute)
{
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  *hour = timeinfo.tm_hour;
  *minute = timeinfo.tm_min;
}

int get_moon_index()
{
  time_t now;
  time(&now);

  return calculateMoonIndex(now);
}

bool is_time_in_range(uint16_t start_min, uint16_t end_min, int current_min)
{
  if (start_min == 0 && end_min == 0)
    return true;

  if (start_min == end_min && start_min != 0)
    return true;

  if (start_min <= end_min)
    return (current_min >= (int)start_min && current_min <= (int)end_min);

  return (current_min >= (int)start_min || current_min <= (int)end_min);
}