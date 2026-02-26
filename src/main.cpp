#include "raylib.h"
#include <bits/stdc++.h>

struct Character {
    float health = 300.0f;
    float mana = 300.0f;
    double health_timer = 0.0;
    bool draining_mana = false;
    bool is_cast = false;
    float deathTime = 0.0f;
    bool isDead = false;
    bool deathSoundPlayed = false;
    Vector2 pos = {};
    Rectangle rect = { 0, 0, 20, 40 };
};

Color bloodRed = { 128, 0, 0, 255 };
Color spellColor = RED;
int number_of_trees = 115;

std::string state = "MENU";

struct Ball {
    Color spellColor = RED;
    Vector2 ball_pos = {};
    Vector2 direction = { 0, 0 };  // normalized direction vector at cast time
    float ball_r = 25.0f;
    float ball_speed = 2.0f;
    float damage = 0.0f;
};

bool checkCollision(Vector2 player_pos, std::vector<Vector2>& trees_pos) {
    Rectangle player_rect = { player_pos.x, player_pos.y, 20, 40 };
    for (int i = 0; i < (int)trees_pos.size(); i++) {
        Rectangle tree_rect = { trees_pos[i].x, trees_pos[i].y, 20, 60 };
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
    int lower_bound = 0;
    int upper_bound = 2000;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(lower_bound, upper_bound);

    for (int i = 0; i < number_of_trees; i++) {
        float random_x = (float)distr(gen);
        float random_y = (float)distr(gen);
        vec[i] = { random_x, random_y };
    }
    return vec;
}

std::string getDirectionToMouse(Vector2 player_pos) {
    Vector2 mouse = GetMousePosition();
    float angle = atan2(mouse.y - player_pos.y, mouse.x - player_pos.x);
    float deg = angle * RAD2DEG;
    if (deg < 0) deg += 360;

    if (deg >= 337.5 || deg < 22.5)  return "east";
    if (deg < 67.5)   return "southeast";
    if (deg < 112.5)  return "south";
    if (deg < 157.5)  return "southwest";
    if (deg < 202.5)  return "west";
    if (deg < 247.5)  return "northwest";
    if (deg < 292.5)  return "north";
    return "northeast";
}

// Helper: compute normalized direction from player to world-space mouse
Vector2 calcDirection(Vector2 from, Vector2 to) {
    Vector2 dir = { to.x - from.x, to.y - from.y };
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.0f) { dir.x /= len; dir.y /= len; }
    return dir;
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "WIZARD DUEL");
    InitAudioDevice();

    Image island    = LoadImage("assets/island.png");
    Texture2D island_img = LoadTextureFromImage(island);
    Image cursor    = LoadImage("assets/cursor.png");
    Texture2D cursor_img = LoadTextureFromImage(cursor);
    Image player_self = LoadImage("assets/self_char.png");
    Texture2D self_char = LoadTextureFromImage(player_self);

    UnloadImage(island);
    UnloadImage(cursor);
    UnloadImage(player_self);

    std::vector<Ball> ball_vec;

    Character player;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> x_distr(100, 640);
    std::uniform_int_distribution<> y_distr(100, 720);

    std::vector<Texture2D> trees = tree_spawner();
    std::vector<Vector2> tree_pos = tree_positions();

    // safe spawn
    do {
        player.pos.x = (float)x_distr(gen);
        player.pos.y = (float)y_distr(gen);
    } while (checkCollision(player.pos, tree_pos));

    Rectangle tree_rect = { 0, 0, 20, 60 };

    SetTargetFPS(60);
    Camera2D camera = { 0 };
    camera.target.x = player.pos.x + player.rect.width / 2;
    camera.target.y = player.pos.y + player.rect.height / 2;
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 7.0f;

    Sound deathSound = LoadSound("assets/music/death.mp3");

    while (!WindowShouldClose()) {

        // --- UPDATE ---

        if (state == "SINGLE") {

            // right click: convert mana to health
            if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && player.mana > 12.0f) {
                player.mana   -= 10.0f;
                player.health += 2.0f;
            }

            // movement
            Vector2 old_pos = player.pos;
            if      (IsKeyDown(KEY_D)) player.pos.x += 0.5f;
            else if (IsKeyDown(KEY_A)) player.pos.x -= 0.5f;
            else if (IsKeyDown(KEY_S)) player.pos.y += 0.5f;
            else if (IsKeyDown(KEY_W)) player.pos.y -= 0.5f;
            if (checkCollision(player.pos, tree_pos)) player.pos = old_pos;

            // ball vs tree collision
            for (int b = 0; b < (int)ball_vec.size(); b++) {
                for (int t = 0; t < (int)tree_pos.size(); t++) {
                    Rectangle tr = { tree_pos[t].x, tree_pos[t].y, 20, 60 };
                    if (CheckCollisionCircleRec(ball_vec[b].ball_pos, ball_vec[b].ball_r, tr)) {
                        if (ball_vec[b].ball_r >= 4.0f) tree_pos[t] = { 0, 0 };
                        ball_vec[b].ball_r -= 5.0f;
                        break;
                    }
                }
            }

            // spell casting — KEY_ONE: slow heavy red ball
            if (IsKeyDown(KEY_ONE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player.mana >= 25.0f) {
                Ball red_ball;
                player.is_cast = true;
                player.mana -= 25.0f;  // was only costing 5, fixed to match mana check

                // convert screen mouse to world coords so direction is correct at any zoom
                Vector2 mouse_world = GetScreenToWorld2D(GetMousePosition(), camera);
                red_ball.direction  = calcDirection(player.pos, mouse_world);

                red_ball.spellColor = bloodRed;
                red_ball.ball_speed = 2.0f;
                red_ball.ball_pos   = player.pos;
                red_ball.ball_r     = 35.0f;
                red_ball.damage     = 1.0f * red_ball.ball_r;
                ball_vec.push_back(red_ball);
            }

            // KEY_TWO: fast small blue ball
            if (IsKeyDown(KEY_TWO) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player.mana >= 35.0f) {
                Ball blue_ball;
                player.is_cast = true;
                player.mana -= 35.0f;  // fixed to match mana check

                Vector2 mouse_world  = GetScreenToWorld2D(GetMousePosition(), camera);
                blue_ball.direction  = calcDirection(player.pos, mouse_world);

                blue_ball.spellColor = DARKBLUE;
                blue_ball.ball_pos   = player.pos;
                blue_ball.ball_speed = 4.0f;
                blue_ball.ball_r     = 25.0f;
                blue_ball.damage     = 1.0f * blue_ball.ball_r;
                ball_vec.push_back(blue_ball);
            }

            // move all active balls along their fixed direction
            for (int i = 0; i < (int)ball_vec.size(); i++) {
                Ball& b = ball_vec[i];
                b.ball_pos.x += b.direction.x * b.ball_speed;
                b.ball_pos.y += b.direction.y * b.ball_speed;
                b.ball_r -= 0.1f;
            }

            // remove dead balls
            ball_vec.erase(
                std::remove_if(ball_vec.begin(), ball_vec.end(),
                    [](const Ball& b){ return b.ball_r <= 0.0f; }),
                ball_vec.end()
            );

            if (ball_vec.empty()) player.is_cast = false;

            // camera
            camera.target = (Vector2){ player.pos.x + 20, player.pos.y + 20 };

            if (IsKeyDown(KEY_Z)) camera.zoom += 0.1f;
            else if (IsKeyDown(KEY_X)) camera.zoom -= 0.1f;
            if (camera.zoom > 10.0f) camera.zoom = 10.0f;
            else if (camera.zoom < 1.5f) camera.zoom = 1.5f;
            if (IsKeyPressed(KEY_R)) camera.zoom = 9.9f;

            // mana drain/regen based on zoom
            if (camera.zoom < 5.0f) {
                player.mana -= camera.zoom * 0.03f;
                player.draining_mana = true;
            } else if (player.mana < 300.0f && camera.zoom >= 5.0f) {
                player.mana += 0.01f;
                player.draining_mana = false;
            }

            // out of bounds kills player
            if (player.pos.x < 0 || player.pos.x > 2000 || player.pos.y < 0 || player.pos.y > 2000) {
                player.health = 0.0f;
            }

            // health cap / shield drain
            if (player.health > 300.0f) {
                player.health_timer += GetFrameTime();
                if (player.health_timer >= 2.0) {
                    player.health -= 3.0f;
                    player.health_timer = 0.0;
                }
            }

            // death check
            if (player.health <= 0.0f) player.isDead = true;

            if (player.isDead) {
                if (!player.deathSoundPlayed) {
                    PlaySound(deathSound);
                    player.deathSoundPlayed = true;
                }
                player.deathTime += GetFrameTime() * 0.5f;
                if (player.deathTime > 1.0f) player.deathTime = 1.0f;
            }

            // cheat key
            if (IsKeyDown(KEY_I)) player.health -= 50.0f;
        }

        // --- DRAW ---
        BeginDrawing();

        if (state == "MENU") {
            ShowCursor();
            ClearBackground(BLACK);
            DrawText("WIZARD DUEL", screenWidth / 2 - 180, 120, 50, WHITE);

            Rectangle singleBtn = { screenWidth / 2 - 150.0f, 260, 300, 60 };
            Rectangle quitBtn   = { screenWidth / 2 - 150.0f, 350, 300, 60 };
            Vector2 mouse = GetMousePosition();

            bool singleHover = CheckCollisionPointRec(mouse, singleBtn);
            bool quitHover   = CheckCollisionPointRec(mouse, quitBtn);

            DrawRectangleRec(singleBtn, singleHover ? MAROON : RED);
            DrawRectangleRec(quitBtn,   quitHover   ? DARKGRAY : GRAY);
            DrawText("SINGLEPLAYER", (int)singleBtn.x + 55, (int)singleBtn.y + 18, 25, WHITE);
            DrawText("QUIT",         (int)quitBtn.x  + 115, (int)quitBtn.y  + 18, 25, WHITE);

            if (singleHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) state = "SINGLE";
            if (quitHover   && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) break;

        } else if (state == "SINGLE") {
            HideCursor();
            ClearBackground(BLUE);
            BeginMode2D(camera);

            DrawTexture(island_img, 0, 0, WHITE);

            for (int i = 0; i < number_of_trees; i++) {
                DrawTextureRec(trees[i], tree_rect, tree_pos[i], WHITE);
            }

            DrawTextureRec(self_char, player.rect, player.pos, WHITE);

            // health bar
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 20, 40, 5, DARKGRAY);
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 20,
                          (int)((player.health / 300.0f) * 40), 5, RED);
            // mana bar
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 13, 40, 5, DARKGRAY);
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 13,
                          (int)((player.mana / 300.0f) * 40), 5, PURPLE);

            // draw all balls
            for (int i = 0; i < (int)ball_vec.size(); i++) {
                Ball& cur = ball_vec[i];
                DrawCircle((int)cur.ball_pos.x, (int)cur.ball_pos.y, cur.ball_r, cur.spellColor);
            }

            // custom cursor (drawn in world space inside camera mode)
            DrawTexture(cursor_img, GetMouseX(), GetMouseY(), WHITE);

            EndMode2D();

            // HUD
            int distance = (int)sqrtf(
                (float)((GetMouseY() - player.pos.y) * (GetMouseY() - player.pos.y)) +
                (float)((GetMouseX() - player.pos.x) * (GetMouseX() - player.pos.x)));
            std::string dist_bw = " Target Distance: " + std::to_string(distance) +
                                  "  Direction: " + getDirectionToMouse(player.pos);
            DrawText(dist_bw.c_str(), 30, 520, 20, BLACK);

            // mana drain warning
            if (player.draining_mana) {
                DrawText("draining mana", 30, 545, 20, PURPLE);
            }

            // shield indicator
            if (player.health > 300.0f) {
                DrawText("shields up", 30, 570, 20, SKYBLUE);
            }

            // death overlay
            if (player.isDead) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, player.deathTime));
                if (player.deathTime >= 0.6f) {
                    DrawText("YOU DIED",
                             screenWidth / 2 - 100, screenHeight / 2 - 50,
                             50, Fade(RED, player.deathTime));
                }
            }

            if (player.isDead && player.deathTime >= 1.0f) {
                state = "MENU";
                player.health          = 300.0f;
                player.mana            = 300.0f;
                player.isDead          = false;
                player.deathSoundPlayed = false;
                player.deathTime       = 0.0f;
                player.draining_mana   = false;
                ball_vec.clear();
            }
        }

        EndDrawing();
    }

    for (auto& t : trees) UnloadTexture(t);
    UnloadTexture(island_img);
    UnloadTexture(cursor_img);
    UnloadTexture(self_char);
    UnloadSound(deathSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}