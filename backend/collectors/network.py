"""
Network collector — ping, external IP, LAN device count, speedtest.
WiFi and BLE scanning are done by the ESP32 itself; this collector
provides the PC-side network view.
"""

import asyncio
import logging
import socket
import subprocess
import time

import aiohttp

from collectors import BaseCollector

log = logging.getLogger("blip.network")


class NetworkCollector(BaseCollector):
    def __init__(self, state, cfg: dict, interval: float, speedtest_interval: float):
        super().__init__(state, interval)
        self._ping_host          = cfg.get("ping_host", "8.8.8.8")
        self._lan_subnet         = cfg.get("lan_subnet", "192.168.1.0/24")
        self._speedtest_interval = speedtest_interval
        self._last_speedtest     = 0.0

    async def collect(self):
        loop = asyncio.get_event_loop()

        # Ping
        self.state.ping_ms = await loop.run_in_executor(None, self._ping)

        # External IP (cheap, cached by aiohttp)
        try:
            async with aiohttp.ClientSession() as s:
                async with s.get("https://api.ipify.org",
                                 timeout=aiohttp.ClientTimeout(total=5)) as r:
                    if r.status == 200:
                        self.state.external_ip = (await r.text()).strip()
        except Exception:
            pass

        # LAN device count via arp-scan (best-effort)
        self.state.lan_devices = await loop.run_in_executor(None, self._arp_count)

        # Latency (same as ping for now)
        self.state.latency_ms = self.state.ping_ms

        # Speedtest — only run periodically (expensive)
        now = time.time()
        if now - self._last_speedtest >= self._speedtest_interval:
            self._last_speedtest = now
            dl, ul = await loop.run_in_executor(None, self._speedtest)
            self.state.dl_mbps = dl
            self.state.ul_mbps = ul

    def _ping(self) -> int:
        """ICMP ping via subprocess. Returns latency ms or 999 on failure."""
        try:
            import platform
            flag = "-n" if platform.system().lower() == "windows" else "-c"
            result = subprocess.run(
                ["ping", flag, "1", self._ping_host],
                capture_output=True, text=True, timeout=5
            )
            output = result.stdout
            # Parse "time=XXms" or "time=XX ms"
            for line in output.splitlines():
                line_l = line.lower()
                if "time=" in line_l:
                    part = line_l.split("time=")[-1].split()[0]
                    ms_str = part.replace("ms", "").replace("<", "").strip()
                    return int(float(ms_str))
        except Exception as e:
            log.debug(f"Ping error: {e}")
        return 999

    def _arp_count(self) -> int:
        """Count unique hosts visible via ARP table. No root needed."""
        try:
            result = subprocess.run(
                ["arp", "-a"], capture_output=True, text=True, timeout=5
            )
            # Count lines that contain an IP address pattern
            count = sum(
                1 for line in result.stdout.splitlines()
                if "(" in line or "dynamic" in line.lower() or
                   (len(line.split()) >= 2 and "." in line.split()[0])
            )
            return max(0, count - 1)  # subtract header line
        except Exception:
            return 0

    def _speedtest(self):
        """Run speedtest-cli. Returns (download_mbps, upload_mbps)."""
        try:
            import speedtest as st
            s = st.Speedtest(secure=True)
            s.get_best_server()
            s.download(threads=4)
            s.upload(threads=4)
            r = s.results.dict()
            dl = round(r["download"] / 1e6, 1)
            ul = round(r["upload"]   / 1e6, 1)
            log.info(f"Speedtest: DL={dl} UL={ul}")
            return dl, ul
        except Exception as e:
            log.warning(f"Speedtest failed: {e}")
            return self.state.dl_mbps, self.state.ul_mbps
