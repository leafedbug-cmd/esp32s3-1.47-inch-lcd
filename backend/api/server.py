"""
Blip Terminal Backend - FastAPI server
Exposes /api/status as the single JSON endpoint polled by the ESP32.
"""

from dataclasses import asdict

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

from api.state import AppState

app = FastAPI(title="Blip Terminal API", version="1.0.0")
state = AppState()

# Allow local-only CORS for development tooling
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost", "http://127.0.0.1"],
    allow_methods=["GET"],
    allow_headers=["*"],
)


@app.get("/api/status")
async def get_status():
    """Return the full status payload consumed by the ESP32."""
    return JSONResponse(content=asdict(state))


@app.get("/health")
async def health():
    return {"status": "ok"}
