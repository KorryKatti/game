#include "raylib.h"
#include <bits/stdc++.h>

int health = 300;
int mana = 300;

void bars(){
    DrawRectangleLines(37, 607 , 300, 10, WHITE);
    DrawText("HEALTH",37,587,20,LIGHTGRAY);
    DrawRectangle(37,607,health,10,RED);
    DrawRectangleLines(37, 647 , 300, 10, WHITE);
    DrawText("MANA",37,627,20,LIGHTGRAY);
    DrawRectangle(37,647,mana,10,BLUE);
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    Image sample = LoadImage("assets/icon.jpg");
    

    InitWindow(screenWidth, screenHeight, "raylib - basic window");
    SetTargetFPS(60);
    std::string text = "";
    while (!WindowShouldClose()) {
        Texture2D img = LoadTextureFromImage(sample);    
        int key_pressed = GetCharPressed();
        if (key_pressed>0){
            text = "You pressed : ";
            text += char(key_pressed);
            key_pressed = GetCharPressed();
        }

        int mouseX = GetMouseX();
        int mouseY = GetMouseY();
        std::string mousePos = "";

        if (mouseX && mouseY) {
            mousePos = "mouse is at : " + std::to_string(mouseX) + " , " + std::to_string(mouseY);
            mouseX = GetMouseX();
            mouseY = GetMouseY();
        }

            BeginDrawing();
            ClearBackground(BLACK);
            DrawText(text.c_str(),190,200,20,LIGHTGRAY);
            DrawText((std::to_string(health)).c_str(),190,220,20,LIGHTGRAY);
            // DrawText("press any key",190,230,20,RED);
            // DrawText(mousePos.c_str(),190,250,20,DARKGRAY);
            if (IsKeyDown(KEY_A)){
                health--;
            }else if (health<300&&IsKeyDown(KEY_D)&&health!=0){
                health++;
            }
            if (health > 0){
                bars();
            }else{
                ClearBackground(BLACK);
                DrawText("You lose",((screenWidth/2)),(screenHeight)/2,20,LIGHTGRAY);
                DrawTexture(img, 400, 500, BLUE); 
            }
            EndDrawing();
        }

    CloseWindow();
    return 0;
}