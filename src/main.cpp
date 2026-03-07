#include "raylib.h"
#include <bits/stdc++.h>
#include <enet/enet.h>

std::string connectionState = "DISCONNECTED";
static int trees_received = 0;
ENetHost *clientHost = nullptr;
ENetPeer *serverPeer = nullptr; // for client connection to server
ENetHost *serverHost = nullptr; // for host server
ENetPeer *clientPeer = nullptr; // for host connection to client
uint32_t next_spell_id = 1;     // For generating unique spell IDs

const char *SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 7777;

// just testing for now
// Packet structure for position updates
// ok a bit of AI assistance was used this , i couldn;t realy figure out enet
enum MessageType {
  MSG_POSITION_UPDATE = 0,
  MSG_SPELL_CAST = 1,
  MSG_HIT_CONFIRM = 2,
  MSG_HEALTH_UPDATE = 3,
  MSG_PLAYER_DEATH = 4,
  MSG_TREE_POSITIONS = 5
};
// struct for all the messages
struct NetworkPacket {
  uint8_t type;
  uint8_t playerId;
};

struct TreePacket : NetworkPacket {
  float x;
  float y;
  uint16_t index;
};

struct PositionPacket : NetworkPacket {
  float x;
  float y;
};

// spell casting packet
struct SpellCastPacket : NetworkPacket {
  float target_x;
  float target_y;
  uint8_t spell_type;
  float pos_x;
  float pos_y;
  uint32_t spell_id;
};

// hit confirm packet
struct HitPacket : NetworkPacket {
  float damage;
  uint32_t spell_id;
  uint8_t target_player;
};

struct HealthPacket : NetworkPacket {
  float health;
  float mana;
};

struct Character {
  float health = 300.0f;
  float mana = 300.0f;
  double health_timer = 0.0;
  bool draining_mana = false;
  bool is_cast = false;
  float deathTime = 0.0f;
  bool isDead = false;
  bool deathSoundPlayed = false;
  bool is_local = false; // to check if its a local character or opponent
  Vector2 pos = {};
  Rectangle rect = {0, 0, 20, 40};
  uint8_t id = 0; // network id
};

Color bloodRed = {128, 0, 0, 255};
Color spellColor = RED;
int number_of_trees = 115;

std::string state = "MENU";

struct Ball {
  Color spellColor = RED;
  Vector2 ball_pos = {};
  Vector2 target_pos = {}; // Renamed from mouse_pos
  float ball_r = 25.0f;
  float ball_speed;
  float damage = 0.0f;
  uint32_t spell_id = 0;
  uint8_t owner_id = 0;
  bool has_hit = false;
};

std::unordered_map<uint32_t, Ball> active_spells; // Track spells by ID

bool checkCollision(Vector2 player_pos, std::vector<Vector2> &trees_pos) {
  Rectangle player_rect = {player_pos.x, player_pos.y, 20, 40};

  for (int i = 0; i < trees_pos.size(); i++) {
    Rectangle tree_rect = {trees_pos[i].x, trees_pos[i].y, 20, 60};

    if (CheckCollisionRecs(player_rect, tree_rect)) {
      return true;
    }
  }
  return false;
}

std::vector<Texture2D> tree_spawner() {
  std::vector<Texture2D> vec(number_of_trees);
  Image tree_img = LoadImage("assets/tree.png");
  for (int i = 0; i < number_of_trees; i++) {
    vec[i] = LoadTextureFromImage(tree_img);
  }
  UnloadImage(tree_img);
  return vec;
}

std::vector<Vector2> tree_positions() {
  std::vector<Vector2> vec(number_of_trees);
  int lower_bound = 000;
  int upper_bound = 2000;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(lower_bound, upper_bound);

  for (int i = 0; i < number_of_trees; i++) {
    float random_x = distr(gen);
    float random_y = distr(gen);
    vec[i] = {random_x, random_y};
  }
  return vec;
}

std::string getDirectionToMouse(Vector2 player_pos, Vector2 mouse_world) {
  float angle =
      atan2(mouse_world.y - player_pos.y, mouse_world.x - player_pos.x);

  float deg = angle * RAD2DEG;

  if (deg < 0)
    deg += 360;

  if (deg >= 337.5 || deg < 22.5)
    return "east";
  if (deg < 67.5)
    return "southeast";
  if (deg < 112.5)
    return "south";
  if (deg < 157.5)
    return "southwest";
  if (deg < 202.5)
    return "west";
  if (deg < 247.5)
    return "northwest";
  if (deg < 292.5)
    return "north";
  return "northeast";
}

bool SetupHost() {
  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = SERVER_PORT;
  // making the server host
  serverHost = enet_host_create(&address,
                                1, // max clients
                                2, // max channels
                                0, // incomding bandiwth ( 0 = uinlmited)
                                0);
  if (serverHost == NULL) {
    printf("failed to create server host"); // i was watching a codeforces
                                            // tournament and realized u can use
                                            // c functions in cpp
    return false;
  }
  printf("Server h osting on port %d\n", SERVER_PORT);
  return true;
}

