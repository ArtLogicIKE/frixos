"""The Frixos integration."""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.config_entries import ConfigEntry
from homeassistant.const import CONF_HOST, CONF_PORT, Platform
from homeassistant.core import HomeAssistant
from homeassistant.exceptions import ConfigEntryNotReady

from .const import DOMAIN
from .coordinator import FrixosDataUpdateCoordinator

_LOGGER = logging.getLogger(__name__)

PLATFORMS: list[Platform] = [
    Platform.SENSOR,
    Platform.SWITCH,
    Platform.NUMBER,
    Platform.SELECT,
    Platform.TEXT,
]


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Set up Frixos from a config entry."""
    coordinator = FrixosDataUpdateCoordinator(
        hass,
        entry.data[CONF_HOST],
        entry.data[CONF_PORT],
    )

    # Fetch initial data so we have data when entities are added
    try:
        await coordinator.async_config_entry_first_refresh()
    except Exception as err:
        _LOGGER.error("Failed to connect to Frixos device: %s", err)
        await coordinator.async_close()
        raise ConfigEntryNotReady(f"Failed to connect: {err}") from err

    hass.data.setdefault(DOMAIN, {})
    hass.data[DOMAIN][entry.entry_id] = coordinator

    # Set up all platforms
    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)

    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Unload a config entry."""
    unload_ok = await hass.config_entries.async_unload_platforms(entry, PLATFORMS)
    if unload_ok:
        coordinator = hass.data[DOMAIN][entry.entry_id]
        await coordinator.async_close()
        hass.data[DOMAIN].pop(entry.entry_id)

    return unload_ok
