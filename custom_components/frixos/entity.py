"""Base entity for Frixos integration."""
from __future__ import annotations

from homeassistant.helpers.entity import DeviceInfo
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN
from .coordinator import FrixosDataUpdateCoordinator


class FrixosEntity(CoordinatorEntity):
    """Base entity for Frixos devices."""

    def __init__(
        self,
        coordinator: FrixosDataUpdateCoordinator,
        unique_id: str,
        name: str,
        icon: str | None = None,
    ) -> None:
        """Initialize the entity."""
        super().__init__(coordinator)
        self._attr_unique_id = unique_id
        self._attr_name = name
        self._attr_icon = icon

    @property
    def device_info(self) -> DeviceInfo:
        """Return device information."""
        # Safely get version from coordinator data
        version = "Unknown"
        if self.coordinator.data and isinstance(self.coordinator.data, dict):
            status = self.coordinator.data.get("status", {})
            if isinstance(status, dict):
                version = status.get("version", "Unknown")
        
        return DeviceInfo(
            identifiers={(DOMAIN, self.coordinator.host)},
            name=self.coordinator.host,
            manufacturer="Frixos",
            model="Frixos Device",
            sw_version=version,
        )
