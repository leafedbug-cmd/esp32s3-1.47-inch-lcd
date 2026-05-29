"""
Discord collector — uses discord.py to read server stats.
Runs as a Discord bot. Requires a bot token with:
  - Server Members Intent
  - Message Content Intent (optional, for message counting)
"""

import asyncio
import logging
from datetime import datetime, timezone

import discord

from collectors import BaseCollector

log = logging.getLogger("blip.discord")


class DiscordCollector(BaseCollector):
    """
    Wraps a discord.py Client inside the async collector loop.
    The bot connects once and then reads cached guild data on each tick.
    """

    def __init__(self, state, cfg: dict, interval: float):
        super().__init__(state, interval)
        self._token    = cfg.get("token", "")
        self._guild_id = int(cfg.get("guild_id", 0))
        self._client   = None
        self._ready    = asyncio.Event()

    async def run(self):
        """Override run — start the bot AND the polling loop together."""
        if not self._token:
            log.warning("Discord token missing — skipping")
            return

        intents = discord.Intents.default()
        intents.members         = True
        intents.message_content = True
        intents.presences       = True

        self._client = discord.Client(intents=intents)

        @self._client.event
        async def on_ready():
            log.info(f"Discord bot logged in as {self._client.user}")
            self._ready.set()

        # Run bot and polling loop concurrently
        await asyncio.gather(
            self._client.start(self._token),
            self._poll_loop(),
        )

    async def _poll_loop(self):
        await self._ready.wait()
        while True:
            try:
                await self.collect()
            except Exception as e:
                log.warning(f"Discord collect error: {e}")
            await asyncio.sleep(self.interval)

    async def collect(self):
        if not self._client or not self._client.is_ready():
            return

        guild = self._client.get_guild(self._guild_id)
        if not guild:
            log.warning(f"Guild {self._guild_id} not found")
            return

        self.state.dc_members = guild.member_count or 0

        # Online members (not offline/invisible)
        online = sum(
            1 for m in guild.members
            if m.status not in (discord.Status.offline, discord.Status.invisible)
        )
        self.state.dc_online = online

        # Voice channel users
        voice_users = sum(
            len(vc.members) for vc in guild.voice_channels
        )
        self.state.dc_voice_users = voice_users

        # Messages today — scan recent messages in text channels
        today_count = await self._count_today_messages(guild)
        self.state.dc_msgs_today = today_count

    async def _count_today_messages(self, guild: discord.Guild) -> int:
        """Count messages sent today across all text channels."""
        today_start = datetime.now(timezone.utc).replace(
            hour=0, minute=0, second=0, microsecond=0
        )
        total = 0
        for channel in guild.text_channels:
            try:
                async for msg in channel.history(after=today_start, limit=500):
                    total += 1
            except discord.Forbidden:
                pass
            except Exception as e:
                log.debug(f"Channel {channel.name} read error: {e}")
        return total