bool SetupClient() {
  // create a client host
  clientHost = enet_host_create(NULL, 1, 2, 0, 0);
  if (clientHost == NULL) {
    printf("failed to create client host\n");
    return false;
  }

  ENetAddress address;
  enet_address_set_host(&address, SERVER_IP);
  address.port = SERVER_PORT;

  serverPeer = enet_host_connect(clientHost, &address, 2, 0);
  if (serverPeer == NULL) {
    printf("failed to connect to server");
    return false;
  }
  printf("Connecting to %s:%d...\n", SERVER_IP, SERVER_PORT);
  return true;
}

int main() {

  // enet initilization
  if (enet_initialize() != 0) {
    fprintf(stderr, "An error occurred while initializing ENet.\n");
    return EXIT_FAILURE;
  } else {
    std::cout << "enet initialized" << std::endl;
  }
  atexit(enet_deinitialize);

  const int screenWidth = 1280;
  const int screenHeight = 720;

  InitWindow(screenWidth, screenHeight, "WIZARD DUEL");
  InitAudioDevice();
  Image island = LoadImage("assets/island.png");
  Texture2D island_img = LoadTextureFromImage(island);
  Image cursor = LoadImage("assets/cursor.png");
  Texture2D cursor_img = LoadTextureFromImage(cursor);
  Image player_self = LoadImage("assets/self_char.png");
  Texture2D self_char = LoadTextureFromImage(player_self);

  std::vector<Ball> ball_vec;
  int balls_size = 0;

  UnloadImage(island);
  UnloadImage(cursor);
  UnloadImage(player_self);

  Character player;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> x_distr(100, 1280);
  std::uniform_int_distribution<> y_distr(100, 720);

  std::vector<Texture2D> trees = tree_spawner();
  std::vector<Vector2> tree_pos = tree_positions();
  std::vector<Character> all_players; // 1v1 for now
  all_players.push_back(player);      // local player always first
  all_players[0].is_local = true;

  // Push another player for testing
  Character opponent;
  opponent.pos.x = x_distr(gen);
  opponent.pos.y = y_distr(gen);
  opponent.is_local = false;
  all_players.push_back(opponent);

  // safe spawn for local player
  do {
    all_players[0].pos.x = x_distr(gen);
    all_players[0].pos.y = y_distr(gen);
  } while (checkCollision(all_players[0].pos, tree_pos));

  Rectangle tree_rect = {0, 0, 20, 60};

  SetTargetFPS(60);
  Camera2D camera = {0};
  camera.target.x = all_players[0].pos.x + all_players[0].rect.width / 2;
  camera.target.y = all_players[0].pos.y + all_players[0].rect.height / 2;
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.zoom = 7.0f;

  int ball_x = all_players[0].pos.x;
  int ball_y = all_players[0].pos.y;
  float ball_r = 25.0f;
  Vector2 ball_pos = {(float)ball_x, (float)ball_y};
  Sound deathSound = LoadSound("assets/music/death.mp3");

  while (!WindowShouldClose()) {
    Vector2 mouse_world = GetScreenToWorld2D(GetMousePosition(), camera);

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && all_players[0].mana > 12.0f) {
      all_players[0].mana = all_players[0].mana - 10.0f;
      all_players[0].health = all_players[0].health + 2.0f;
    }

    Vector2 old_pos = all_players[0].pos;
    if (all_players[0].is_local) {
      if (IsKeyDown(KEY_D))
        all_players[0].pos.x += 0.5f;
      else if (IsKeyDown(KEY_A))
        all_players[0].pos.x -= 0.5f;
      else if (IsKeyDown(KEY_S))
        all_players[0].pos.y += 0.5f;
      else if (IsKeyDown(KEY_W))
        all_players[0].pos.y -= 0.5f;
      if (checkCollision(all_players[0].pos, tree_pos)) {
        all_players[0].pos = old_pos;
      }
    }

    // Update camera target to follow local player
    camera.target =
        (Vector2){all_players[0].pos.x + 20, all_players[0].pos.y + 20};

    // // Handle spell casting with proper world coordinates
    // if (IsKeyDown(KEY_ONE)){
    //     if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
    //         if (all_players[0].mana >= 25.0f){
    //             Ball red_ball;
    //             all_players[0].is_cast = true;
    //             red_ball.target_pos = mouse_world;  // Use world coordinates
    //             all_players[0].mana = all_players[0].mana - 5.0f;
    //             red_ball.spellColor = bloodRed;
    //             red_ball.ball_speed = 2.0f;
    //             red_ball.ball_pos = all_players[0].pos;
    //             red_ball.ball_r = 35.0f;
    //             red_ball.damage = 1.0f * red_ball.ball_r;
    //             ball_vec.push_back(red_ball);
    //             balls_size = ball_vec.size();
    //         }
    //     }
    // } else if (IsKeyDown(KEY_TWO)){
    //     if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
    //         if (all_players[0].mana >= 35.0f){
    //             Ball blue_ball;
    //             all_players[0].is_cast = true;
    //             blue_ball.target_pos = mouse_world;  // Use world coordinates
    //             all_players[0].mana = all_players[0].mana - 5.0f;
    //             blue_ball.spellColor = DARKBLUE;
    //             blue_ball.ball_pos = all_players[0].pos;
    //             blue_ball.ball_speed = 4.0f;
    //             blue_ball.damage = 1.0f * blue_ball.ball_r;
    //             ball_vec.push_back(blue_ball);
    //             balls_size = ball_vec.size();
    //         }
    //     }
    // }
    if (IsKeyDown(KEY_ONE)) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (all_players[0].mana >= 25.0f) {
          Ball new_ball;
          new_ball.spell_id = next_spell_id++;
          new_ball.owner_id = all_players[0].id;
          new_ball.target_pos = mouse_world;
          new_ball.ball_pos = all_players[0].pos;
          new_ball.spellColor = bloodRed;
          new_ball.ball_speed = 2.0f;
          new_ball.ball_r = 35.0f;
          new_ball.damage = 35.0f;
          new_ball.has_hit = false;
          ball_vec.push_back(new_ball);
          active_spells[new_ball.spell_id] = new_ball;
          all_players[0].mana -= 5.0f;
          all_players[0].is_cast = true;

          if (state == "MULTIPLAYER") {
            SpellCastPacket spellPacket;
            spellPacket.type = MSG_SPELL_CAST;
            spellPacket.playerId = all_players[0].id;
            spellPacket.target_x = mouse_world.x;
            spellPacket.target_y = mouse_world.y;
            spellPacket.spell_type = 1; // the red one
            spellPacket.pos_x = all_players[0].pos.x;
            spellPacket.pos_y = all_players[0].pos.y;
            spellPacket.spell_id = new_ball.spell_id;

            ENetPacket *packet =
                enet_packet_create(&spellPacket, sizeof(SpellCastPacket),
                                   ENET_PACKET_FLAG_RELIABLE);
            if (clientHost != nullptr && serverPeer != nullptr) {
              enet_peer_send(serverPeer, 0, packet);
            } else if (serverHost != nullptr && clientPeer != nullptr) {
              enet_peer_send(clientPeer, 0, packet);
            }
          }
        }
      }
    } else if (IsKeyDown(KEY_TWO)) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (all_players[0].mana >= 35.0f) {
          Ball new_ball;
          new_ball.spell_id = next_spell_id++;
          new_ball.owner_id = all_players[0].id;
          new_ball.target_pos = mouse_world;
          new_ball.ball_pos = all_players[0].pos;
          new_ball.spellColor = DARKBLUE;
          new_ball.ball_speed = 4.0f;
          new_ball.ball_r = 25.0f;
          new_ball.damage = 25.0f;
          new_ball.has_hit = false;

          ball_vec.push_back(new_ball);
          active_spells[new_ball.spell_id] = new_ball;

          all_players[0].mana -= 5.0f;
          all_players[0].is_cast = true;

          if (state == "MULTIPLAYER") {
            SpellCastPacket spellPacket;
            spellPacket.type = MSG_SPELL_CAST;
            spellPacket.playerId = all_players[0].id;
            spellPacket.target_x = mouse_world.x;
            spellPacket.target_y = mouse_world.y;
            spellPacket.spell_type = 2; // blue
            spellPacket.pos_x = all_players[0].pos.x;
            spellPacket.pos_y = all_players[0].pos.y;
            spellPacket.spell_id = new_ball.spell_id;

            ENetPacket *packet =
                enet_packet_create(&spellPacket, sizeof(SpellCastPacket),
                                   ENET_PACKET_FLAG_RELIABLE);

            if (clientHost != nullptr && serverPeer != nullptr) {
              enet_peer_send(serverPeer, 0, packet);
            } else if (serverHost != nullptr && clientPeer != nullptr) {
              enet_peer_send(clientPeer, 0, packet);
            }
          }
        }
      }
    }

    // Handle tree collision for balls
    for (int b = 0; b < ball_vec.size(); b++) {
      for (int t = 0; t < tree_pos.size(); t++) {
        Rectangle tree_rect = {tree_pos[t].x, tree_pos[t].y, 20, 60};

        if (CheckCollisionCircleRec(ball_vec[b].ball_pos, ball_vec[b].ball_r,
                                    tree_rect)) {
          if (ball_vec[b].ball_r >= 4.0f) {
            tree_pos[t] = {0, 0};
          }
          ball_vec[b].ball_r -= 5.0f;
          break;
        }
      }
    }

    // Camera zoom controls
    if (IsKeyDown(KEY_Z)) {
      camera.zoom += 0.02f;
    } else if (IsKeyDown(KEY_X)) {
      camera.zoom -= 0.02f;
    }

    if (camera.zoom > 10.0f)
      camera.zoom = 10.0f;
    else if (camera.zoom < 1.5f)
      camera.zoom = 1.5f;

    if (IsKeyPressed(KEY_R)) {
      camera.zoom = 9.9f;
    }

    // Mana management
    if (camera.zoom < 5.0f) {
      all_players[0].mana = all_players[0].mana - (camera.zoom * 0.03f);
      all_players[0].draining_mana = true;
    } else if (all_players[0].mana < 300 && camera.zoom > 5.0f) {
      all_players[0].mana = all_players[0].mana + 0.01f;
      all_players[0].draining_mana = false;
    }

    BeginDrawing();
    if (state == "MENU") {
      ShowCursor();
      ClearBackground(BLACK);

      DrawText("WIZARD DUEL", screenWidth / 2 - 180, 80, 50, WHITE);

      // Adjust button positions to accommodate more buttons
      Rectangle singleBtn = {screenWidth / 2 - 150, 180, 300, 60};
      Rectangle hostBtn = {screenWidth / 2 - 150, 260, 300, 60};
      Rectangle joinBtn = {screenWidth / 2 - 150, 340, 300, 60};
      Rectangle quitBtn = {screenWidth / 2 - 150, 420, 300, 60};

      Vector2 mouse = GetMousePosition();

      bool singleHover = CheckCollisionPointRec(mouse, singleBtn);
      bool hostHover = CheckCollisionPointRec(mouse, hostBtn);
      bool joinHover = CheckCollisionPointRec(mouse, joinBtn);
      bool quitHover = CheckCollisionPointRec(mouse, quitBtn);

      // Draw buttons
      DrawRectangleRec(singleBtn, singleHover ? MAROON : RED);
      DrawRectangleRec(hostBtn, hostHover ? DARKGREEN : GREEN);
      DrawRectangleRec(joinBtn, joinHover ? DARKBLUE : BLUE);
      DrawRectangleRec(quitBtn, quitHover ? DARKGRAY : GRAY);

      // Button text
      DrawText("SINGLEPLAYER", singleBtn.x + 55, singleBtn.y + 18, 25, WHITE);

      DrawText("HOST GAME", hostBtn.x + 70, hostBtn.y + 18, 25, WHITE);

      DrawText("JOIN GAME", joinBtn.x + 75, joinBtn.y + 18, 25, WHITE);

      DrawText("QUIT", quitBtn.x + 115, quitBtn.y + 18, 25, WHITE);

      // Button interactions
      if (singleHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        state = "SINGLE";
      }

      if (hostHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        state = "HOST";
      }

      if (joinHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        state = "JOIN";
      }

      if (quitHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        break;
      }
      EndDrawing();
    } else if (state == "SINGLE") {
      HideCursor();
      ClearBackground(BLUE);
      BeginMode2D(camera);

      // Draw world
      DrawTexture(island_img, 0, 0, LIGHTGRAY);
      for (int i = 0; i < number_of_trees; i++) {
        DrawTextureRec(trees[i], tree_rect, tree_pos[i], WHITE);
      }

      // Draw all players according to number of players in array
      for (int i = 0; i < all_players.size(); i++) {
        // Draw player with appropriate color/tint
        Color playerTint = all_players[i].is_local ? WHITE : GRAY;
        DrawTextureRec(self_char, all_players[i].rect, all_players[i].pos,
                       playerTint);

        // Draw health and mana bars for each player
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 20, 40, 5, DARKGRAY);
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 20,
                      (all_players[i].health / 300.0f) * 40, 5, RED);
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 13, 40, 5, DARKGRAY);
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 13,
                      (all_players[i].mana / 300.0f) * 40, 5, PURPLE);
      }

      // Draw cursor in world space
      DrawTexture(cursor_img, mouse_world.x - 10, mouse_world.y - 10, WHITE);

      // Check boundaries for local player
      if (all_players[0].pos.x < 0 || all_players[0].pos.x > 2000 ||
          all_players[0].pos.y < 0 || all_players[0].pos.y > 2000) {
        all_players[0].health = 0.0f;
      }

      // Update and draw balls
      if (all_players[0].is_cast && !ball_vec.empty()) {
        for (int i = 0; i < ball_vec.size(); i++) {
          Ball &current_ball = ball_vec[i];

          DrawCircle(current_ball.ball_pos.x, current_ball.ball_pos.y,
                     current_ball.ball_r, current_ball.spellColor);

          Vector2 direction = {
              current_ball.target_pos.x - current_ball.ball_pos.x,
              current_ball.target_pos.y - current_ball.ball_pos.y};

          float length =
              sqrt(direction.x * direction.x + direction.y * direction.y);

          current_ball.ball_r -= 0.1f;

          if (length > 1.0f) {
            direction.x /= length;
            direction.y /= length;

            current_ball.ball_pos.x += direction.x * current_ball.ball_speed;
            current_ball.ball_pos.y += direction.y * current_ball.ball_speed;
          }
        }
      }

      if (ball_vec.empty()) {
        all_players[0].is_cast = false;
      }

      // Death animation for all players
      for (int i = 0; i < all_players.size(); i++) {
        if (all_players[i].isDead && all_players[i].deathTime < 1.0f) {
          all_players[i].deathTime += GetFrameTime() * 0.5f;
          if (all_players[i].deathTime > 1.0f)
            all_players[i].deathTime = 1.0f;
        }

        if (all_players[i].health <= 0.0f) {
          all_players[i].isDead = true;
        }
      }

      EndMode2D();

      // UI text (screen space) - using local player for reference
      float distance = sqrt(pow(mouse_world.y - all_players[0].pos.y, 2) +
                            pow(mouse_world.x - all_players[0].pos.x, 2));
      std::string dist_bw =
          "Target Distance: " + std::to_string((int)distance) +
          "  Direction: " +
          getDirectionToMouse(all_players[0].pos, mouse_world) +
          "  Players: " + std::to_string(all_players.size());
      DrawText(dist_bw.c_str(), 30, 520, 20, BLACK);

      if (all_players[0].draining_mana) {
        DrawText("DRAINING MANA", 30, 550, 20, BLACK);
      }
      if (all_players[0].health > 300.0f) {
        DrawText("SHIELDS UP", 30, 580, 20, BLACK);
        all_players[0].health_timer += GetFrameTime();
        if (all_players[0].health_timer >= 2.0) {
          all_players[0].health = all_players[0].health - 3.0f;
          all_players[0].health_timer = 0.0f;
        }
      }

      // Death screen (only for local player)
      if (all_players[0].isDead) {
        DrawRectangle(0, 0, screenWidth, screenHeight,
                      Fade(BLACK, all_players[0].deathTime));
        if (all_players[0].deathTime >= 0.6f) {
          DrawText("YOU DIED", screenWidth / 2 - 100, screenHeight / 2 - 50, 50,
                   Fade(RED, all_players[0].deathTime));
        }
      }

      // Death sound and reset (only for local player)
      if (all_players[0].isDead && !all_players[0].deathSoundPlayed) {
        PlaySound(deathSound);
        all_players[0].deathSoundPlayed = true;
      }

      // Check if opponent died
      if (all_players.size() > 1 && all_players[1].isDead &&
          !all_players[1].deathSoundPlayed) {
        // Opponent died, show victory
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(GREEN, 0.3f));
        DrawText("VICTORY!", screenWidth / 2 - 120, screenHeight / 2 - 50, 50,
                 GOLD);
      }

      bool shouldReset =
          (all_players[0].isDead && all_players[0].deathTime >= 1.0f) ||
          (all_players.size() > 1 && all_players[1].isDead &&
           all_players[1].deathTime >= 1.0f);

      if (shouldReset) {
        state = "MENU";
        // reset local player
        all_players[0].health = 300.0f;
        all_players[0].mana = 300.0f;
        all_players[0].isDead = false;
        all_players[0].deathSoundPlayed = false;
        all_players[0].deathTime = 0.0f;
        ball_vec.clear();

        // Reset opponent for testing
        if (all_players.size() > 1) {
          all_players[1].health = 300.0f;
          all_players[1].mana = 300.0f;
          all_players[1].isDead = false;
          all_players[1].deathSoundPlayed = false;
          all_players[1].deathTime = 0.0f;
        }
      }

      // Debug key
      if (IsKeyDown(KEY_I)) {
        all_players[0].health = all_players[0].health - 50.0f;
      }
      EndDrawing();
    } else if (state == "HOST") {
      BeginDrawing();
      ClearBackground(BLACK);

      if (connectionState == "DISCONNECTED") {
        DrawText("Starting server...", screenWidth / 2 - 120, screenHeight / 2,
                 30, WHITE);

        if (SetupHost()) {
          connectionState = "HOSTING";
        } else {
          state = "MENU";
          connectionState = "DISCONNECTED";
        }
      } else if (connectionState == "HOSTING") {
        DrawText("HOSTING GAME...", screenWidth / 2 - 150, screenHeight / 2, 30,
                 WHITE);
        DrawText("Waiting for player to join", screenWidth / 2 - 180,
                 screenHeight / 2 + 50, 20, GRAY);

        // Check for new connections
        ENetEvent event;
        while (enet_host_service(serverHost, &event, 0) > 0) {
          if (event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("Client connected!\n");
            clientPeer = event.peer;
            connectionState = "CONNECTED";

            // Keep our position
            Vector2 current_pos = all_players[0].pos;

            // Start the game
            all_players.clear();
            player.id = 0;
            player.pos = current_pos;
            all_players.push_back(player);
            all_players[0].is_local = true;

            // Add opponent (will be updated by network)
            Character opponent;
            opponent.id = 1;
            opponent.pos.x = 0;
            opponent.pos.y = 0;
            opponent.is_local = false;
            all_players.push_back(opponent);

            // sending tree positions so both clients have same map
            for (size_t i = 0; i < tree_pos.size(); i++) {
              struct TreePacket {
                float x;
                float y;
              };

              TreePacket treePkt = {tree_pos[i].x, tree_pos[i].y};
              ENetPacket *packet = enet_packet_create(
                  &treePkt, sizeof(TreePacket), ENET_PACKET_FLAG_RELIABLE);

              enet_peer_send(clientPeer, 0, packet);
            }

            // sending intitial position of host
            PositionPacket posPacket;
            posPacket.type = MSG_POSITION_UPDATE;
            posPacket.playerId = 0;
            posPacket.x = all_players[0].pos.x;
            posPacket.y = all_players[0].pos.y;

            ENetPacket *posPkt = enet_packet_create(
                &posPacket, sizeof(PositionPacket), ENET_PACKET_FLAG_RELIABLE);

            enet_peer_send(clientPeer, 0, posPkt);

            state = "MULTIPLAYER";
          }
        }
      }
      EndDrawing();
    } else if (state == "JOIN") {
      BeginDrawing();
      ClearBackground(BLACK);

      if (connectionState == "DISCONNECTED") {
        DrawText("Connecting...", screenWidth / 2 - 100, screenHeight / 2, 30,
                 WHITE);

        if (SetupClient()) {
          connectionState = "JOINING";
        } else {
          state = "MENU";
          connectionState = "DISCONNECTED";
        }
      } else if (connectionState == "JOINING") {
        DrawText("JOINING GAME...", screenWidth / 2 - 150, screenHeight / 2, 30,
                 WHITE);
        DrawText("Connecting to host...", screenWidth / 2 - 140,
                 screenHeight / 2 + 50, 20, GRAY);

        // Check connection status
        ENetEvent event;
        while (enet_host_service(clientHost, &event, 0) > 0) {
          if (event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("Connected to server!\n");
            connectionState = "CONNECTED";

            // Keep our position
            Vector2 current_pos = all_players[0].pos;

            // Start the game
            all_players.clear();
            player.id = 1; // client is [player 1]
            player.pos = current_pos;
            all_players.push_back(player);
            all_players[0].is_local = true;

            // Add opponent (the host)
            Character opponent;
            opponent.id = 0;
            opponent.pos.x = 0;
            opponent.pos.y = 0;
            opponent.is_local = false;
            all_players.push_back(opponent);

            // Send our position to host
            PositionPacket posPacket;
            posPacket.type = MSG_POSITION_UPDATE;
            posPacket.playerId = 1;
            posPacket.x = all_players[0].pos.x;
            posPacket.y = all_players[0].pos.y;

            ENetPacket *posPkt = enet_packet_create(
                &posPacket, sizeof(PositionPacket), ENET_PACKET_FLAG_RELIABLE);

            enet_peer_send(serverPeer, 0, posPkt);

            state = "MULTIPLAYER";
          }
        }
      }
      EndDrawing();
    } else if (state == "MULTIPLAYER") {
      // receive updates from other player
      ENetEvent event;
      ENetHost *activeHost = clientHost != nullptr ? clientHost : serverHost;
      if (activeHost != nullptr) {
        while (enet_host_service(activeHost, &event, 0) > 0) {
          if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            NetworkPacket *header = (NetworkPacket *)event.packet->data;

            switch (header->type) {
            case MSG_POSITION_UPDATE: {
              PositionPacket *posPacket = (PositionPacket *)event.packet->data;

              for (auto &p : all_players) {
                if (p.id == header->playerId) {
                  p.pos.x = posPacket->x;
                  p.pos.y = posPacket->y;
                  break;
                }
              }
              break;
            }
            case MSG_SPELL_CAST: {
              SpellCastPacket *spellPacket =
                  (SpellCastPacket *)event.packet->data;

              // locally spawning so dont respawn our own spell ( balls )
              if (spellPacket->playerId == all_players[0].id) {
                break;
              }

              Ball new_ball;
              new_ball.spell_id = spellPacket->spell_id;
              new_ball.owner_id = spellPacket->playerId;
              // i am tired boss
              new_ball.target_pos = {spellPacket->target_x,
                                     spellPacket->target_y};
              new_ball.ball_pos = {spellPacket->pos_x, spellPacket->pos_y};

              if (spellPacket->spell_type == 1) {
                new_ball.spellColor = bloodRed;
                new_ball.ball_speed = 2.0f;
                new_ball.damage = 35.0f;
                new_ball.ball_r = 35.0f;
              } else {
                new_ball.spellColor = DARKBLUE;
                new_ball.ball_speed = 4.0f;
                new_ball.damage = 25.0f;
                new_ball.ball_r = 25.0f;
              }

              // new_ball.ball_r = 25.0f;
              new_ball.has_hit = false;

              ball_vec.push_back(new_ball);
              active_spells[new_ball.spell_id] = new_ball;
              break;
            }
            case MSG_HIT_CONFIRM: {
              HitPacket *hitPacket = (HitPacket *)event.packet->data;

              for (auto &p : all_players) {
                if (p.id == hitPacket->target_player) {
                  p.health -= hitPacket->damage;
                  break;
                }
              }
              auto it = active_spells.find(hitPacket->spell_id);
              if (it != active_spells.end()) {
                it->second.has_hit = true;
              }
              break;
            }

            case MSG_HEALTH_UPDATE: {
              HealthPacket *healthPacket = (HealthPacket *)event.packet->data;
              for (auto &p : all_players) {
                if (p.id == header->playerId) {
                  p.health = healthPacket->health;
                  p.mana = healthPacket->mana;
                  break;
                }
              }
              break;
            }

            case MSG_PLAYER_DEATH: {
              for (auto &p : all_players) {
                if (p.id == header->playerId) {
                  p.health = 0;
                  p.isDead = true;
                  break;
                }
              }
              break;
            }
            case MSG_TREE_POSITIONS: {
              TreePacket *treePkt = (TreePacket *)event.packet->data;
              tree_pos[treePkt->index] = {treePkt->x, treePkt->y};
              trees_received++;

              // idk what to do here but if all trees go we should start ig
              if (trees_received >= number_of_trees) {
                // idk what to put here
              }
              break;
            }
            }
            enet_packet_destroy(event.packet);
          }
        }
      }
      HideCursor();
      ClearBackground(BLUE);
      BeginMode2D(camera);

      // Draw world
      DrawTexture(island_img, 0, 0, WHITE);
      for (int i = 0; i < number_of_trees; i++) {
        DrawTextureRec(trees[i], tree_rect, tree_pos[i], WHITE);
      }

      // Draw all players according to number of players in array
      for (int i = 0; i < all_players.size(); i++) {
        // Draw player with appropriate color/tint
        Color playerTint = all_players[i].is_local ? WHITE : GRAY;
        DrawTextureRec(self_char, all_players[i].rect, all_players[i].pos,
                       playerTint);

        // Draw health and mana bars for each player
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 20, 40, 5, DARKGRAY);
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 20,
                      (all_players[i].health / 300.0f) * 40, 5, RED);
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 13, 40, 5, DARKGRAY);
        DrawRectangle((int)all_players[i].pos.x - 10,
                      (int)all_players[i].pos.y - 13,
                      (all_players[i].mana / 300.0f) * 40, 5, PURPLE);
      }

      // player IDs for debugging
      for (int i = 0; i < all_players.size(); i++) {
        DrawText(TextFormat("P%d", all_players[i].id), all_players[i].pos.x,
                 all_players[i].pos.y - 30, 15, YELLOW);
      }

      // Draw cursor in world space
      DrawTexture(cursor_img, mouse_world.x - 10, mouse_world.y - 10, WHITE);

      // Check boundaries for local player
      if (all_players[0].pos.x < 0 || all_players[0].pos.x > 2000 ||
          all_players[0].pos.y < 0 || all_players[0].pos.y > 2000) {
        all_players[0].health = 0.0f;
      }

      // Update and draw balls
      if (all_players[0].is_cast && !ball_vec.empty()) {
        for (int i = 0; i < ball_vec.size(); i++) {
          Ball &current_ball = ball_vec[i];

          DrawCircle(current_ball.ball_pos.x, current_ball.ball_pos.y,
                     current_ball.ball_r, current_ball.spellColor);

          Vector2 direction = {
              current_ball.target_pos.x - current_ball.ball_pos.x,
              current_ball.target_pos.y - current_ball.ball_pos.y};

          float length =
              sqrt(direction.x * direction.x + direction.y * direction.y);

          current_ball.ball_r -= 0.1f;

          if (length > 1.0f) {
            direction.x /= length;
            direction.y /= length;

            current_ball.ball_pos.x += direction.x * current_ball.ball_speed;
            current_ball.ball_pos.y += direction.y * current_ball.ball_speed;
          }
        }
      }

      // Hit detection for balls
      for (int i = ball_vec.size() - 1; i >= 0; i--) {
        Ball &ball = ball_vec[i];

        // Skip if ball already hit something
        if (ball.has_hit)
          continue;

        for (auto &player : all_players) {
          if (player.id == ball.owner_id)
            continue;
          if (player.isDead)
            continue;

          Rectangle playerRect = {player.pos.x, player.pos.y, 20, 40};

          if (CheckCollisionCircleRec(ball.ball_pos, ball.ball_r, playerRect)) {
            // Hit detected!
            ball.has_hit = true;

            player.health -= ball.damage;

            if (state == "MULTIPLAYER") {
              HitPacket hitPacket;
              hitPacket.type = MSG_HIT_CONFIRM;
              hitPacket.playerId = all_players[0].id; // sender
              hitPacket.damage = ball.damage;
              hitPacket.spell_id = ball.spell_id;
              hitPacket.target_player = player.id;

              ENetPacket *packet = enet_packet_create(
                  &hitPacket, sizeof(HitPacket), ENET_PACKET_FLAG_RELIABLE);

              if (clientHost != nullptr && serverPeer != nullptr) {
                enet_peer_send(serverPeer, 0, packet);
              } else if (serverHost != nullptr && clientPeer != nullptr) {
                enet_peer_send(clientPeer, 0, packet);
              }
            }

            ball.ball_r -= 10.0f;
            if (ball.ball_r <= 5.0f) {
              ball_vec.erase(ball_vec.begin() + i);
            }

            break;
          }
        }
      }

      if (ball_vec.empty()) {
        all_players[0].is_cast = false;
      }

      // Death animation for all players
      for (int i = 0; i < all_players.size(); i++) {
        if (all_players[i].isDead && all_players[i].deathTime < 1.0f) {
          all_players[i].deathTime += GetFrameTime() * 0.5f;
          if (all_players[i].deathTime > 1.0f)
            all_players[i].deathTime = 1.0f;
        }

        if (all_players[i].health <= 0.0f && !all_players[i].isDead) {
          all_players[i].isDead = true;

          // Notify the other player of the death
          if (i == 0 && state == "MULTIPLAYER") {
            NetworkPacket deathPacket;
            deathPacket.type = MSG_PLAYER_DEATH;
            deathPacket.playerId = all_players[0].id;

            ENetPacket *packet = enet_packet_create(
                &deathPacket, sizeof(NetworkPacket), ENET_PACKET_FLAG_RELIABLE);

            if (clientHost != nullptr && serverPeer != nullptr) {
              enet_peer_send(serverPeer, 0, packet);
            } else if (serverHost != nullptr && clientPeer != nullptr) {
              enet_peer_send(clientPeer, 0, packet);
            }
          }
        }
      }

      EndMode2D();

      // UI text (screen space) - using local player for reference
      float distance = sqrt(pow(mouse_world.y - all_players[0].pos.y, 2) +
                            pow(mouse_world.x - all_players[0].pos.x, 2));
      std::string dist_bw =
          "Target Distance: " + std::to_string((int)distance) +
          "  Direction: " +
          getDirectionToMouse(all_players[0].pos, mouse_world) +
          "  Players: " + std::to_string(all_players.size());
      DrawText(dist_bw.c_str(), 30, 520, 20, BLACK);

      if (all_players[0].draining_mana) {
        DrawText("DRAINING MANA", 30, 550, 20, BLACK);
      }
      if (all_players[0].health > 300.0f) {
        DrawText("SHIELDS UP", 30, 580, 20, BLACK);
        all_players[0].health_timer += GetFrameTime();
        if (all_players[0].health_timer >= 2.0) {
          all_players[0].health = all_players[0].health - 3.0f;
          all_players[0].health_timer = 0.0f;
        }
      }

      // Death screen (only for local player)
      if (all_players[0].isDead) {
        DrawRectangle(0, 0, screenWidth, screenHeight,
                      Fade(BLACK, all_players[0].deathTime));
        if (all_players[0].deathTime >= 0.6f) {
          DrawText("YOU DIED", screenWidth / 2 - 100, screenHeight / 2 - 50, 50,
                   Fade(RED, all_players[0].deathTime));
        }
      }

      // Death sound and reset (only for local player)
      if (all_players[0].isDead && !all_players[0].deathSoundPlayed) {
        PlaySound(deathSound);
        all_players[0].deathSoundPlayed = true;
      }

      // Check if opponent died
      if (all_players.size() > 1 && all_players[1].isDead &&
          !all_players[1].deathSoundPlayed) {
        // Opponent died, show victory
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(GREEN, 0.3f));
        DrawText("VICTORY!", screenWidth / 2 - 120, screenHeight / 2 - 50, 50,
                 GOLD);
      }

      DrawText(TextFormat("Ping: %d ms",
                          clientPeer
                              ? clientPeer->roundTripTime
                              : (serverPeer ? serverPeer->roundTripTime : 0)),
               30, 600, 20, BLACK);

      bool shouldReset =
          (all_players[0].isDead && all_players[0].deathTime >= 1.0f) ||
          (all_players.size() > 1 && all_players[1].isDead &&
           all_players[1].deathTime >= 1.0f);

      if (shouldReset) {
        state = "MENU";
        // reset local player
        all_players[0].health = 300.0f;
        all_players[0].mana = 300.0f;
        all_players[0].isDead = false;
        all_players[0].deathSoundPlayed = false;
        all_players[0].deathTime = 0.0f;
        ball_vec.clear();

        // Reset opponent for testing
        if (all_players.size() > 1) {
          all_players[1].health = 300.0f;
          all_players[1].mana = 300.0f;
          all_players[1].isDead = false;
          all_players[1].deathSoundPlayed = false;
          all_players[1].deathTime = 0.0f;
        }

        // Clean up connections for next match
        if (serverHost != nullptr) {
          enet_host_destroy(serverHost);
          serverHost = nullptr;
        }
        if (clientHost != nullptr) {
          enet_host_destroy(clientHost);
          clientHost = nullptr;
        }
        serverPeer = nullptr;
        clientPeer = nullptr;
        connectionState = "DISCONNECTED";
      }

      // Debug key
      if (IsKeyDown(KEY_I)) {
        all_players[0].health = all_players[0].health - 50.0f;
      }
      EndDrawing();

      Vector2 old_pos = all_players[0].pos;
      if (all_players[0].is_local) {
        if (IsKeyDown(KEY_D))
          all_players[0].pos.x += 0.5f;
        else if (IsKeyDown(KEY_A))
          all_players[0].pos.x -= 0.5f;
        else if (IsKeyDown(KEY_S))
          all_players[0].pos.y += 0.5f;
        else if (IsKeyDown(KEY_W))
          all_players[0].pos.y -= 0.5f;
        if (checkCollision(all_players[0].pos, tree_pos)) {
          all_players[0].pos = old_pos;
        }
      }

      if (all_players[0].pos.x != old_pos.x ||
          all_players[0].pos.y != old_pos.y) {
        PositionPacket posPacket;
        posPacket.type = MSG_POSITION_UPDATE;
        posPacket.playerId = all_players[0].id;
        posPacket.x = all_players[0].pos.x;
        posPacket.y = all_players[0].pos.y;

        ENetPacket *packet = enet_packet_create(
            &posPacket, sizeof(PositionPacket), ENET_PACKET_FLAG_RELIABLE);

        if (clientHost != nullptr && serverPeer != nullptr) {
          enet_peer_send(serverPeer, 0, packet);
        } else if (serverHost != nullptr && clientPeer != nullptr) {
          enet_peer_send(clientPeer, 0, packet);
        }
      }

      // Periodic health/mana sync (every 30 frames)
      static int health_sync_counter = 0;
      health_sync_counter++;
      if (health_sync_counter >= 30) {
        health_sync_counter = 0;

        HealthPacket healthPacket;
        healthPacket.type = MSG_HEALTH_UPDATE;
        healthPacket.playerId = all_players[0].id;
        healthPacket.health = all_players[0].health;
        healthPacket.mana = all_players[0].mana;

        ENetPacket *packet = enet_packet_create(
            &healthPacket, sizeof(HealthPacket), ENET_PACKET_FLAG_RELIABLE);

        if (clientHost != nullptr && serverPeer != nullptr) {
          enet_peer_send(serverPeer, 0, packet);
        } else if (serverHost != nullptr && clientPeer != nullptr) {
          enet_peer_send(clientPeer, 0, packet);
        }
      }
    }
  }
  // Network cleanup
  if (serverHost != nullptr) {
    enet_host_destroy(serverHost);
  }
  if (clientHost != nullptr) {
    enet_host_destroy(clientHost);
  }
  // Cleanup
  for (auto &t : trees) {
    UnloadTexture(t);
  }
  UnloadTexture(island_img);
  UnloadTexture(cursor_img);
  UnloadTexture(self_char);
  UnloadSound(deathSound);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}