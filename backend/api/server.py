"""
Blip Terminal Backend - FastAPI server
Exposes a token-protected /api/status endpoint polled by the ESP32.
"""

from dataclasses import asdict

from fastapi import FastAPI, Header, HTTPException, Query, Request
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


def _require_status_token(
    request: Request,
    path_token: str | None,
    header_token: str | None,
    query_token: str | None,
) -> None:
    expected = getattr(request.app.state, "status_api_token", "")
    supplied = path_token or header_token or query_token
    if not expected or supplied != expected:
        raise HTTPException(status_code=404, detail="Not Found")


@app.get("/api/status")
@app.get("/api/status/{path_token}")
async def get_status(
    request: Request,
    path_token: str | None = None,
    x_api_key: str | None = Header(default=None),
    key: str | None = Query(default=None),
):
    """Return the full status payload consumed by the ESP32."""
    _require_status_token(request, path_token, x_api_key, key)
    return JSONResponse(content=asdict(state))


@app.get("/health")
async def health():
    return {"ok": True}
