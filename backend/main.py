"""
Blip Terminal Backend - Main entry point
Starts all collectors and the FastAPI server.
"""

import asyncio
import logging
import os
import sys
from pathlib import Path

import yaml
import uvicorn

from collectors.system import SystemCollector
from collectors.discord_collector import DiscordCollector
from collectors.openclaw import OpenClawCollector
from collectors.weather import WeatherCollector
from collectors.network import NetworkCollector
from api.server import app, state

log = logging.getLogger("blip")


def load_config() -> dict:
    """Load config.yaml from the backend directory."""
    config_path = Path(__file__).parent / "config.yaml"
    if not config_path.exists():
        example = Path(__file__).parent / "config.example.yaml"
        log.error(
            "config.yaml not found. Copy config.example.yaml to config.yaml "
            "and fill in your values."
        )
        if example.exists():
            import shutil
            shutil.copy(example, config_path)
            log.warning("Copied config.example.yaml -> config.yaml with placeholder values.")
        else:
            sys.exit(1)

    with open(config_path) as f:
        return yaml.safe_load(f)


async def run_collectors(cfg: dict):
    """Spin up all data collectors as background tasks."""
    intervals = cfg.get("intervals", {})

    collectors = [
        SystemCollector(state, intervals.get("system", 2)),
        WeatherCollector(state, cfg.get("weather", {}), intervals.get("weather", 300)),
        NetworkCollector(state, cfg.get("network", {}), intervals.get("network", 10),
                         intervals.get("speedtest", 300)),
        OpenClawCollector(state, cfg.get("openclaw", {}), intervals.get("openclaw", 3)),
    ]

    discord_cfg = cfg.get("discord", {})
    discord_token = discord_cfg.get("token", "")
    if discord_token and discord_token != "YOUR_BOT_TOKEN_HERE":
        collectors.append(
            DiscordCollector(state, discord_cfg, intervals.get("discord", 15))
        )
    else:
        log.warning("Discord token not configured — Discord screen will show zeros.")

    tasks = [asyncio.create_task(c.run()) for c in collectors]
    await asyncio.gather(*tasks)


async def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(name)s] %(levelname)s: %(message)s",
        datefmt="%H:%M:%S",
    )

    cfg = load_config()
    server_cfg = cfg.get("server", {})

    log.info("Blip Terminal Backend starting...")
    log.info(f"Serving on http://{server_cfg.get('host','0.0.0.0')}:{server_cfg.get('port',8765)}")

    # Run collectors concurrently with the web server
    server = uvicorn.Server(
        uvicorn.Config(
            app=app,
            host=server_cfg.get("host", "0.0.0.0"),
            port=server_cfg.get("port", 8765),
            log_level=server_cfg.get("log_level", "info"),
        )
    )

    await asyncio.gather(
        server.serve(),
        run_collectors(cfg),
    )


if __name__ == "__main__":
    asyncio.run(main())
