"""Constants for the Frixos integration."""
from typing import Final

DOMAIN: Final = "frixos"
DEFAULT_SCAN_INTERVAL: Final = 60  # seconds
DEFAULT_TIMEOUT: Final = 10  # seconds
DEFAULT_PORT: Final = 80

# API endpoints
ENDPOINT_SETTINGS: Final = "/api/settings"
ENDPOINT_STATUS: Final = "/api/status"

# Parameter field mappings (API uses pXX format)
PARAM_HOSTNAME = "p00"
PARAM_X_OFFSET = "p01"
PARAM_Y_OFFSET = "p02"
PARAM_ROTATION = "p03"
PARAM_DAY_FONT = "p04"
PARAM_NIGHT_FONT = "p05"
PARAM_QUIET_SCROLL = "p06"
PARAM_QUIET_WEATHER = "p07"
PARAM_SHOW_GRID = "p08"
PARAM_MIRRORING = "p09"
PARAM_COLOR_FILTER = "p10"
PARAM_NIGHT_COLOR_FILTER = "p11"
PARAM_MSG_COLOR = "p12"
PARAM_MSG_FONT = "p13"
PARAM_SCROLL_DELAY = "p14"
PARAM_NIGHT_MSG_COLOR = "p15"
PARAM_MESSAGE = "p16"
PARAM_LATITUDE = "p17"
PARAM_LONGITUDE = "p18"
PARAM_TIMEZONE = "p19"
PARAM_LUX_SENSITIVITY = "p20"
PARAM_LUX_THRESHOLD = "p21"
PARAM_DIM_DISABLE = "p22"
PARAM_BRIGHTNESS_LED = "p23"
PARAM_SHOW_LEADING_ZERO = "p24"
PARAM_HA_URL = "p25"
PARAM_HA_TOKEN = "p26"
PARAM_HA_REFRESH_MINS = "p27"
PARAM_STOCK_KEY = "p28"
PARAM_STOCK_REFRESH_MINS = "p29"
PARAM_DEXCOM_REGION = "p30"
PARAM_DEXCOM_USERNAME = "p31"
PARAM_DEXCOM_PASSWORD = "p32"
PARAM_DEXCOM_REFRESH = "p33"
PARAM_WIFI_SSID = "p34"
PARAM_WIFI_PASS = "p35"
PARAM_FAHRENHEIT = "p36"
PARAM_HOUR12 = "p37"
PARAM_SCROLL_SPEED = "p38"
PARAM_UPDATE_FIRMWARE = "p39"
PARAM_DARK_THEME = "p40"
PARAM_LANGUAGE = "p41"
PARAM_PWM_FREQUENCY = "p42"
PARAM_MAX_POWER = "p43"

# Font options
FONT_OPTIONS = [
    "bold",
    "light",
    "lcd",
    "nixie",
    "robrito",
    "ficasso",
    "lichten",
    "kablame",
    "kablamo",
    "kaboom",
    "user1",
    "user2",
]

# Color filter options
COLOR_FILTER_OPTIONS = {
    0: "None",
    1: "Red",
    2: "Green",
    3: "Blue",
    4: "Black & White",
}

# Rotation options
ROTATION_OPTIONS = {
    0: "0°",
    1: "90°",
    2: "180°",
    3: "270°",
}

# Message font options
MSG_FONT_OPTIONS = {
    0: "8pt",
    1: "10pt",
    2: "12pt",
}

# Language options
LANGUAGE_OPTIONS = {
    0: "English",
    1: "Deutsch",
    2: "Français",
    3: "Italiano",
    4: "Português",
    5: "Svenska",
    6: "Dansk",
    7: "Polski",
    8: "Español",
}

# Dexcom region options
DEXCOM_REGION_OPTIONS = {
    0: "Disabled",
    1: "US",
    2: "Japan",
    3: "Rest of World",
}

# Settings that trigger device restart
RESTART_REQUIRED_PARAMS = {
    PARAM_HOSTNAME,
    PARAM_WIFI_SSID,
    PARAM_WIFI_PASS,
    PARAM_LATITUDE,
    PARAM_LONGITUDE,
    PARAM_TIMEZONE,
}

# Password fields (should be masked in config flow)
PASSWORD_PARAMS = {
    PARAM_WIFI_PASS,
}
