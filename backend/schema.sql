-- Users table
CREATE TABLE users (
    id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    api_key TEXT UNIQUE NOT NULL,
    avatar_url TEXT,
    elo INTEGER DEFAULT 1200,
    wins INTEGER DEFAULT 0,
    losses INTEGER DEFAULT 0,
    total_matches INTEGER DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Matches table
CREATE TABLE matches (
    id TEXT PRIMARY KEY,
    player_id TEXT REFERENCES users(id),
    opponent_name TEXT NOT NULL,
    winner TEXT NOT NULL,
    duration INTEGER NOT NULL,
    match_file_path TEXT,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Sessions (Heartbeat) table
CREATE TABLE sessions (
    user_id TEXT PRIMARY KEY REFERENCES users(id),
    game_ip TEXT,
    game_port INTEGER,
    status TEXT, -- online, ingame, offline
    last_seen TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Index for online players search
CREATE INDEX idx_sessions_last_seen ON sessions(last_seen);
