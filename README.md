# Wizard Duel

A 1v1 wizard duel game built with C++, Raylib, and ENet.

Two wizards fight in a procedural arena. Shoot fireballs, dodge spells, use trees for cover. Simple to learn, hard to master.

## Status

The game is architecturally complete. Core gameplay, multiplayer networking, match recording, and a companion website (leaderboard, stats, chat) all work. Further polishing is optional.

## Install & Run

### Dependencies

- g++ (C++17)
- raylib
- enet
- openssl
- libX11, libGL

**Ubuntu/Debian:**
```bash
sudo apt install build-essential libraylib-dev libenet-dev libssl-dev libx11-dev libxcursor-dev libxinerama-dev libxi-dev libxrandr-dev
```

### Build & Run

```bash
make run
```

Or step by step:
```bash
make
./build/game
```

### First Launch

1. The game will ask for an API key on first launch.
2. Go to the website, create an account, copy your API key.
3. Paste the key into the game, or put it in a file called `wizard_api_key.txt` next to the game binary.

## Controls

| Key | Action |
|-----|--------|
| WASD | Move |
| 1 + Left Click | Cast Blood Red (high damage, slow) |
| 2 + Left Click | Cast Dark Blue (low damage, fast) |
| Right Click | Mana Shield (heal, costs mana) |
| Z / X | Zoom out / in |
| R | Reset zoom |

## How to Play Multiplayer

### With Someone on the Same Network (LAN)

1. One player clicks **HOST GAME**. The game listens on port 7777.
2. The other player clicks **JOIN GAME**. The host's IP is used to connect.
3. On LAN, the host's local IP (like `192.168.x.x`) works directly.

### With Friends Over the Internet (Tailscale)

The game uses peer-to-peer connections, so both players need to reach each other. The easiest way is [Tailscale](https://tailscale.com) — a free VPN that creates a private network between your devices.

1. Both players install Tailscale and log in (free account works).
2. Both devices appear on a shared virtual network (e.g. `100.x.x.x`).
3. One player clicks **HOST GAME**.
4. The other player clicks **JOIN GAME** and enters the host's Tailscale IP.

No port forwarding needed. Tailscale handles NAT traversal.

### Using Port Forwarding (Advanced)

If you don't want to use Tailscale:

1. The host forwards port **7777** (UDP) on their router to their local IP.
2. The joining player connects to the host's public IP.
3. You can find your public IP at https://api.ipify.org

### Find Players & Chat

Click **FIND PLAYERS** to see who's online. You can:
- **PING** someone to alert them
- **CHAT** to coordinate a game

Both players need to be registered on the website and have entered their API keys.

### Central Server

Right now, multiplayer is peer-to-peer — players connect directly to each other. The website server only handles accounts, stats, and player discovery. If the game gets enough players, a proper central relay server could be added for matchmaking and always-on lobbies.

### Website

The website (leaderboard, stats, chat) is not publicly deployed. Feel free to host your own instance — the server code is in `game-server/`. If you have a good host and want it to become the official server, open an issue on the repo.

## Game Modes

- **Singleplayer** — Fight an AI opponent
- **Host Game** — Host a multiplayer match
- **Join Game** — Connect to a host
- **Find Players** — See who's online, ping or chat with them

## Architecture

```
game/                  Game client (C++/Raylib/ENet)
  src/
    main.cpp           Game loop, states, rendering
    GameClient.h       API client (auth, players, chat, matches)
    UIHelper.h         Menu UI, chat window, player list
    SingleplayerAI.h   AI opponent logic
    MatchRecorder.h    Local match recording
    MatchUploader.h    Upload matches to server
    island_generator.h Procedural island generation

game-server/
  backend/             API server (Bun + Hono + SQLite)
    index.ts           Auth, players, chat, matches, leaderboard
  frontend/            Companion website (Vite)
    src/               Login, dashboard, leaderboard, docs
```

## License

See LICENSE.
