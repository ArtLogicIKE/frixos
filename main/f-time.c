#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include "frixos.h"
#include "f-time.h"
#include "moon.h"

static const char *TAG = "f-time";

time_t first_time_sync = 0;

void time_sync_notification_cb(struct timeval *tv)
{
  
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "NTP Time sync");
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
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Moon phase after NTP: %d", moon_icon_index);
  if (first_time_sync == 0) {
    first_time_sync = now;
  }
}

void sync_time_with_ntp(void)
{
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Initializing SNTP");

  // First stop any existing SNTP instance
  esp_sntp_stop();

  // Set operating mode before initialization
  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");  // Default NTP server
  esp_sntp_setservername(1, "time.nist.gov"); // Backup NTP server
  esp_sntp_setservername(2, "time.google.com");

  esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  esp_sntp_init();

    // ✅ Set timezone
  setenv("TZ", my_timezone, 1); // Set environment variable for timezone
  tzset();                      // Apply the timezone settings
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Timezone set to: %s", my_timezone);
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