#ifndef F_WU_H
#define F_WU_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/* One current observation from api.weather.com PWS, stored in metric units.
 * Wind is m/s (the API's metric block is km/h). Display converts with the
 * Fahrenheit flag, the same way met.no tokens do. */
typedef struct {
    bool valid;
    time_t obs_epoch;
    bool has_temp;
    double temp_c;
    bool has_hum;
    double humidity;
    bool has_dew;
    double dew_c;
    bool has_wind;
    double wind_mps;
    int wind_dir_deg;
    bool has_gust;
    double gust_mps;
    bool has_pressure;
    double pressure_hpa;
    bool has_rain;
    double rain_mm;
    bool has_uv;
    double uv;
} wu_obs_t;

extern wu_obs_t wu_obs;

/* Parse a current-conditions body. On failure *out is left unchanged
 * (no observations, or "Data Expired"). */
bool wu_parse_current(const char *json, wu_obs_t *out);

/* Write the display string for one [wu:…] token. Returns false when that
 * field is absent; *buf is then left unchanged. */
bool wu_format_token(const char *token, char *buf, size_t n, bool fahrenheit);

/* Fresh means a stored observation younger than 30 minutes. A clock that
 * has not been set yet counts as fresh so the dot is not stuck amber. */
bool wu_obs_is_fresh(const wu_obs_t *obs, time_t now);
void wu_format_age(const wu_obs_t *obs, time_t now, char *buf, size_t n);

bool fetch_wu_observation(void);

#endif /* F_WU_H */
