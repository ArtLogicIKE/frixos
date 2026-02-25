"""Number platform for Frixos integration."""
from __future__ import annotations

from homeassistant.components.number import NumberEntity, NumberEntityDescription
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import EntityCategory
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import (
    DOMAIN,
    PARAM_X_OFFSET,
    PARAM_Y_OFFSET,
    PARAM_SCROLL_DELAY,
    PARAM_LUX_SENSITIVITY,
    PARAM_LUX_THRESHOLD,
    PARAM_BRIGHTNESS_LED,
    PARAM_PWM_FREQUENCY,
    PARAM_MAX_POWER,
    PARAM_HA_REFRESH_MINS,
    PARAM_STOCK_REFRESH_MINS,
    PARAM_DEXCOM_REFRESH,
)
from .coordinator import FrixosDataUpdateCoordinator
from .entity import FrixosEntity

NUMBER_DESCRIPTIONS: tuple[NumberEntityDescription, ...] = (
    NumberEntityDescription(
        key=PARAM_X_OFFSET,
        name="X Offset",
        icon="mdi:arrow-left-right",
        native_min_value=0,
        native_max_value=160,
        native_step=1,
        # No entity_category - appears in main/default section
    ),
    NumberEntityDescription(
        key=PARAM_Y_OFFSET,
        name="Y Offset",
        icon="mdi:arrow-up-down",
        native_min_value=0,
        native_max_value=160,
        native_step=1,
        # No entity_category - appears in main/default section
    ),
    NumberEntityDescription(
        key=PARAM_SCROLL_DELAY,
        name="Scroll Delay",
        icon="mdi:timer-outline",
        native_min_value=30,
        native_max_value=500,
        native_step=1,
        native_unit_of_measurement="ms",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=PARAM_LUX_SENSITIVITY,
        name="Light Sensitivity",
        icon="mdi:brightness-6",
        native_min_value=0,
        native_max_value=50,
        native_step=0.1,
        native_unit_of_measurement="lux",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=PARAM_LUX_THRESHOLD,
        name="Day Threshold",
        icon="mdi:brightness-6",
        native_min_value=0,
        native_max_value=500,
        native_step=0.1,
        native_unit_of_measurement="lux",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=f"{PARAM_BRIGHTNESS_LED}_0",
        name="LED Brightness (Day)",
        icon="mdi:led-on",
        native_min_value=1,
        native_max_value=100,
        native_step=1,
        native_unit_of_measurement="%",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=f"{PARAM_BRIGHTNESS_LED}_1",
        name="LED Brightness (Night)",
        icon="mdi:led-on",
        native_min_value=1,
        native_max_value=100,
        native_step=1,
        native_unit_of_measurement="%",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=PARAM_PWM_FREQUENCY,
        name="PWM Frequency",
        icon="mdi:sine-wave",
        native_min_value=10,
        native_max_value=5000,
        native_step=1,
        native_unit_of_measurement="Hz",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=PARAM_MAX_POWER,
        name="Max Power",
        icon="mdi:lightbulb-on",
        native_min_value=1,
        native_max_value=1023,
        native_step=1,
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=PARAM_HA_REFRESH_MINS,
        name="Home Assistant Refresh Interval",
        icon="mdi:refresh",
        native_min_value=1,
        native_max_value=7200,
        native_step=1,
        native_unit_of_measurement="min",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=PARAM_STOCK_REFRESH_MINS,
        name="Stock Refresh Interval",
        icon="mdi:refresh",
        native_min_value=1,
        native_max_value=1440,
        native_step=1,
        native_unit_of_measurement="min",
        entity_category=EntityCategory.CONFIG,
    ),
    NumberEntityDescription(
        key=PARAM_DEXCOM_REFRESH,
        name="Dexcom Refresh Interval",
        icon="mdi:refresh",
        native_min_value=1,
        native_max_value=60,
        native_step=1,
        native_unit_of_measurement="min",
        entity_category=EntityCategory.CONFIG,
    ),
)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Frixos number entities."""
    coordinator: FrixosDataUpdateCoordinator = hass.data[DOMAIN][entry.entry_id]

    entities = [
        FrixosNumber(coordinator, description)
        for description in NUMBER_DESCRIPTIONS
    ]

    async_add_entities(entities)


class FrixosNumber(FrixosEntity, NumberEntity):
    """Representation of a Frixos number."""

    def __init__(
        self,
        coordinator: FrixosDataUpdateCoordinator,
        description: NumberEntityDescription,
    ) -> None:
        """Initialize the number entity."""
        super().__init__(
            coordinator,
            f"{coordinator.host}_{description.key}",
            description.name,
            description.icon,
        )
        self.entity_description = description

    @property
    def native_value(self) -> float | None:
        """Return the current value."""
        if not self.coordinator.data or not isinstance(self.coordinator.data, dict):
            return None
        
        settings = self.coordinator.data.get("settings", {})
        if not isinstance(settings, dict):
            return None
        
        # Handle brightness LED array (indexed by _0 or _1 suffix)
        if self.entity_description.key.startswith(f"{PARAM_BRIGHTNESS_LED}_"):
            brightness_array = settings.get(PARAM_BRIGHTNESS_LED, [])
            if isinstance(brightness_array, list):
                index = int(self.entity_description.key.split("_")[-1])
                if index < len(brightness_array):
                    return float(brightness_array[index])
            return None
        
        value = settings.get(self.entity_description.key)
        return float(value) if value is not None else None

    async def async_set_native_value(self, value: float) -> None:
        """Update the current value."""
        param_key = self.entity_description.key
        
        # Handle brightness LED array
        if param_key.startswith(f"{PARAM_BRIGHTNESS_LED}_"):
            settings = self.coordinator.data.get("settings", {})
            brightness_array = list(settings.get(PARAM_BRIGHTNESS_LED, [0, 0]))
            index = int(param_key.split("_")[-1])
            brightness_array[index] = int(value)
            
            success = await self.coordinator.async_set_setting(PARAM_BRIGHTNESS_LED, brightness_array)
        else:
            success = await self.coordinator.async_set_setting(param_key, value)
        
        if success:
            self.async_write_ha_state()
