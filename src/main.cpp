#include "raylib.h"
#include <bits/stdc++.h>

int health = 300;
int mana = 300;

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
    
 
    SetTargetFPS(60);
    Rectangle player = { 400, 280, 20, 40 };
    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + 10.0f, player.y + 10.0f };
    camera.offset = (Vector2){ player.x + 10.0f, player.y + 10.0f };
    camera.zoom = 7.0f;
    // std::string text = "";
    while (!WindowShouldClose()) {  
               // Player movement
        if (IsKeyDown(KEY_RIGHT)) player.x += 0.5;
        else if (IsKeyDown(KEY_LEFT)) player.x -= 0.5;
        else if (IsKeyDown(KEY_DOWN)) player.y += 0.5;
        else if (IsKeyDown(KEY_UP)) player.y -= 0.5;

        // Camera target follows player
        camera.target = (Vector2){ player.x + 20, player.y + 20 };

        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        // camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
        if (IsKeyDown(KEY_Z)){camera.zoom = expf(logf(camera.zoom + 0.1f));}
        else if (IsKeyDown(KEY_X)){camera.zoom = expf(logf(camera.zoom - 0.1f));}

        if (camera.zoom > 7.0f) camera.zoom = 7.0f;
        else if (camera.zoom < 2.0f) camera.zoom = 2.0f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 7.0f;
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
            DrawTexture(cursor_img,GetMouseX(),GetMouseY(),WHITE);
            EndMode2D();
            int distance = sqrt(((GetMouseY()-player.y)*(GetMouseY()-player.y)) + ((GetMouseX()-player.x)*(GetMouseX()-player.x)));
            std::string dist_bw = " Spell Distance : " + std::to_string(distance);
            DrawText((dist_bw).c_str(),30,520,20,BLACK);
            bars();

            DrawRectangleRec(player, RED);
            EndDrawing();
        }

    CloseWindow();
    return 0;
}