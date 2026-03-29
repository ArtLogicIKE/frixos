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
async def get_status(logs: str = "0"):
    status_data = {
        "wifi_connected": True,
        "connected_ssid": "MockSSID",
        "app": "Frixos",
        "version": "2.15b",
        "fwversion": 63,
        "poh": 1234,
        "mac_address": "AA:BB:CC:DD:EE:FF",
        "ip_address": "192.168.1.100",
        "chip_revision": 1,
        "flash_size": 4 * 1024 * 1024,
        "cpu_freq": 240000000,
        "compile_time": "Mar 29 2024 12:00:00",
        "free_heap": 100000,
        "min_free_heap": 80000,
        "current_memory_usage": 5000,
        "peak_memory_usage": 6000,
        "time_valid": 1,
        "time_just_validated": 0,
        "weather_icon_index": 1,
        "moon_icon_index": 2,
        "weather_status": True,
        "time_status": True,
        "last_weather_update": 1711713600,
        "last_time_update": 1711713600,
        "uptime": 3600,
        "lux": 150.5,
        "latitude": "48.123456",
        "longitude": "16.123456",
        "timezone": "CET-1CEST,M3.5.0,M10.5.0/3"
    }

    if logs == "1":
        status_data["system_logs"] = ["Mock log line 1", "Mock log line 2"]
        status_data["ha_tokens"] = ["Home Assistant tokens: 1, Last update: Fri Mar 29 12:00:00 2024", "0: Entity entity.name value 123"]

    return JSONResponse(content=status_data)

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
