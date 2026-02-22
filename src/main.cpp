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

struct Ball{
    Color spellColor = RED;
    Vector2 ball_pos = {};
    Vector2 mouse_pos = {};
    float ball_r = 25.0f;
    float ball_speed;
    float damage = 0.0f;
};

bool checkCollision(Vector2 player_pos, std::vector<Vector2>& trees_pos){
    Rectangle player_rect = {player_pos.x, player_pos.y, 20, 40};

    for (int i = 0; i < trees_pos.size(); i++){
        Rectangle tree_rect = {trees_pos[i].x, trees_pos[i].y, 20, 60};

        if (CheckCollisionRecs(player_rect, tree_rect)){
            return true;
        }
    }
    return false;
}


std::vector<Texture2D> tree_spawner(){
    std::vector<Texture2D> vec(number_of_trees);
    Image tree_img = LoadImage("assets/tree.png");
    for (int i=0;i<number_of_trees;i++){
        vec[i] = LoadTextureFromImage(tree_img);
    }
    UnloadImage(tree_img);
    return vec;
}

std::vector<Vector2> tree_positions(){
    std::vector<Vector2> vec(number_of_trees);
    int lower_bound = 000;
    int upper_bound = 2000;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(lower_bound, upper_bound);

    for (int i=0;i<number_of_trees;i++){
        float random_x = distr(gen);
        float random_y = distr(gen);
        vec[i] = {random_x,random_y};
    }
    return vec;
    
}

std::string getDirectionToMouse(Vector2 player_pos){
    Vector2 mouse = GetMousePosition();
    float angle = atan2(mouse.y - player_pos.y,
                        mouse.x - player_pos.x);

    float deg = angle * RAD2DEG;

    if (deg < 0) deg += 360;

    if (deg >= 337.5 || deg < 22.5)   return "east";
    if (deg < 67.5)   return "southeast";
    if (deg < 112.5)  return "south";
    if (deg < 157.5)  return "southwest";
    if (deg < 202.5)  return "west";
    if (deg < 247.5)  return "northwest";
    if (deg < 292.5)  return "north";
    return "northeast";
}

