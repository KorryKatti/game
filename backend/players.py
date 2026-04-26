from fastapi import APIRouter, HTTPException, Header, Depends
from .models import HeartbeatRequest, PlayerSearchResponse, OnlinePlayersResponse, OnlinePlayer
from .database import supabase
from .auth import get_current_user
from datetime import datetime, timedelta

router = APIRouter(prefix="/players", tags=["Players"])

@router.post("/heartbeat")
async def heartbeat(req: HeartbeatRequest, current_user: dict = Depends(get_current_user)):
    data = {
        "user_id": current_user["id"],
        "game_ip": req.game_ip,
        "game_port": req.game_port,
        "status": req.status,
        "last_seen": datetime.utcnow().isoformat()
    }
    # upsert session
    res = supabase.table("sessions").upsert(data).execute()
    return {"success": True}

@router.get("/find", response_model=PlayerSearchResponse)
async def find_player(username: str, current_user: dict = Depends(get_current_user)):
    res = supabase.table("users").select("*, sessions(*)").eq("username", username).execute()
    if not res.data:
        return PlayerSearchResponse(found=False)
    
    player = res.data[0]
    session = player.get("sessions")
    if session and isinstance(session, list):
        session = session[0] if session else None
    
    is_online = False
    is_ingame = False
    game_ip = None
    game_port = None
    
    if session:
        last_seen = datetime.fromisoformat(session["last_seen"].replace("Z", "+00:00"))
        if datetime.utcnow().replace(tzinfo=None) - last_seen.replace(tzinfo=None) < timedelta(minutes=5):
            is_online = True
            is_ingame = session["status"] == "ingame"
            game_ip = session["game_ip"]
            game_port = session["game_port"]
            
    return PlayerSearchResponse(
        found=True,
        username=player["username"],
        avatar_url=player.get("avatar_url"),
        game_ip=game_ip,
        game_port=game_port,
        is_online=is_online,
        is_ingame=is_ingame
    )

@router.get("/online", response_model=OnlinePlayersResponse)
async def get_online_players(current_user: dict = Depends(get_current_user)):
    # Players seen in last 5 minutes
    cutoff = (datetime.utcnow() - timedelta(minutes=5)).isoformat()
    res = supabase.table("sessions").select("user_id, users(username, avatar_url, elo)").gt("last_seen", cutoff).execute()
    
    players = []
    seen_ids = set()
    for s in res.data:
        user = s.get("users")
        u_id = s.get("user_id")
        if user and u_id not in seen_ids:
            players.append(OnlinePlayer(
                username=user["username"],
                avatar_url=user.get("avatar_url"),
                elo=user["elo"]
            ))
            seen_ids.add(u_id)
            
    return OnlinePlayersResponse(players=players, count=len(players))
