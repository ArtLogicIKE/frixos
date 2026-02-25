"""Switch platform for Frixos integration."""
from __future__ import annotations

from homeassistant.components.switch import SwitchEntity, SwitchEntityDescription
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import EntityCategory
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import (
    DOMAIN,
    PARAM_FAHRENHEIT,
    PARAM_HOUR12,
    PARAM_QUIET_SCROLL,
    PARAM_QUIET_WEATHER,
    PARAM_SHOW_GRID,
    PARAM_MIRRORING,
    PARAM_DIM_DISABLE,
    PARAM_SHOW_LEADING_ZERO,
    PARAM_UPDATE_FIRMWARE,
)
from .coordinator import FrixosDataUpdateCoordinator
from .entity import FrixosEntity

SWITCH_DESCRIPTIONS: tuple[SwitchEntityDescription, ...] = (
    SwitchEntityDescription(
        key=PARAM_FAHRENHEIT,
        name="Temperature in Fahrenheit",
        icon="mdi:thermometer",
        entity_category=EntityCategory.CONFIG,
    ),
    SwitchEntityDescription(
        key=PARAM_HOUR12,
        name="12-Hour Time Format",
        icon="mdi:clock-outline",
        entity_category=EntityCategory.CONFIG,
    ),
    SwitchEntityDescription(
        key=PARAM_QUIET_SCROLL,
        name="Show Scrolling Message",
        icon="mdi:message-scroll",
        entity_category=EntityCategory.CONFIG,
    ),
    SwitchEntityDescription(
        key=PARAM_QUIET_WEATHER,
        name="Show Weather Forecast",
        icon="mdi:weather-partly-cloudy",
        entity_category=EntityCategory.CONFIG,
    ),
    SwitchEntityDescription(
        key=PARAM_SHOW_GRID,
        name="Show Grid",
        icon="mdi:grid",       
    ),
    SwitchEntityDescription(
        key=PARAM_MIRRORING,
        name="Mirror Display",
        icon="mdi:mirror",        
    ),
    SwitchEntityDescription(
        key=PARAM_DIM_DISABLE,
        name="Maintain Full Brightness",
        icon="mdi:brightness-7",
        entity_category=EntityCategory.CONFIG,
    ),
    SwitchEntityDescription(
        key=PARAM_SHOW_LEADING_ZERO,
        name="Show Leading Zero",
        icon="mdi:clock-digital",        
    ),
    SwitchEntityDescription(
        key=PARAM_UPDATE_FIRMWARE,
        name="Auto Firmware Update",
        icon="mdi:update",
        entity_category=EntityCategory.CONFIG,
    ),
)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Frixos switch entities."""
    coordinator: FrixosDataUpdateCoordinator = hass.data[DOMAIN][entry.entry_id]

    entities = [
        FrixosSwitch(coordinator, description)
        for description in SWITCH_DESCRIPTIONS
    ]

    async_add_entities(entities)


class FrixosSwitch(FrixosEntity, SwitchEntity):
    """Representation of a Frixos switch."""

    def __init__(
        self,
        coordinator: FrixosDataUpdateCoordinator,
        description: SwitchEntityDescription,
    ) -> None:
        """Initialize the switch."""
        super().__init__(
            coordinator,
            f"{coordinator.host}_{description.key}",
            description.name,
            description.icon,
        )
        self.entity_description = description

    @property
    def is_on(self) -> bool | None:
        """Return if the switch is turned on."""
        if not self.coordinator.data or not isinstance(self.coordinator.data, dict):
            return None
        
        settings = self.coordinator.data.get("settings", {})
        if not isinstance(settings, dict):
            return None
        
        value = settings.get(self.entity_description.key)
        return bool(value) if value is not None else None

    async def async_turn_on(self, **kwargs) -> None:
        """Turn the switch on."""
        success = await self.coordinator.async_set_setting(self.entity_description.key, 1)
        if success:
            self.async_write_ha_state()

    async def async_turn_off(self, **kwargs) -> None:
        """Turn the switch off."""
        success = await self.coordinator.async_set_setting(self.entity_description.key, 0)
        if success:
            self.async_write_ha_state()
