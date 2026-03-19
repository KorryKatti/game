# Wizard Duel

C++ Game with Raylib and ENet.
Source: https://github.com/korrykatti/game

## Build with Nix
1. Run `nix develop` (or `direnv allow`).
2. Run `make run`.

## Build Manually
Dependencies: `raylib`, `enet`, `openssl`, `libX11`, `libGL`.

Ubuntu/Debian:
```bash
sudo apt install build-essential libraylib-dev libenet-dev libssl-dev libx11-dev libxcursor-dev libxinerama-dev libxi-dev libxrandr-dev
```

Steps:
1. `make`
2. `./build/game`

## Controls
- WASD: Move
- 1 / 2: Select Spell
- Left Click: Shoot
