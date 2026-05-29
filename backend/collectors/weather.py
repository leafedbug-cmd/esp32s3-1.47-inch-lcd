"""Weather collector — OpenWeatherMap current conditions."""

import logging

import aiohttp

from collectors import BaseCollector

log = logging.getLogger("blip.weather")

# Weather condition codes -> simple icon string
_ICON_MAP = {
    range(200, 300): "STORM",
    range(300, 400): "DRIZL",
    range(500, 600): "RAIN",
    range(600, 700): "SNOW",
    range(700, 800): "MIST",
    800:             "CLEAR",
    range(801, 900): "CLOUD",
}

def _condition_icon(code: int) -> str:
    if code == 800:
        return "CLEAR"
    for k, v in _ICON_MAP.items():
        if isinstance(k, range) and code in k:
            return v
    return "?"


class WeatherCollector(BaseCollector):
    def __init__(self, state, cfg: dict, interval: float):
        super().__init__(state, interval)
        self._api_key = cfg.get("api_key", "")
        self._city    = cfg.get("city", "New York")
        self._units   = cfg.get("units", "imperial")

    async def collect(self):
        if not self._api_key or self._api_key == "YOUR_OPENWEATHERMAP_KEY":
            self.state.weather      = "NO API KEY"
            self.state.weather_icon = "?"
            return

        unit_sym = "F" if self._units == "imperial" else "C"
        url = (
            "https://api.openweathermap.org/data/2.5/weather"
            f"?q={self._city}&appid={self._api_key}&units={self._units}"
        )

        async with aiohttp.ClientSession() as session:
            async with session.get(url, timeout=aiohttp.ClientTimeout(total=10)) as resp:
                if resp.status != 200:
                    log.warning(f"Weather API returned {resp.status}")
                    return
                data = await resp.json()

        temp   = round(data["main"]["temp"])
        desc   = data["weather"][0]["description"].title()
        code   = data["weather"][0]["id"]

        self.state.weather      = f"{temp}\xb0{unit_sym} {desc}"
        self.state.weather_icon = _condition_icon(code)
        log.debug(f"Weather: {self.state.weather}")
