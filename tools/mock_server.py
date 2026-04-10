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
    "p36": 1,
    "p37": 1,
    "p39": 1,
    "p40": 1,
    "p41": 0,
    "p01": 22,
    "p02": 22,
    "p03": 3,
    "p04": "bold",
    "p05": "light",
}

@app.get("/")
async def read_index():
    with open("spiffs/index.html", "r") as f:
        return HTMLResponse(content=f.read())

@app.get("/api/settings")
async def get_settings(group: str = None):
    return JSONResponse(content=settings)

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

@app.get("/api/wifi/scan")
async def start_wifi_scan():
    return JSONResponse(content={"status": "ok"})

@app.get("/api/wifi/status")
async def get_wifi_status():
    return JSONResponse(content={
        "scanning": False,
        "scan_done": True,
        "networks": [
            {"ssid": "MockSSID", "signal_strength": 90, "requires_password": True},
            {"ssid": "OpenSSID", "signal_strength": 60, "requires_password": False}
        ]
    })

@app.get("/language_{lang}.json")
async def get_language(lang: str):
    path = f"spiffs/language_{lang}.json"
    if os.path.exists(path):
        return FileResponse(path)
    return JSONResponse(status_code=404, content={"message": "Not found"})

@app.get("/{filename}")
async def get_static(filename: str):
    path = f"spiffs/{filename}"
    if os.path.exists(path):
        return FileResponse(path)
    return JSONResponse(status_code=404, content={"message": "Not found"})

if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8080)
