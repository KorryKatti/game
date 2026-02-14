#include "raylib.h"
#include <bits/stdc++.h>

int health = 300;
int mana = 300;

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
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();

    float dx = mouseX - player_pos.x;
    float dy = mouseY - player_pos.y;

    // Small deadzone so it doesn't flicker when very close
    if (fabs(dx) < 5 && fabs(dy) < 5)
        return "center";

    if (dx > 0 && dy < 0)  return "northeast";
    if (dx > 0 && dy > 0)  return "southeast";
    if (dx < 0 && dy < 0)  return "northwest";
    if (dx < 0 && dy > 0)  return "southwest";

    if (dx > 0) return "east";
    if (dx < 0) return "west";
    if (dy < 0) return "north";
    if (dy > 0) return "south";

    return "center";
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
               // Player movement
        if (IsKeyDown(KEY_D)) player_pos.x += 0.5;
        else if (IsKeyDown(KEY_A)) player_pos.x -= 0.5;
        else if (IsKeyDown(KEY_S)) player_pos.y += 0.5;
        else if (IsKeyDown(KEY_W)) player_pos.y -= 0.5;

        // Camera target follows player
        camera.target = (Vector2){ player_pos.x + 20, player_pos.y + 20 };

        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        // camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
        if (IsKeyDown(KEY_Z)){camera.zoom = expf(logf(camera.zoom + 0.1f));}
        else if (IsKeyDown(KEY_X)){camera.zoom = expf(logf(camera.zoom - 0.1f));}

        if (camera.zoom > 5.0f) camera.zoom = 5.0f;
        else if (camera.zoom < 1.5f) camera.zoom = 1.5f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 10.0f;
        }
        // int key_pressed = GetCharPressed();
        // if (key_pressed>0){
        //     text = "You pressed : ";
        //     text += char(key_pressed);
        //     key_pressed = GetCharPressed();
        // }

        // int mouseX = GetMouseX();
        // int mouseY = GetMouseY();
        // std::string mousePos = "";

        // if (mouseX && mouseY) {
        //     mousePos = "mouse is at : " + std::to_string(mouseX) + " , " + std::to_string(mouseY);
        //     mouseX = GetMouseX();
        //     mouseY = GetMouseY();
        // }

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

           
            
            EndDrawing();
        }

    for (auto& t : trees) {
    UnloadTexture(t);
}

    CloseWindow();
    return 0;
}