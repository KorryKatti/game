#include "raylib.h"
#include <bits/stdc++.h>

float health = 300.0f;
float mana = 300.0f;
double health_timer = 0.0;
bool draining_mana =false;

bool checkCollision(Vector2 player_pos,std::vector<Vector2> trees_pos){
    Rectangle player_rect = {player_pos.x,player_pos.y,20,40};
    for (int i=0;i<15;i++){
        Rectangle tree_rect = {trees_pos[i].x,trees_pos[i].y,20,60};
            if (CheckCollisionRecs(player_rect, tree_rect)){
                return true;
            }
    }
    return false;
}

std::vector<Texture2D> tree_spawner(){
    std::vector<Texture2D> vec(15);
    Image tree_img = LoadImage("assets/tree.png");
    for (int i=0;i<15;i++){
        vec[i] = LoadTextureFromImage(tree_img);
    }
    UnloadImage(tree_img);
    return vec;
}

std::vector<Vector2> tree_positions(){
    // will do later , returns random aroud 15 positons to puit trees at
    std::vector<Vector2> vec(15);
    int lower_bound = 000;  // Minimum value (inclusive)
    int upper_bound = 2000; // Maximum value (inclusive)

    std::random_device rd;           // Non-deterministic random seed
    std::mt19937 gen(rd());          // Mersenne Twister generator
    std::uniform_int_distribution<> distr(lower_bound, upper_bound);

    for (int i=0;i<15;i++){
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



void bars(){
    DrawRectangleLines(37, 607 , 300, 10, WHITE);
    DrawText("HEALTH",37,587,20,GRAY);
    DrawRectangle(37,607,health,10,RED);
    DrawRectangleLines(37, 647 , 300, 10, WHITE);
    DrawText("MANA",37,627,20,GRAY);
    DrawRectangle(37,647,mana,10,PURPLE);
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib - basic window");

    Image island = LoadImage("assets/island.png");
    Texture2D island_img = LoadTextureFromImage(island); 
    Image cursor = LoadImage("assets/cursor.png");
    Texture2D cursor_img = LoadTextureFromImage(cursor);
    Image player_self = LoadImage("assets/self_char.png");
    Texture2D self_char = LoadTextureFromImage(player_self);

    UnloadImage(island);
    UnloadImage(cursor);
    UnloadImage(player_self);

    Rectangle player = { 0, 0, 20, 40 };
    Vector2 player_pos = {400,280};
    
    Rectangle tree_rect = {0,0,20,60};
    
 
    SetTargetFPS(60);
    Camera2D camera = { 0 };
    camera.target = (Vector2){ player_pos.x + 10.0f, player_pos.y + 10.0f };
    camera.offset = (Vector2){ player_pos.x + 10.0f, player_pos.y + 10.0f };
    camera.zoom = 7.0f;

    std::vector<Texture2D> trees = tree_spawner();
    std::vector<Vector2> tree_pos = tree_positions();

    // std::string text = "";
    while (!WindowShouldClose()) {  
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && mana>12.0f){
            mana = mana - 10.0f;
            health = health+2.0f;
        }

               // Player movement
        Vector2 old_pos = player_pos;
        if (IsKeyDown(KEY_D)) player_pos.x += 0.5f;
        else if (IsKeyDown(KEY_A)) player_pos.x -= 0.5f;
        else if (IsKeyDown(KEY_S)) player_pos.y += 0.5f;
        else if (IsKeyDown(KEY_W)) player_pos.y -= 0.5f;
        if (checkCollision(player_pos,tree_pos)){
            player_pos = old_pos;
        }

        // Camera target follows player
        camera.target = (Vector2){ player_pos.x + 20, player_pos.y + 20 };

        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        // camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
        if (IsKeyDown(KEY_Z)){camera.zoom = expf(logf(camera.zoom + 0.1f));}
        else if (IsKeyDown(KEY_X)){camera.zoom = expf(logf(camera.zoom - 0.1f));}

        if (camera.zoom > 10.0f) camera.zoom = 10.0f;
        else if (camera.zoom < 1.5f) camera.zoom = 1.5f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 9.9f;
        }
        if (camera.zoom <5.0f){
            mana = mana-(camera.zoom * 0.03f);
            draining_mana= true;
        }else if(mana<300 && camera.zoom > 5.0f){
            mana = mana + 0.001f;
            draining_mana = false;
        }

            BeginDrawing();
            ClearBackground(BLUE);
            BeginMode2D(camera);
            HideCursor();
            DrawTexture(island_img, 0, 0, WHITE);
            for (int i = 0; i < 15; i++){
                DrawTextureRec(trees[i], tree_rect, tree_pos[i], WHITE);
            }
            
            DrawTextureRec(self_char, player, player_pos, WHITE);
            DrawTexture(cursor_img,GetMouseX(),GetMouseY(),WHITE);
            EndMode2D();
            int distance = sqrt(((GetMouseY()-player_pos.y)*(GetMouseY()-player_pos.y)) + ((GetMouseX()-player_pos.x)*(GetMouseX()-player_pos.x)));
            std::string dist_bw = " Target Distance :  " + std::to_string(distance) + " Target Direction : " + getDirectionToMouse(player_pos);
            DrawText((dist_bw).c_str(),30,520,20,BLACK);
            bars();
          
            if (draining_mana){
                    DrawText("draining mana",player_pos.x+20,player_pos.x+20,20,BLACK);
            }if (health>300.0f){
                    DrawText("shields up",player_pos.x-20,player_pos.x-20,20,BLACK);
                    health_timer += GetFrameTime();  // Add time since last frame
                if (health_timer >= 2.0) {
                    // Action to perform after 2 seconds
                    printf("Two seconds have passed!\n");
                    health = health - 3.0f;
                    health_timer = 0.0f;
                }
            }
            EndDrawing();
        }

    for (auto& t : trees) {
    UnloadTexture(t);
}

    CloseWindow();
    return 0;
}