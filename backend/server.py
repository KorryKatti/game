from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse
from . import auth, matches, players, leaderboard
import uvicorn
import os

app = FastAPI(
    title="Wizard Duel Backend",
    version="2.0",
    docs_url="/docs",
    redoc_url="/redoc"
)

# Exception handler for consistent error responses as per docs
@app.exception_handler(Exception)
async def global_exception_handler(request: Request, exc: Exception):
    status_code = 500
    if hasattr(exc, "status_code"):
        status_code = exc.status_code
    
    return JSONResponse(
        status_code=status_code,
        content={
            "error": str(exc.detail) if hasattr(exc, "detail") else str(exc),
            "code": status_code
        }
    )

# Profile endpoint as per apidocs (GET /profile)
@app.get("/profile", tags=["Profile"])
async def profile(user=auth.Depends(auth.get_current_user)):
    return {
        "user_id": user["id"],
        "username": user["username"],
        "avatar_url": user.get("avatar_url"),
        "elo": user["elo"],
        "total_matches": user["total_matches"],
        "wins": user["wins"],
        "losses": user["losses"],
        "created_at": user["created_at"]
    }

# Include routers
app.include_router(auth.router)
app.include_router(matches.router)
app.include_router(players.router)
app.include_router(leaderboard.router)

@app.get("/")
async def root():
    return {"message": "Wizard Duel Central Server v2.0", "status": "online"}

if __name__ == "__main__":
    port = int(os.getenv("PORT", 3000))
    uvicorn.run("backend.server:app", host="0.0.0.0", port=port, reload=True)
