"""
OpenClaw AI collector — polls the local OpenClaw/OpenWebUI API endpoint.
Reads model info, active agents, task queue, and recent log output.

Adjust the endpoint paths in config.yaml to match your OpenClaw API.
"""

import logging
import time

import aiohttp

from collectors import BaseCollector

log = logging.getLogger("blip.openclaw")

# Fallback uptime formatting
def _fmt_uptime(seconds: int) -> str:
    if seconds < 60:
        return f"{seconds}s"
    if seconds < 3600:
        return f"{seconds//60}m {seconds%60}s"
    h = seconds // 3600
    m = (seconds % 3600) // 60
    return f"{h}h {m}m"


class OpenClawCollector(BaseCollector):
    def __init__(self, state, cfg: dict, interval: float):
        super().__init__(state, interval)
        self._base_url  = cfg.get("base_url", "http://localhost:8000").rstrip("/")
        self._api_key   = cfg.get("api_key", "")
        self._start_time = time.time()

    def _headers(self) -> dict:
        h = {"Accept": "application/json"}
        if self._api_key:
            h["Authorization"] = f"Bearer {self._api_key}"
        return h

    async def collect(self):
        timeout = aiohttp.ClientTimeout(total=5)

        async with aiohttp.ClientSession(headers=self._headers()) as session:
            # --- Model info (OpenWebUI /api/models or similar) ---
            try:
                async with session.get(
                    f"{self._base_url}/api/models", timeout=timeout
                ) as r:
                    if r.status == 200:
                        data = await r.json()
                        # Try common response shapes
                        models = (
                            data.get("data") or
                            data.get("models") or
                            (data if isinstance(data, list) else [])
                        )
                        if models:
                            first = models[0]
                            name = (
                                first.get("name") or
                                first.get("id") or
                                first.get("model") or
                                "Unknown"
                            )
                            self.state.ai_model = name[:47]
                        self.state.ai_status = "ACTIVE"
            except aiohttp.ClientConnectorError:
                self.state.ai_status = "OFFLINE"
                self.state.ai_model  = "---"
                return
            except Exception as e:
                log.debug(f"Model fetch: {e}")

            # --- Chat/session stats ---
            try:
                async with session.get(
                    f"{self._base_url}/api/chats", timeout=timeout
                ) as r:
                    if r.status == 200:
                        data = await r.json()
                        chats = data.get("data") or data if isinstance(data, list) else []
                        self.state.ai_conversations = len(chats)
            except Exception as e:
                log.debug(f"Chats fetch: {e}")

            # --- Agent / task queue (OpenClaw-specific endpoints) ---
            try:
                async with session.get(
                    f"{self._base_url}/api/agents", timeout=timeout
                ) as r:
                    if r.status == 200:
                        data = await r.json()
                        agents = data.get("agents") or data if isinstance(data, list) else []
                        self.state.oc_agents = len(agents)
                        active = [a for a in agents if a.get("status") == "running"]
                        if active:
                            self.state.oc_current_tool = (
                                active[0].get("current_tool") or
                                active[0].get("tool") or
                                "running"
                            )
            except Exception as e:
                log.debug(f"Agents fetch: {e}")

            try:
                async with session.get(
                    f"{self._base_url}/api/tasks", timeout=timeout
                ) as r:
                    if r.status == 200:
                        data = await r.json()
                        tasks = data.get("tasks") or data if isinstance(data, list) else []
                        self.state.oc_tasks     = len([t for t in tasks if t.get("status") == "running"])
                        self.state.oc_queue_len = len([t for t in tasks if t.get("status") == "queued"])
                        completed = [t for t in tasks if t.get("status") == "completed"]
                        if completed:
                            last = completed[-1]
                            self.state.oc_last_task = (
                                last.get("name") or last.get("title") or "completed"
                            )
            except Exception as e:
                log.debug(f"Tasks fetch: {e}")

            # --- Activity log ---
            try:
                async with session.get(
                    f"{self._base_url}/api/logs?limit=4", timeout=timeout
                ) as r:
                    if r.status == 200:
                        data = await r.json()
                        lines = data.get("logs") or data if isinstance(data, list) else []
                        self.state.oc_log = [
                            str(l.get("message") or l)[:79]
                            for l in lines[-4:]
                        ]
            except Exception as e:
                log.debug(f"Logs fetch: {e}")

        # Uptime / memory from system collector are already in state.
        # Format uptime string.
        elapsed = int(time.time() - self._start_time)
        self.state.ai_uptime = _fmt_uptime(elapsed)
