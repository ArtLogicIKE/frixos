"""Select platform for Frixos integration."""
from __future__ import annotations

from homeassistant.components.select import SelectEntity, SelectEntityDescription
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity import EntityCategory
from homeassistant.helpers.entity_platform import AddEntitiesCallback

from .const import (
    DOMAIN,
    PARAM_ROTATION,
    PARAM_DAY_FONT,
    PARAM_NIGHT_FONT,
    PARAM_COLOR_FILTER,
    PARAM_NIGHT_COLOR_FILTER,
    PARAM_MSG_FONT,
    PARAM_DEXCOM_REGION,
    PARAM_LANGUAGE,
    FONT_OPTIONS,
    COLOR_FILTER_OPTIONS,
    ROTATION_OPTIONS,
    MSG_FONT_OPTIONS,
    DEXCOM_REGION_OPTIONS,
    LANGUAGE_OPTIONS,
)
from .coordinator import FrixosDataUpdateCoordinator
from .entity import FrixosEntity

SELECT_DESCRIPTIONS: tuple[SelectEntityDescription, ...] = (
    SelectEntityDescription(
        key=PARAM_ROTATION,
        name="Display Rotation",
        icon="mdi:rotate-3d-variant",
        options=list(ROTATION_OPTIONS.values()),        
    ),
    SelectEntityDescription(
        key=PARAM_DAY_FONT,
        name="Day Font",
        icon="mdi:format-font",
        options=FONT_OPTIONS,
        entity_category=EntityCategory.CONFIG,
    ),
    SelectEntityDescription(
        key=PARAM_NIGHT_FONT,
        name="Night Font",
        icon="mdi:format-font",
        options=FONT_OPTIONS,
        entity_category=EntityCategory.CONFIG,
    ),
    SelectEntityDescription(
        key=PARAM_COLOR_FILTER,
        name="Day Color Filter",
        icon="mdi:palette",
        options=list(COLOR_FILTER_OPTIONS.values()),
        entity_category=EntityCategory.CONFIG,
    ),
    SelectEntityDescription(
        key=PARAM_NIGHT_COLOR_FILTER,
        name="Night Color Filter",
        icon="mdi:palette",
        options=list(COLOR_FILTER_OPTIONS.values()),
        entity_category=EntityCategory.CONFIG,
    ),
    SelectEntityDescription(
        key=PARAM_MSG_FONT,
        name="Message Font Size",
        icon="mdi:format-size",
        options=list(MSG_FONT_OPTIONS.values()),
        entity_category=EntityCategory.CONFIG,
    ),
    SelectEntityDescription(
        key=PARAM_DEXCOM_REGION,
        name="Dexcom Region",
        icon="mdi:map-marker",
        options=list(DEXCOM_REGION_OPTIONS.values()),
        entity_category=EntityCategory.CONFIG,
    ),
    SelectEntityDescription(
        key=PARAM_LANGUAGE,
        name="Language",
        icon="mdi:translate",
        options=list(LANGUAGE_OPTIONS.values()),
        entity_category=EntityCategory.CONFIG,
    ),
)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """Set up Frixos select entities."""
    coordinator: FrixosDataUpdateCoordinator = hass.data[DOMAIN][entry.entry_id]

    # Map each description to its options mapping
    mappings = {
        PARAM_ROTATION: ROTATION_OPTIONS,
        PARAM_DAY_FONT: None,  # String value, no mapping
        PARAM_NIGHT_FONT: None,  # String value, no mapping
        PARAM_COLOR_FILTER: COLOR_FILTER_OPTIONS,
        PARAM_NIGHT_COLOR_FILTER: COLOR_FILTER_OPTIONS,
        PARAM_MSG_FONT: MSG_FONT_OPTIONS,
        PARAM_DEXCOM_REGION: DEXCOM_REGION_OPTIONS,
        PARAM_LANGUAGE: LANGUAGE_OPTIONS,
    }

    entities = [
        FrixosSelect(coordinator, description, mappings[description.key])
        for description in SELECT_DESCRIPTIONS
    ]

    async_add_entities(entities)


class FrixosSelect(FrixosEntity, SelectEntity):
    """Representation of a Frixos select."""

    def __init__(
        self,
        coordinator: FrixosDataUpdateCoordinator,
        description: SelectEntityDescription,
        options_map: dict[int, str] | None,
    ) -> None:
        """Initialize the select entity."""
        super().__init__(
            coordinator,
            f"{coordinator.host}_{description.key}",
            description.name,
            description.icon,
        )
        self.entity_description = description
        self._param_key = description.key
        self._options_map = options_map

    @property
    def current_option(self) -> str | None:
        """Return the selected option."""
        if not self.coordinator.data or not isinstance(self.coordinator.data, dict):
            return None
        
        settings = self.coordinator.data.get("settings", {})
        if not isinstance(settings, dict):
            return None
        
        value = settings.get(self._param_key)
        
        if value is None:
            return None
        
        # If we have a mapping (for numeric values), use it
        if self._options_map:
            try:
                return self._options_map.get(int(value))
            except (ValueError, TypeError):
                return None
        
        # Otherwise, return the value as-is (for string values like fonts)
        return str(value) if value else None

    async def async_select_option(self, option: str) -> None:
        """Change the selected option."""
        # If we have a mapping, find the key for the option
        if self._options_map:
            # Find key by value
            for key, val in self._options_map.items():
                if val == option:
                    success = await self.coordinator.async_set_setting(self._param_key, key)
                    if success:
                        self.async_write_ha_state()
                    return
        else:
            # For string values (fonts), use the option directly
            success = await self.coordinator.async_set_setting(self._param_key, option)
            if success:
                self.async_write_ha_state()
