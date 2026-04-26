# Wizard Duel Backend API Docs v2.0

## Base URL
`https://api.wizardduel.com/v1`

## Authentication
`X-API-Key: your_api_key`

---

## 1. Account Management

### Register
`POST /auth/register`
```json
{
  "username": "Merlin",
  "password": "secret123",
  "avatar_url": "https://imgur.com/merlin.png"  // optional
}
```
**Response:**
```json
{
  "success": true,
  "user_id": "usr_abc123",
  "api_key": "wz_xyz789..."
}
```

### Login
`POST /auth/login`
```json
{
  "username": "Merlin",
  "password": "secret123"
}
```
**Response:**
```json
{
  "success": true,
  "user_id": "usr_abc123",
  "api_key": "wz_xyz789...",
  "avatar_url": "https://imgur.com/merlin.png"
}
```

### Get Profile
`GET /profile`
**Headers:** `X-API-Key`
**Response:**
```json
{
  "user_id": "usr_abc123",
  "username": "Merlin",
  "avatar_url": "https://imgur.com/merlin.png",
  "elo": 1200,
  "total_matches": 42,
  "wins": 28,
  "losses": 14,
  "created_at": "2024-01-15T10:00:00Z"
}
```

---

## 2. Match Operations

### Upload Match Result
`POST /matches`
**Headers:** `X-API-Key`
**Multipart Form:**
| Field | Type |
|-------|------|
| `match_file` | `.wzrd` file |
| `opponent_name` | string |
| `winner` | "me" or "opponent" |
| `duration_sec` | integer |

**Response:**
```json
{
  "success": true,
  "match_id": "mch_abc123"
}
```

### Get Match History
`GET /matches?limit=20&offset=0`
**Headers:** `X-API-Key`
**Response:**
```json
{
  "matches": [
    {
      "match_id": "mch_abc123",
      "opponent": "Morgana",
      "result": "win",
      "duration": 187,
      "played_at": "2024-01-15T14:30:22Z"
    }
  ],
  "total": 42
}
```

### Get Match Details (for replay)
`GET /matches/{match_id}`
**Headers:** `X-API-Key`
**Response:**
```json
{
  "match_id": "mch_abc123",
  "player": "Merlin", 
  "opponent": "Morgana",
  "winner": "Merlin",
  "duration": 187,
  "download_url": "https://api.wizardduel.com/matches/mch_abc123/download"
}
```

### Download Match File
`GET /matches/{match_id}/download`
**Headers:** `X-API-Key`
**Response:** `.wzrd` file download

---

## 3. Player Discovery

### Update Online Status
`POST /players/heartbeat`
**Headers:** `X-API-Key`
```json
{
  "game_ip": "192.168.1.100",
  "game_port": 7777,
  "status": "online"  // online, ingame, offline
}
```

### Find Player
`GET /players/find?username=Morgana`
**Headers:** `X-API-Key`
**Response:**
```json
{
  "found": true,
  "username": "Morgana",
  "avatar_url": "https://imgur.com/morgana.png",
  "game_ip": "192.168.1.50",
  "game_port": 7777,
  "is_online": true,
  "is_ingame": false
}
```

### Get Online Players
`GET /players/online`
**Headers:** `X-API-Key`
**Response:**
```json
{
  "players": [
    {
      "username": "Merlin",
      "avatar_url": "https://imgur.com/merlin.png",
      "elo": 1200
    },
    {
      "username": "Morgana", 
      "avatar_url": "https://imgur.com/morgana.png",
      "elo": 1150
    }
  ],
  "count": 2
}
```

---

## 4. Leaderboard

`GET /leaderboard?limit=50`
**Headers:** optionally `X-API-Key`
**Response:**
```json
{
  "players": [
    {
      "rank": 1,
      "username": "Merlin",
      "elo": 1450,
      "wins": 28,
      "avatar_url": "https://imgur.com/merlin.png"
    }
  ]
}
```

---

## Error Responses

All errors return:
```json
{
  "error": "Description of what went wrong",
  "code": 400
}
```

**Common codes:**
- `400` - Bad request (missing fields)
- `401` - Invalid API key
- `404` - Player/match not found
- `409` - Username already taken

---

## Storage Requirements (Minimal)

**Supabase/MongoDB collections:**
- `users`: id, username, password_hash, api_key, avatar_url, elo, created_at
- `matches`: id, player_id, opponent_name, winner, duration, timestamp, match_file_path
- `sessions`: user_id, game_ip, game_port, status, last_seen

**File storage:**
- `.wzrd` files in cloud storage (Supabase Storage / S3 / local disk)
- Or store as Base64 in MongoDB (if small)

**Estimated per user:** ~1KB metadata + ~50KB per match file