"""Sensor platform for Frixos integration."""
from __future__ import annotations

from homeassistant.components.sensor import SensorEntity, SensorEntityDescription
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import EntityCategory
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import DOMAIN
from .coordinator import FrixosDataUpdateCoordinator
from .entity import FrixosEntity

SENSOR_DESCRIPTIONS: tuple[SensorEntityDescription, ...] = (
    SensorEntityDescription(
        key="lux",
        name="Light Level",
        native_unit_of_measurement="lx",
        icon="mdi:brightness-6",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SensorEntityDescription(
        key="uptime",
        name="Uptime",
        native_unit_of_measurement="s",
        icon="mdi:timer-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SensorEntityDescription(
        key="free_heap",
        name="Free Heap Memory",
        native_unit_of_measurement="bytes",
        icon="mdi:memory",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SensorEntityDescription(
        key="min_free_heap",
        name="Min Free Heap Memory",
        native_unit_of_measurement="bytes",
        icon="mdi:memory",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Frixos sensor entities."""
    coordinator: FrixosDataUpdateCoordinator = hass.data[DOMAIN][entry.entry_id]

    entities = [
        FrixosSensor(coordinator, description)
        for description in SENSOR_DESCRIPTIONS
    ]

    async_add_entities(entities)


class FrixosSensor(FrixosEntity, SensorEntity):
    """Representation of a Frixos sensor."""

    def __init__(
        self,
        coordinator: FrixosDataUpdateCoordinator,
        description: SensorEntityDescription,
    ) -> None:
        """Initialize the sensor."""
        super().__init__(
            coordinator,
            f"{coordinator.host}_{description.key}",
            description.name,
            description.icon,
        )
        self.entity_description = description

    @property
    def native_value(self) -> float | int | str | None:
        """Return the state of the sensor."""
        if not self.coordinator.data or not isinstance(self.coordinator.data, dict):
            return None
        
        status = self.coordinator.data.get("status", {})
        if not isinstance(status, dict):
            return None
        
        value = status.get(self.entity_description.key)
        return value

    @property
    def available(self) -> bool:
        """Return if entity is available."""
        return self.coordinator.last_update_success
