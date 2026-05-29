"""Shared state container updated by collectors, read by API."""

from dataclasses import dataclass, field
from typing import List, Dict, Any


@dataclass
class AppState:
    # ---- System ----
    cpu: int = 0
    cpu_temp: float = 0.0
    ram: int = 0
    gpu: int = 0
    gpu_temp: float = 0.0
    vram: int = 0
    disk: int = 0
    net_down_mbps: float = 0.0
    net_up_mbps: float = 0.0

    # ---- Blip AI / OpenClaw ----
    ai_model: str = "---"
    ai_tpm: int = 0
    ai_conversations: int = 0
    ai_latency_ms: int = 0
    ai_uptime: str = "---"
    ai_mem_pct: int = 0
    ai_status: str = "OFFLINE"

    # ---- Internet ----
    weather: str = "---"
    weather_icon: str = "?"
    external_ip: str = "---"
    latency_ms: int = 0
    dl_mbps: float = 0.0
    ul_mbps: float = 0.0

    # ---- Discord ----
    dc_members: int = 0
    dc_online: int = 0
    dc_msgs_today: int = 0
    dc_voice_users: int = 0

    # ---- OpenClaw ops ----
    oc_agents: int = 0
    oc_tasks: int = 0
    oc_current_tool: str = "---"
    oc_queue_len: int = 0
    oc_last_task: str = "---"
    oc_log: List[str] = field(default_factory=list)

    # ---- Network scan ----
    wifi_networks: List[Dict[str, Any]] = field(default_factory=list)
    ble_devices: List[Dict[str, Any]] = field(default_factory=list)
    lan_devices: int = 0
    ping_ms: int = 0
