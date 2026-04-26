from fastapi import APIRouter, HTTPException, Header, Depends
from .models import UserRegister, UserLogin, AuthResponse, ProfileResponse, ErrorResponse
from .database import supabase, verify_api_key
import uuid
from passlib.context import CryptContext

router = APIRouter(prefix="/auth", tags=["Authentication"])
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

def get_password_hash(password):
    return pwd_context.hash(password)

def verify_password(plain_password, hashed_password):
    return pwd_context.verify(plain_password, hashed_password)

@router.post("/register", response_model=AuthResponse)
async def register(user: UserRegister):
    # Check if user exists
    existing = supabase.table("users").select("*").eq("username", user.username).execute()
    if existing.data:
        raise HTTPException(status_code=409, detail="Username already taken")
    
    user_id = f"usr_{uuid.uuid4().hex[:8]}"
    api_key = f"wz_{uuid.uuid4().hex}"
    hashed_password = get_password_hash(user.password)
    
    new_user = {
        "id": user_id,
        "username": user.username,
        "password_hash": hashed_password,
        "api_key": api_key,
        "avatar_url": user.avatar_url,
        "elo": 1200,
        "wins": 0,
        "losses": 0,
        "total_matches": 0
    }
    
    res = supabase.table("users").insert(new_user).execute()
    if not res.data:
        raise HTTPException(status_code=500, detail="Failed to create user")
    
    return AuthResponse(
        success=True,
        user_id=user_id,
        api_key=api_key,
        avatar_url=user.avatar_url
    )

@router.post("/login", response_model=AuthResponse)
async def login(user: UserLogin):
    res = supabase.table("users").select("*").eq("username", user.username).execute()
    if not res.data:
        raise HTTPException(status_code=401, detail="Invalid username or password")
    
    db_user = res.data[0]
    if not verify_password(user.password, db_user["password_hash"]):
        raise HTTPException(status_code=401, detail="Invalid username or password")
    
    return AuthResponse(
        success=True,
        user_id=db_user["id"],
        api_key=db_user["api_key"],
        avatar_url=db_user["avatar_url"]
    )

async def get_current_user(x_api_key: str = Header(...)):
    user = await verify_api_key(x_api_key)
    if not user:
        raise HTTPException(status_code=401, detail="Invalid API key")
    return user

@APIRouter().get("/profile", response_model=ProfileResponse, tags=["Profile"])
async def get_profile(current_user: dict = Depends(get_current_user)):
    return ProfileResponse(
        user_id=current_user["id"],
        username=current_user["username"],
        avatar_url=current_user.get("avatar_url"),
        elo=current_user["elo"],
        total_matches=current_user["total_matches"],
        wins=current_user["wins"],
        losses=current_user["losses"],
        created_at=current_user["created_at"]
    )
