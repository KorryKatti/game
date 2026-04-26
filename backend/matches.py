from fastapi import APIRouter, HTTPException, Header, Depends, UploadFile, File, Form
from fastapi.responses import Response
from .models import MatchUploadResponse, MatchHistoryResponse, MatchDetailsResponse, MatchHistoryItem
from .database import supabase
from .auth import get_current_user
import uuid
import os

router = APIRouter(prefix="/matches", tags=["Matches"])

@router.post("", response_model=MatchUploadResponse)
async def upload_match_result(
    opponent_name: str = Form(...),
    winner: str = Form(...),
    duration_sec: int = Form(...),
    match_file: UploadFile = File(...),
    current_user: dict = Depends(get_current_user)
):
    match_id = f"mch_{uuid.uuid4().hex[:8]}"
    
    # Save file to Supabase Storage
    # Assuming bucket "match-files"
    file_path = f"{current_user['id']}/{match_id}/{match_file.filename}"
    file_content = await match_file.read()
    
    # try to upload
    try:
        supabase.storage.from_("match-files").upload(file_path, file_content)
    except Exception as e:
        # Fallback if storage fails/not configured
        print(f"Storage upload error: {e}")
    
    # Store match metadata
    match_data = {
        "id": match_id,
        "player_id": current_user["id"],
        "opponent_name": opponent_name,
        "winner": current_user["username"] if winner == "me" else opponent_name,
        "duration": duration_sec,
        "match_file_path": file_path
    }
    
    supabase.table("matches").insert(match_data).execute()
    
    # Update user stats (simple logic)
    new_total = current_user["total_matches"] + 1
    new_wins = current_user["wins"] + (1 if winner == "me" else 0)
    new_losses = current_user["losses"] + (0 if winner == "me" else 1)
    # Simple ELO change
    elo_change = 25 if winner == "me" else -15
    new_elo = current_user["elo"] + elo_change
    
    supabase.table("users").update({
        "total_matches": new_total,
        "wins": new_wins,
        "losses": new_losses,
        "elo": new_elo
    }).eq("id", current_user["id"]).execute()
    
    return MatchUploadResponse(success=True, match_id=match_id)

@router.get("", response_model=MatchHistoryResponse)
async def get_match_history(
    limit: int = 20, 
    offset: int = 0, 
    current_user: dict = Depends(get_current_user)
):
    res = supabase.table("matches").select("*").eq("player_id", current_user["id"]).order("created_at", desc=True).range(offset, offset + limit - 1).execute()
    total_res = supabase.table("matches").select("id", count="exact").eq("player_id", current_user["id"]).execute()
    
    matches = [
        MatchHistoryItem(
            match_id=m["id"],
            opponent=m["opponent_name"],
            result="win" if m["winner"] == current_user["username"] else "loss",
            duration=m["duration"],
            played_at=m["created_at"]
        ) for m in res.data
    ]
    
    return MatchHistoryResponse(matches=matches, total=total_res.count or 0)

@router.get("/{match_id}", response_model=MatchDetailsResponse)
async def get_match_details(match_id: str, current_user: dict = Depends(get_current_user)):
    res = supabase.table("matches").select("*").eq("id", match_id).execute()
    if not res.data:
        raise HTTPException(status_code=404, detail="Match not found")
    
    m = res.data[0]
    # Simple signed URL or direct URL
    download_url = f"/matches/{match_id}/download" # Internal redirect for simplicity
    
    return MatchDetailsResponse(
        match_id=m["id"],
        player=current_user["username"], # This assumes the requester is a player in the match
        opponent=m["opponent_name"],
        winner=m["winner"],
        duration=m["duration"],
        download_url=download_url
    )

@router.get("/{match_id}/download")
async def download_match_file(match_id: str, current_user: dict = Depends(get_current_user)):
    res = supabase.table("matches").select("*").eq("id", match_id).execute()
    if not res.data:
        raise HTTPException(status_code=404, detail="Match not found")
    
    m = res.data[0]
    try:
        file_res = supabase.storage.from_("match-files").download(m["match_file_path"])
        return Response(content=file_res, media_type="application/octet-stream")
    except Exception as e:
        raise HTTPException(status_code=500, detail="Failed to download file")
