#include "raylib.h"
#include <bits/stdc++.h>

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib - basic window");
    SetTargetFPS(60);
    std::string text = "";
    while (!WindowShouldClose()) {
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
            ClearBackground(RAYWHITE);
            DrawText(text.c_str(),190,200,20,LIGHTGRAY);
            DrawText("press any key",190,230,20,RED);
            DrawText(mousePos.c_str(),190,250,20,DARKGRAY);
            EndDrawing();
        }

    CloseWindow();
    return 0;
}