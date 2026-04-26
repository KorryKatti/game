import os
from supabase import create_client, Client
from dotenv import load_dotenv

load_dotenv()

SUPABASE_URL = os.getenv("SUPABASE_URL")
SUPABASE_KEY = os.getenv("SUPABASE_KEY")

if not SUPABASE_URL or not SUPABASE_KEY:
    print("Warning: SUPABASE_URL and SUPABASE_KEY are not set.")

supabase: Client = create_client(SUPABASE_URL or "", SUPABASE_KEY or "")

# Helper to verify API Key
async def verify_api_key(api_key: str):
    res = supabase.table("users").select("*").eq("api_key", api_key).execute()
    if res.data:
        return res.data[0]
    return None
