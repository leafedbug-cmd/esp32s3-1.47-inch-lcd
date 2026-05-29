"""System metrics collector — CPU, RAM, GPU, disk, network."""

import asyncio
import logging

import psutil

from collectors import BaseCollector

log = logging.getLogger("blip.system")

# Optional GPU support via GPUtil
try:
    import GPUtil
    _HAS_GPU = True
except ImportError:
    _HAS_GPU = False
    log.warning("GPUtil not available — GPU metrics will read 0")


class SystemCollector(BaseCollector):
    def __init__(self, state, interval: float):
        super().__init__(state, interval)
        self._net_last = psutil.net_io_counters()
        self._net_time = asyncio.get_event_loop().time()

    async def collect(self):
        loop = asyncio.get_event_loop()

        # CPU (non-blocking — psutil.cpu_percent(interval=None) reads cached value)
        self.state.cpu = int(psutil.cpu_percent(interval=None))

        # CPU temperature (platform-dependent)
        self.state.cpu_temp = _get_cpu_temp()

        # RAM
        mem = psutil.virtual_memory()
        self.state.ram = int(mem.percent)

        # Disk (root partition)
        disk = psutil.disk_usage("/")
        self.state.disk = int(disk.percent)

        # Network throughput (delta since last call)
        net_now  = psutil.net_io_counters()
        t_now    = loop.time()
        dt       = max(t_now - self._net_time, 0.001)

        bytes_down = net_now.bytes_recv - self._net_last.bytes_recv
        bytes_up   = net_now.bytes_sent - self._net_last.bytes_sent

        self.state.net_down_mbps = round((bytes_down / dt) / 1e6 * 8, 2)
        self.state.net_up_mbps   = round((bytes_up   / dt) / 1e6 * 8, 2)

        self._net_last = net_now
        self._net_time = t_now

        # GPU via GPUtil
        if _HAS_GPU:
            try:
                gpus = GPUtil.getGPUs()
                if gpus:
                    g = gpus[0]
                    self.state.gpu      = int(g.load * 100)
                    self.state.gpu_temp = g.temperature
                    # VRAM usage percent
                    if g.memoryTotal > 0:
                        self.state.vram = int(g.memoryUsed / g.memoryTotal * 100)
            except Exception as e:
                log.debug(f"GPU read error: {e}")


def _get_cpu_temp() -> float:
    """Best-effort CPU temperature, cross-platform."""
    try:
        temps = psutil.sensors_temperatures()
        if not temps:
            return 0.0
        # Try common keys in priority order
        for key in ("coretemp", "cpu_thermal", "k10temp", "acpitz"):
            if key in temps:
                readings = temps[key]
                if readings:
                    return round(readings[0].current, 1)
        # Fallback: first available sensor
        for readings in temps.values():
            if readings:
                return round(readings[0].current, 1)
    except Exception:
        pass
    return 0.0
