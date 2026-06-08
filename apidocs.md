# Wizard Duel - Central Server API Documentation

This document explains the architecture and API of the Wizard Duel central server (`server.py`), which handles authentication, match recordings, player discovery, and game data.

## Overview

The server is built using **FastAPI**, a modern, high-performance web framework for Python. It serves as:
- **Authentication Hub**: Validates API keys and manages player accounts.
- **Match Repository**: Centralizes match logs (`.wzrd` files) from game clients.
- **Player Discovery**: Provides online player listings for matchmaking.
- **Heartbeat Monitor**: Tracks which players are currently online and available.

### Repository Structure
- **Game Clients**: (C++) Authenticate via API key, record match events locally, report online status via heartbeats, and upload completed matches via `GameClient`.
- **Web Server**: (Python/FastAPI) Handles authentication, player status, and match uploads.

---

## API Endpoints

### 1. Authentication

#### `/v1/profile` (GET)
**Purpose**: Validate API key and retrieve player profile.

**Headers**:
```
Authorization: Bearer <api_key>
```

**Response** (200 OK):
```json
{
  "username": "player_name",
  "elo": 1200,
  "avatar_url": "https://..."
}
```

**Used by**: Game client on startup to verify API key validity.

---

#### `/v1/players/heartbeat` (POST)
**Purpose**: Report player online status and connection details.

**Headers**:
```
Authorization: Bearer <api_key>
Content-Type: application/json
```

**Request Body**:
```json
{
  "game_ip": "203.0.113.42",
  "game_port": 7777,
  "status": "online"
}
```

**Response** (200 OK):
```json
{
  "success": true,
  "message": "Heartbeat received"
}
```

**Used by**: 
- Host player calls `goOnline()` when starting a game.
- Updates every 30 seconds via `updateHeartbeat()` timer during gameplay.
- Sends `"status": "offline"` when exiting.

---

#### `/v1/players/online` (GET)
**Purpose**: Retrieve list of currently online players available for matchmaking.

**Headers**:
```
Authorization: Bearer <api_key>
```

**Response** (200 OK):
```json
{
  "players": [
    {
      "username": "Wizard_Alpha",
      "game_ip": "203.0.113.42",
      "game_port": 7777,
      "elo": 1350,
      "avatar_url": "https://..."
    },
    {
      "username": "Wizard_Beta",
      "game_ip": "198.51.100.8",
      "game_port": 7777,
      "elo": 1200,
      "avatar_url": "https://..."
    }
  ]
}
```

**Used by**: 
- Game client calls `getOnlinePlayers()` when user clicks "FIND PLAYERS".
- Displays in `PLAYER_LIST` state for matchmaking selection.

---

### 2. Match Recording

#### `/v1/matches` (POST)
**Purpose**: Upload completed match recording and metadata.

#### `/v1/matches` (POST)
**Purpose**: Upload completed match recording and metadata.

**Headers**:
```
Authorization: Bearer <api_key>
Content-Type: multipart/form-data
```

#### Form Fields:
| Field | Type | Description |
|-------|------|-------------|
| `opponent_name` | `string` | Name of the opponent. |
| `winner` | `string` | Name of the winning player. |
| `duration` | `string` | Match duration in seconds. |
| `match_id` | `string` | A unique ID (timestamp) shared by both players to group recordings. |
| `match_file`| `file` | The actual `.wzrd` log file. |

**Response** (200 OK):
```json
{
  "success": true,
  "match_id": "1719327600",
  "message": "Match recorded successfully"
}
```

**Called by**: 
- Game client at the end of a match (both SINGLE and MULTIPLAYER modes).
- Uses `g_gameClient.uploadMatch()` instead of async upload.
- Stores match in `recordings/<match_id>/` with both players' perspective files.

---

## Game Client Integration (`GameClient` Class)

The `GameClient` class in the C++ game client encapsulates all server communication:

### Initialization & Authentication
```cpp
GameClient g_gameClient;  // Created at game startup
```

1. **API Key Loading**: On startup, `loadAPIKey()` reads stored key from `wizard_api_key.txt`.
2. **Validation**: User enters API key in menu → `validateAndLogin(key)` verifies via `/v1/profile`.
3. **Storage**: Upon success, key is saved locally via `saveAPIKey()`.

### Online Player Discovery
**Menu Flow**: User clicks "FIND PLAYERS" → Game calls:
```cpp
auto players = g_gameClient.getOnlinePlayers();
```
This fetches the current list from `/v1/players/online` and displays in `PLAYER_LIST` state.

