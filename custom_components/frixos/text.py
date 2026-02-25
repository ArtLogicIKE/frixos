"""Text platform for Frixos integration."""
from __future__ import annotations

from homeassistant.components.text import TextEntity, TextEntityDescription
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import EntityCategory
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import (
    DOMAIN,
    PARAM_MESSAGE,
    PARAM_LATITUDE,
    PARAM_LONGITUDE,
    PARAM_TIMEZONE,
    PASSWORD_PARAMS,
)
from .coordinator import FrixosDataUpdateCoordinator
from .entity import FrixosEntity

# Map parameter keys to their max lengths
TEXT_MAX_LENGTHS = {
    PARAM_MESSAGE: 511,
    PARAM_LATITUDE: 12,
    PARAM_LONGITUDE: 12,
    PARAM_TIMEZONE: 64,
}

TEXT_DESCRIPTIONS: tuple[TextEntityDescription, ...] = (
    TextEntityDescription(
        key=PARAM_MESSAGE,
        name="Scrolling Message",
        icon="mdi:message-text",
        entity_category=EntityCategory.CONFIG,
    ),
    TextEntityDescription(
        key=PARAM_LATITUDE,
        name="Latitude",
        icon="mdi:latitude",
        entity_category=EntityCategory.CONFIG,
    ),
    TextEntityDescription(
        key=PARAM_LONGITUDE,
        name="Longitude",
        icon="mdi:longitude",
        entity_category=EntityCategory.CONFIG,
    ),
    TextEntityDescription(
        key=PARAM_TIMEZONE,
        name="Timezone",
        icon="mdi:map-clock",
        entity_category=EntityCategory.CONFIG,
    ),
)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Frixos text entities."""
    coordinator: FrixosDataUpdateCoordinator = hass.data[DOMAIN][entry.entry_id]

    entities = [
        FrixosText(coordinator, description)
        for description in TEXT_DESCRIPTIONS
    ]

    async_add_entities(entities)


class FrixosText(FrixosEntity, TextEntity):
    """Representation of a Frixos text entity."""

    def __init__(
        self,
        coordinator: FrixosDataUpdateCoordinator,
        description: TextEntityDescription,
    ) -> None:
        """Initialize the text entity."""
        super().__init__(
            coordinator,
            f"{coordinator.host}_{description.key}",
            description.name,
            description.icon,
        )
        self.entity_description = description
        self._attr_native_min = 0
        # Get max length from our mapping
        self._attr_native_max = TEXT_MAX_LENGTHS.get(description.key, 255)
        # Set password mode for sensitive fields
        if description.key in PASSWORD_PARAMS:
            self._attr_mode = "password"

    @property
    def native_value(self) -> str | None:
        """Return the current value."""
        if not self.coordinator.data or not isinstance(self.coordinator.data, dict):
            return ""
        
        settings = self.coordinator.data.get("settings", {})
        if not isinstance(settings, dict):
            return ""
        
        value = settings.get(self.entity_description.key)
        return str(value) if value is not None else ""

    async def async_set_value(self, value: str) -> None:
        """Update the current value."""
        success = await self.coordinator.async_set_setting(self.entity_description.key, value)
        if success:
            self.async_write_ha_state()
