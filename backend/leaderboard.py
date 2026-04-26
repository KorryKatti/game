from fastapi import APIRouter, HTTPException, Header, Depends
from .models import LeaderboardResponse, LeaderboardEntry
from .database import supabase
from .auth import verify_api_key
from typing import Optional

router = APIRouter(prefix="/leaderboard", tags=["Leaderboard"])

@router.get("", response_model=LeaderboardResponse)
async def get_leaderboard(limit: int = 50, x_api_key: Optional[str] = Header(None)):
    # Auth is optional for leaderboard in docs
    res = supabase.table("users").select("username, elo, wins, avatar_url").order("elo", desc=True).limit(limit).execute()
    
    entries = [
        LeaderboardEntry(
            rank=i+1,
            username=u["username"],
            elo=u["elo"],
            wins=u["wins"],
            avatar_url=u.get("avatar_url")
        ) for i, u in enumerate(res.data)
    ]
    
    return LeaderboardResponse(players=entries)