### Heartbeat System
**In Main Loop**:
```cpp
float deltaTime = GetFrameTime();
UIHelper::updateHeartbeat(deltaTime, g_gameClient);
```

- Timer increments by `deltaTime` each frame.
- Every 30 seconds, `client.heartbeat()` sends a POST to `/v1/players/heartbeat`.
- Signals server: "This player is still online at IP:PORT".

### Going Online
**Host Button Flow**:
```cpp
g_gameClient.goOnline(SERVER_PORT);
state = "HOST";
```

- Fetches public IP via `fetchPublicIP()`.
- Sends to `/v1/players/heartbeat` with `"status": "online"`.
- Now visible in `/v1/players/online` list for other players to discover.

### Match Upload
**End of Match**:
```cpp
g_gameClient.uploadMatch(
    g_recorder.getFilename(),    // path to .wzrd file
    g_player2_name,               // opponent name
    winner_name,                  // winning player
    (int)getMatchDuration(),      // duration in seconds
    g_recorder.getMatchId()       // shared match ID
);
```

- Multipart POST to `/v1/matches`.
- Server groups both players' uploads by `match_id`.

---

## Building a "Proper" Central Server

To transform the current `server.py` into a full-scale game backend, you should consider these components:

### 1. Database Integration
Instead of just printing logs to the terminal, use a database (like **Postgres** or **MongoDB**) to store match metadata (who won, how long it lasted, etc.).
- **Tools**: SQLAlchemy, Tortoise ORM.

### 2. Match Analysis Service
Create a background task that parses the `.wzrd` CSV file after it is uploaded.
- **Goal**: Calculate advanced stats like "Accuracy %", "Most used spell", or "Highest burst damage".
- **Benefit**: Display these on a web dashboard for players.

### 3. Replay System (The "Killer Feature")
Build a web-based replay viewer using JavaScript (or Three.js if you want 3D).
- **How**: The web app reads the CSV from the server and "plays back" the events (positions, spells) on an HTML5 canvas.

### 4. Authentication & Profiles
Implement JWT (JSON Web Tokens) so players can log in.
- **Result**: `recordings/user_id/match_id/` structure.
- **Benefit**: Match history, friend lists, and global leaderboards.

---

## Future Enhancements Suggestions

### 🟢 Short Term (Easy)
- **Automatic Summary**: Instead of just saving the file, have the server return a "Match Summary" JSON with calculated stats (accuracy, spell usage, damage dealt).
- **Match IDs as UUIDs**: Upgrade from timestamps to UUID v4 for collision-free match identification across servers.
- **Persistent Profiles**: Store ELO ratings and player stats in a database.

### 🟡 Medium Term (Moderate)
- **Web Dashboard**: A React/Vue frontend to list recent matches and view winners, player stats, and match replays.
- **Discord Webhooks**: Post match results: "Wizard_Alpha defeated Wizard_Beta in 45 seconds!"
- **Automatic Matchmaking Queue**: `/v1/matchmaking/queue` endpoint to pair players by ELO rating.
- **Replay Viewer**: Parse `.wzrd` CSV and render match playback on HTML5 canvas.

### 🔴 Long Term (Advanced)
- **Anti-Cheat Verification**: Server simulates match using logs to detect impossible actions (teleporting, infinite health).
- **Seasonal Leaderboards**: Track win rates, seasonal rankings, achievement unlocks.
- **Friend Lists & Social**: Add friend system, spectate live matches, clan/team support.

---

## Example Usage (C++ GameClient)
```cpp
// Authenticate
if (g_gameClient.validateAndLogin(apiKeyFromUser)) {
    // Success - key saved
}

// Go online (as host)
g_gameClient.goOnline(7777);

// Get available players
auto players = g_gameClient.getOnlinePlayers();

// Heartbeat handled automatically each frame
UIHelper::updateHeartbeat(deltaTime, g_gameClient);

// After match ends
g_gameClient.uploadMatch(
    matchFileName,
    opponentName,
    winnerName,
    durationSeconds,
    matchId
);
```

---

## Example Usage (Python HTTP Client)
```python
import requests

# Validate API key
headers = {'Authorization': f'Bearer {api_key}'}
response = requests.get('http://localhost:3000/v1/profile', headers=headers)

# Get online players
response = requests.get('http://localhost:3000/v1/players/online', headers=headers)
print(response.json()['players'])

# Send heartbeat
data = {
    'game_ip': '203.0.113.42',
    'game_port': 7777,
    'status': 'online'
}
response = requests.post(
    'http://localhost:3000/v1/players/heartbeat',
    json=data,
    headers=headers
)

# Upload match
files = {'match_file': open('duel_log.wzrd', 'rb')}
data = {
    'opponent_name': 'Wizard_Beta',
    'winner': 'Wizard_Alpha',
    'duration': '120',
    'match_id': '1719327600'
}
response = requests.post(
    'http://localhost:3000/v1/matches',
    data=data,
    files=files,
    headers=headers
)
print(response.json())
```

