// UIHelper.h
#ifndef UI_HELPER_H
#define UI_HELPER_H

#include "raylib.h"
#include <string>
#include <vector>
#include "GameClient.h"

class UIHelper {
    private:
        static char apiKeyInput[256];
        static bool showPlayerList; // idk about bool
        static int selectedPlayerIndex;
        static float heartbeatTimer; // tbhi would never do this stuff , its good that college had onbject orienterd progtamming or i would never use this. i hate ts

    public:
        static void drawAPIKeyPrompt(){
            ClearBackground(DARKBLUE);
            DrawText("Enter your api key :",GetScreenWidth()/2-200,100,40,WHITE);
            DrawText("Get it from blabla.com/profile",GetScreenWidth()/2-180,160,20,GRAY);

            Rectangle inputBox = {GetScreenWidth()/2-200,250,400,50};
            DrawRectangleRec(inputBox,LIGHTGRAY);
            DrawRectangleLinesEx(inputBox,2,DARKGRAY);

            DrawText(apiKeyInput,inputBox.x+10,inputBox.y+15,20,BLACK);

            if (IsKeyPressed(KEY_BACKSPACE)&& strlen(apiKeyInput)>0){
                // i cant believe i am making it learn backspace
                apiKeyInput[strlen(apiKeyInput)-1]='\0';

            }
            int key = GetCharPressed();
            while (key>0&&strlen(apiKeyInput)<255){
                apiKeyInput[strlen(apiKeyInput)] = (char)key;
                apiKeyInput[strlen(apiKeyInput)+1]='\0';
                key = GetCharPressed();
            }
            // submit button
            Rectangle submitBtn = {GetScreenWidth()/2-100,250,200,50};
            bool hover = CheckCollisionPointRec(GetMousePosition(),submitBtn);
            DrawRectangleRec(submitBtn,hover?GREEN:DARKGREEN);
            DrawText("SUBMIT",submitBtn.x+60,submitBtn.y+12,25,WHITE);

            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                // will be handled by main.cpp
            }

        }
        static void drawPlayerList(const std::vector<OnlinePlayer>& players, int& selectedPort, std::string& selectedIP){
            // TODO
        }
}

#endif UI_HELPER_H