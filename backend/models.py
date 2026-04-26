from pydantic import BaseModel, HttpUrl
from typing import Optional, List
from datetime import datetime

class UserRegister(BaseModel):
    username: str
    password: str
    avatar_url: Optional[str] = None

class UserLogin(BaseModel):
    username: str
    password: str

class AuthResponse(BaseModel):
    success: bool
    user_id: str
    api_key: str
    avatar_url: Optional[str] = None

class ProfileResponse(BaseModel):
    user_id: str
    username: str
    avatar_url: Optional[str] = None
    elo: int
    total_matches: int
    wins: int
    losses: int
    created_at: datetime

class MatchUploadResponse(BaseModel):
    success: bool
    match_id: str

class MatchHistoryItem(BaseModel):
    match_id: str
    opponent: str
    result: str
    duration: int
    played_at: datetime

class MatchHistoryResponse(BaseModel):
    matches: List[MatchHistoryItem]
    total: int

class MatchDetailsResponse(BaseModel):
    match_id: str
    player: str
    opponent: str
    winner: str
    duration: int
    download_url: str

class HeartbeatRequest(BaseModel):
    game_ip: str
    game_port: int
    status: str  # online, ingame, offline

class PlayerSearchResponse(BaseModel):
    found: bool
    username: Optional[str] = None
    avatar_url: Optional[str] = None
    game_ip: Optional[str] = None
    game_port: Optional[str] = None
    is_online: Optional[bool] = None
    is_ingame: Optional[bool] = None

class OnlinePlayer(BaseModel):
    username: str
    avatar_url: Optional[str] = None
    elo: int

class OnlinePlayersResponse(BaseModel):
    players: List[OnlinePlayer]
    count: int

class LeaderboardEntry(BaseModel):
    rank: int
    username: str
    elo: int
    wins: int
    avatar_url: Optional[str] = None

class LeaderboardResponse(BaseModel):
    players: List[LeaderboardEntry]

class ErrorResponse(BaseModel):
    error: str
    code: int