int main() {
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

    DisableCursor();
    UnloadImage(island);
    UnloadImage(cursor);
    UnloadImage(player_self);

    Character player;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> x_distr(100, 640);
    std::uniform_int_distribution<> y_distr(100, 720);

    player.pos.x = x_distr(gen);
    player.pos.y = y_distr(gen);
    
    Rectangle tree_rect = {0,0,20,60};
    
    SetTargetFPS(60);
    Camera2D camera = { 0 };
    camera.target.x = player.pos.x + player.rect.width / 2;
    camera.target.y = player.pos.y + player.rect.height / 2;
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 7.0f;

    std::vector<Texture2D> trees = tree_spawner();
    std::vector<Vector2> tree_pos = tree_positions();
    int ball_x = player.pos.x;
    int ball_y = player.pos.y;
    float ball_r = 25.0f;
    Vector2 mouse_pos = {0,0};
    Vector2 ball_pos = {(float)ball_x,(float)ball_y};
    Sound deathSound = LoadSound("assets/music/death.mp3");

    while (!WindowShouldClose()) {  
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && player.mana>12.0f){
            player.mana = player.mana - 10.0f;
            player.health = player.health+2.0f;
        }

        Vector2 old_pos = player.pos;
        if (IsKeyDown(KEY_D)) player.pos.x += 0.5f;
        else if (IsKeyDown(KEY_A)) player.pos.x -= 0.5f;
        else if (IsKeyDown(KEY_S)) player.pos.y += 0.5f;
        else if (IsKeyDown(KEY_W)) player.pos.y -= 0.5f;
        if (checkCollision(player.pos,tree_pos)){
            player.pos = old_pos;
        }
        for (int b = 0; b < ball_vec.size(); b++) {
            for (int t = 0; t < tree_pos.size(); t++) {

                Rectangle tree_rect = { tree_pos[t].x, tree_pos[t].y, 20, 60 };

                if (CheckCollisionCircleRec(
                        ball_vec[b].ball_pos,
                        ball_vec[b].ball_r,
                        tree_rect))
                {
                    if (ball_vec[b].ball_r >= 4.0f){tree_pos[t] = {0, 0};}
                    ball_vec[b].ball_r -= 5.0f;
                    break;
                }
            }
        }

        if (IsKeyDown(KEY_ONE)){
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                if (player.mana >= 25.0f){
                Ball red_ball;
                player.is_cast = true;
                red_ball.mouse_pos = GetMousePosition();
                player.mana = player.mana - 5.0f;
                red_ball.spellColor = bloodRed;
                red_ball.ball_speed = 2.0f;
                red_ball.ball_pos = player.pos;
                red_ball.ball_r = 35.0f;
                red_ball.damage = 1.0f*red_ball.ball_r;
                ball_vec.push_back(red_ball);
                balls_size = ball_vec.size();
                }}
        }else if (IsKeyDown(KEY_TWO)){
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                if (player.mana >= 35.0f){
                Ball blue_ball;
                player.is_cast = true;
                blue_ball.mouse_pos = GetMousePosition();
                player.mana = player.mana - 5.0f;
                blue_ball.spellColor = DARKBLUE;
                blue_ball.ball_pos = player.pos;
                blue_ball.ball_speed=4.0f;
                blue_ball.damage = 1.0f * blue_ball.ball_r;
                ball_vec.push_back(blue_ball);
                balls_size = ball_vec.size();
                }}
        }

        camera.target = (Vector2){ player.pos.x + 20, player.pos.y + 20 };

        if (IsKeyDown(KEY_Z)){camera.zoom = expf(logf(camera.zoom + 0.1f));}
        else if (IsKeyDown(KEY_X)){camera.zoom = expf(logf(camera.zoom - 0.1f));}

        if (camera.zoom > 10.0f) camera.zoom = 10.0f;
        else if (camera.zoom < 1.5f) camera.zoom = 1.5f;

        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 9.9f;
        }
        if (camera.zoom <5.0f){
            player.mana = player.mana-(camera.zoom * 0.03f);
            player.draining_mana= true;
        }else if(player.mana<300 && camera.zoom > 5.0f){
            player.mana = player.mana + 0.01f;
            player.draining_mana = false;
        }

            BeginDrawing();
            ClearBackground(BLUE);
            BeginMode2D(camera);
            HideCursor();
            DrawTexture(island_img, 0, 0, WHITE);
            for (int i = 0; i < number_of_trees; i++){
                DrawTextureRec(trees[i], tree_rect, tree_pos[i], WHITE);
            }
            
            DrawTextureRec(self_char, player.rect, player.pos, WHITE);
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 20, 40, 5, DARKGRAY);
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 20, (player.health / 300.0f) * 40, 5, RED);
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 13, 40, 5, DARKGRAY);
            DrawRectangle((int)player.pos.x - 10, (int)player.pos.y - 13, (player.mana / 300.0f) * 40, 5, PURPLE);
            DrawTexture(cursor_img,GetMouseX(),GetMouseY(),WHITE);
            if (player.pos.x < 0 || player.pos.x > 2000 || player.pos.y < 0 || player.pos.y > 2000) {
                player.health = 0.0f;
            }

            if (player.is_cast && ball_r > 0.0f && balls_size!=0){
                if (!ball_vec.empty()){
                    for (int i = 0; i < ball_vec.size(); i++)
                    {
                        Ball& current_ball = ball_vec[i];

                        DrawCircle(current_ball.ball_pos.x,
                                current_ball.ball_pos.y,
                                current_ball.ball_r,
                                current_ball.spellColor);

                        Vector2 direction = {
                            current_ball.mouse_pos.x - current_ball.ball_pos.x,
                            current_ball.mouse_pos.y - current_ball.ball_pos.y
                        };

                        float length = sqrt(direction.x * direction.x +
                                            direction.y * direction.y);

                        current_ball.ball_r -= 0.1f;

                        if (length > 1.0f)
                        {
                            direction.x /= length;
                            direction.y /= length;

                            current_ball.ball_pos.x += direction.x * current_ball.ball_speed;
                            current_ball.ball_pos.y += direction.y * current_ball.ball_speed;
                        }
                    }
                }
            }if (ball_r <= 0.0f){
                player.is_cast = false;
                ball_r = 25.0f;
                ball_x = player.pos.x;
                ball_y = player.pos.y;
                for (int i=0;i<ball_vec.size();i++){
                    ball_vec.clear();
                }
            }

            if (player.isDead && player.deathTime <1.0f){
                player.deathTime += GetFrameTime() * 0.5f;
                if (player.deathTime > 1.0f) player.deathTime = 1.0f;
            }

            if (player.health<=0.0f){
                player.isDead = true;
            }

            EndMode2D();
            int distance = sqrt(((GetMouseY()-player.pos.y)*(GetMouseY()-player.pos.y)) + ((GetMouseX()-player.pos.x)*(GetMouseX()-player.pos.x)));
            std::string dist_bw = " Target Distance :  " + std::to_string(distance) + " Target Direction : " + getDirectionToMouse(player.pos);
            DrawText((dist_bw).c_str(),30,520,20,BLACK);
          
            if (player.draining_mana){
                    DrawText("draining mana",player.pos.x+20,player.pos.x+20,20,BLACK);
            }if (player.health>300.0f){
                    DrawText("shields up",player.pos.x-20,player.pos.x-20,20,BLACK);
                    player.health_timer += GetFrameTime();
                if (player.health_timer >= 2.0) {
                    printf("Two seconds have passed!\n");
                    player.health = player.health - 3.0f;
                    player.health_timer = 0.0f;
                }
            }

            if (player.isDead){
                DrawRectangle(0,0,screenWidth,screenHeight,Fade(BLACK, player.deathTime));
                if (player.deathTime >= 0.6f){
                    DrawText("YOU DIED",screenWidth/2-100,screenHeight/2-50,50,Fade(RED, player.deathTime));
                }
            }

            if (player.isDead && !player.deathSoundPlayed){
                PlaySound(deathSound);
                player.deathSoundPlayed = true;
            }

            if (IsKeyDown(KEY_I)){
                player.health = player.health - 50.0f;
            }

            EndDrawing();
        }

    for (auto& t : trees) {
    UnloadTexture(t);
}

    CloseWindow();
    return 0;
}