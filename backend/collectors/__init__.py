"""Base class for all data collectors."""

import asyncio
import logging

log = logging.getLogger("blip.collector")


class BaseCollector:
    """Runs an async loop calling collect() at a fixed interval."""

    def __init__(self, state, interval: float):
        self.state    = state
        self.interval = interval

    async def collect(self):
        raise NotImplementedError

    async def run(self):
        name = self.__class__.__name__
        log.info(f"{name} started (interval={self.interval}s)")
        while True:
            try:
                await self.collect()
            except Exception as exc:
                log.warning(f"{name} error: {exc}")
            await asyncio.sleep(self.interval)