---

## How to Build Your Own Match Server

If you want to rewrite or expand the server from scratch, follow these key steps:

### 1. Choose a Framework
For Python, **FastAPI** is recommended:
- Handles async file uploads and JWT authentication.
- Built-in validation via Pydantic.
- Auto-generates OpenAPI (Swagger) docs at `/docs`.
- Alternatives: Flask (simpler but single-threaded), Django (overkill).

### 2. Implement Authentication Layer
Use **JWT (JSON Web Tokens)** or **API Keys**:
- Store hashed API keys in a database.
- `/v1/profile` endpoint validates the key and returns user info.
- All endpoints require `Authorization: Bearer <api_key>` header.
- Use FastAPI's `Depends()` for middleware validation.

### 3. Player Status Tracking
Implement heartbeat mechanism:
- `/v1/players/heartbeat` (POST) updates player's online status, IP, and port.
- Store in-memory or Redis cache with expiration (e.g., 5 minutes).
- `/v1/players/online` filters active players from cache.
- Consider: ELO rating, region, game mode filtering.

### 4. Handle Multipart Match Uploads
Your `/v1/matches` endpoint should:
1. **Validate**: Ensure all fields (`opponent_name`, `match_id`, etc.) are present.
2. **Directory Management**: Use `os.makedirs(path, exist_ok=True)` to create `recordings/<match_id>/`.
3. **Stream Saving**: Use `shutil.copyfileobj()` to pipe file directly to disk (avoid RAM overhead).
4. **Duplicate Handling**: Check if both players' files exist before final recording.

### 5. Grouping by Match ID
The core trick to organizing multi-player data:
- **The Concept**: Host generates a unique ID (timestamp or UUID) and shares it with client during connection.
- **The Result**: Server receives uploads from different IPs with the same `match_id` → groups them in one folder.
- **Verification**: Once both files arrive, enable replay playback or advanced statistics.

### 6. Deployment
Use a production runner:
- **Uvicorn**: `uvicorn server:app --host 0.0.0.0 --port 3000 --workers 4`
- **Gunicorn**: `gunicorn -w 4 -k uvicorn.workers.UvicornWorker server:app`
- Handles multiple simultaneous uploads without blocking.
- Add SSL/TLS via **nginx** reverse proxy or **Let's Encrypt**.

### 7. Database Integration (Recommended)
Replace in-memory heartbeat cache with persistent storage:
```python
from sqlalchemy import Column, String, Integer, Float, DateTime
from datetime import datetime, timedelta

class Player(Base):
    __tablename__ = "players"
    api_key = Column(String, primary_key=True)
    username = Column(String, unique=True)
    elo = Column(Integer, default=1200)
    game_ip = Column(String)
    game_port = Column(Integer)
    last_heartbeat = Column(DateTime, default=datetime.utcnow)
    
    @property
    def is_online(self):
        return (datetime.utcnow() - self.last_heartbeat) < timedelta(minutes=5)
```

---

## Architecture Diagram

```
┌──────────────────────────────────────┐
│  Game Client (C++ / Raylib)          │
│  ┌────────────────────────────────┐  │
│  │  GameClient Class              │  │
│  │ • validateAndLogin()           │  │
│  │ • goOnline() / goOffline()     │  │
│  │ • getOnlinePlayers()           │  │
│  │ • uploadMatch()                │  │
│  │ • heartbeat()                  │  │
│  └────────────────────────────────┘  │
└──────────────┬───────────────────────┘
               │ HTTP/Bearer Token
       ┌───────▼────────────────────────┐
       │ FastAPI Central Server         │
       │ (Python)                       │
       │ ┌─────────────────────────────┐│
       │ │ /v1/profile (GET)           ││
       │ │ /v1/players/heartbeat (POST)││
       │ │ /v1/players/online (GET)    ││
       │ │ /v1/matches (POST)          ││
       │ └─────────────────────────────┘│
       │                                │
       │ [PostgreSQL Database]          │
       │ • Players / Profiles           │
       │ • Match Metadata               │
       │ • ELO Ratings                  │
       │                                │
       │ [File Storage]                 │
       │ • recordings/<match_id>/*.wzrd │
       └────────────────────────────────┘
```
