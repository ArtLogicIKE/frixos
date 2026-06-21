import os
from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse, JSONResponse, FileResponse
from fastapi.staticfiles import StaticFiles
import uvicorn

app = FastAPI()

# Settings storage (mock)
settings = {
    "p00": "frixos",
    "p34": "MockSSID",
    "p35": "MockPass",
    "p16": "Hello [temp] test message",
    "p36": 0,
    "p37": 0,
    "p39": 1,
    "p40": 1,
    "p41": 0,
    "p01": 22,
    "p02": 22,
    "p03": 1,
    "p04": "bold",
    "p05": "light",
    "p09": 0,
    "p60": "",
    "p61": "",
    "p62": "255.255.255.0",
    "p63": "",
}

@app.get("/")
async def read_index():
    with open("spiffs/index.html", "r") as f:
        return HTMLResponse(content=f.read())

@app.get("/api/settings")
async def get_settings(group: str = None, params: str = None):
    if group == "theme":
        return JSONResponse(content={k: settings[k] for k in settings if k in ("p40", "p41")})
    if group == "settings":
        keys = {f"p{i:02d}" for i in [0, 3, 9, 16, 34, 35, 36, 37, 39, 60, 61, 62, 63]}
        keys.update({"p00", "p03", "p09", "p16", "p34", "p35", "p36", "p37", "p39", "p60", "p61", "p62", "p63"})
        return JSONResponse(content={k: settings[k] for k in keys if k in settings})
    return JSONResponse(content=settings)

@app.post("/api/settings")
async def post_settings(request: Request):
    body = await request.json()
    settings.update({k: v for k, v in body.items() if k != "status"})
    return JSONResponse(content={"status": "ok", "message": "Settings saved"})

@app.get("/i18n/language_{lang}.json")
async def get_i18n_language(lang: str):
    path = f"spiffs/i18n/language_{lang}.json"
    if os.path.exists(path):
        return FileResponse(path)
    return JSONResponse(status_code=404, content={"message": "Not found"})

@app.get("/api/status")
async def get_status(logs: str = None):
    status_data = {
        "wifi_connected": True,
        "connected_ssid": "MockSSID",
        "app": "Frixos",
        "version": "1.0.0",
        "fwversion": 65,
        "poh": 123,
        "chip_revision": 3,
        "flash_size": 4194304,
        "cpu_freq": 240000000,
        "compile_time": "Oct 27 2023 10:00:00",
        "mac_address": "ABCDEF123456",
        "ip_address": "192.168.4.1",
        "free_heap": 150000,
        "min_free_heap": 100000,
        "time_valid": 1,
        "weather_icon_index": 1,
        "moon_icon_index": 2,
        "weather_status": True,
        "time_status": True,
        "last_weather_update": 1698393600,
        "last_time_update": 1698393600,
        "uptime": 3600,
        "lux": 150.5,
        "latitude": "40.7128",
        "longitude": "-74.0060",
        "timezone": "EST5EDT,M3.2.0,M11.1.0"
    }

    if logs == "1":
        status_data["system_logs"] = ["Mock log line 1", "Mock log line 2"]
        status_data["ha_tokens"] = ["Home Assistant tokens: 1", "0: temp entity sensor.temp path state value 22.5"]

    return JSONResponse(content=status_data)

@app.get("/api/timezone")
async def get_timezone(location: str = None, lat: str = None, lon: str = None):
    iana = location
    if not iana and lat and lon:
        try:
            import urllib.request
            url = f"https://timeapi.io/api/TimeZone/coordinate?latitude={lat}&longitude={lon}"
            with urllib.request.urlopen(url, timeout=10) as resp:
                import json
                data = json.loads(resp.read().decode())
                iana = data.get("timeZone")
        except Exception:
            return JSONResponse(status_code=404, content={"message": "Timezone not found"})
    if not iana:
        return JSONResponse(status_code=400, content={"message": "Missing location or lat/lon"})
    path = "spiffs/timezone.txt"
    key = iana.replace(" ", "_")
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or ";" not in line:
                    continue
                loc, posix = line.split(";", 1)
                if loc.replace(" ", "_") == key:
                    return JSONResponse(content={"iana": iana, "posix": posix.strip()})
    return JSONResponse(status_code=404, content={"message": "Timezone not found"})

@app.get("/language_{lang}.json")
async def get_language_legacy(lang: str):
    return await get_i18n_language(lang)

@app.get("/{filename}")
async def get_static(filename: str):
    path = f"spiffs/{filename}"
    if os.path.exists(path):
        return FileResponse(path)
    return JSONResponse(status_code=404, content={"message": "Not found"})

if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8080)
