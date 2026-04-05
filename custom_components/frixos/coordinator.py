"""Data update coordinator for Frixos integration."""
from __future__ import annotations

import asyncio
import logging
from datetime import timedelta
from typing import Any

import aiohttp
from homeassistant.helpers.aiohttp_client import async_get_clientsession

from homeassistant.core import HomeAssistant
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, UpdateFailed

from .const import DOMAIN, ENDPOINT_SETTINGS, ENDPOINT_STATUS, DEFAULT_SCAN_INTERVAL

_LOGGER = logging.getLogger(__name__)


class FrixosDataUpdateCoordinator(DataUpdateCoordinator):
    """Class to manage fetching Frixos data."""

    def __init__(
        self,
        hass: HomeAssistant,
        host: str,
        port: int,
    ) -> None:
        """Initialize."""
        self.host = host
        self.port = port
        self.base_url = f"http://{host}:{port}"
        self._session: aiohttp.ClientSession = async_get_clientsession(hass)

        super().__init__(
            hass,
            _LOGGER,
            name=DOMAIN,
            update_interval=timedelta(seconds=DEFAULT_SCAN_INTERVAL),
        )

    async def _async_update_data(self) -> dict:
        """Fetch data from Frixos device."""
        try:
            # Fetch both settings and status
            settings_data, status_data = await asyncio.gather(
                self._fetch_settings(),
                self._fetch_status(),
                return_exceptions=True
            )
            
            # Handle exceptions - if either fails completely, we still want to try to continue
            if isinstance(settings_data, Exception):
                _LOGGER.warning("Failed to fetch settings: %s", settings_data)
                settings_data = {}
            elif not isinstance(settings_data, dict):
                _LOGGER.warning("Settings data is not a dict: %s", type(settings_data))
                settings_data = {}
                
            if isinstance(status_data, Exception):
                _LOGGER.warning("Failed to fetch status: %s", status_data)
                status_data = {}
            elif not isinstance(status_data, dict):
                _LOGGER.warning("Status data is not a dict: %s", type(status_data))
                status_data = {}
            
            # If both failed, raise an error
            if not settings_data and not status_data:
                raise UpdateFailed("Failed to fetch both settings and status from device")
            
            # Combine data
            return {
                "settings": settings_data or {},
                "status": status_data or {},
            }
        except UpdateFailed:
            raise
        except Exception as err:
            raise UpdateFailed(f"Error communicating with device: {err}") from err

    async def _fetch_settings(self) -> dict:
        """Fetch settings from device."""
        url = f"{self.base_url}{ENDPOINT_SETTINGS}"
        try:
            async with self._session.get(url, timeout=aiohttp.ClientTimeout(total=10)) as response:
                if response.status == 200:
                    data = await response.json()
                    if not isinstance(data, dict):
                        raise UpdateFailed(f"Settings endpoint returned invalid data: {type(data)}")
                    return data
                else:
                    text = await response.text()
                    raise UpdateFailed(f"Settings endpoint returned status {response.status}: {text}")
        except aiohttp.ClientError as err:
            raise UpdateFailed(f"Network error fetching settings: {err}") from err

    async def _fetch_status(self) -> dict:
        """Fetch status from device."""
        url = f"{self.base_url}{ENDPOINT_STATUS}"
        try:
            async with self._session.get(url, timeout=aiohttp.ClientTimeout(total=10)) as response:
                if response.status == 200:
                    data = await response.json()
                    if not isinstance(data, dict):
                        raise UpdateFailed(f"Status endpoint returned invalid data: {type(data)}")
                    return data
                else:
                    text = await response.text()
                    raise UpdateFailed(f"Status endpoint returned status {response.status}: {text}")
        except aiohttp.ClientError as err:
            raise UpdateFailed(f"Network error fetching status: {err}") from err

    async def async_set_setting(self, param: str, value: Any) -> bool:
        """Update a setting on the device."""
        url = f"{self.base_url}{ENDPOINT_SETTINGS}"
        payload = {param: value}
        
        try:
            async with self._session.post(
                url,
                json=payload,
                timeout=aiohttp.ClientTimeout(total=10)
            ) as response:
                if response.status == 200:
                    result = await response.json()
                    # Refresh data after successful update
                    await self.async_request_refresh()
                    # API returns {"status": "ok"} on success
                    return result.get("status") == "ok" if isinstance(result, dict) else True
                else:
                    response_text = await response.text()
                    _LOGGER.error("Failed to update setting %s: status %d, response: %s", param, response.status, response_text)
                    return False
        except Exception as err:
            _LOGGER.error("Error updating setting %s: %s", param, err)
            return False

    async def async_close(self) -> None:
        """No-op: session is managed by Home Assistant."""
