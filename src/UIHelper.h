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
            ClearBackground(DARKBLUE);
            DrawText("ONLINE PLAYERS",GetScreenWidth()/2-150,50,40,WHITE);
            DrawText("Click a player to challenge them",GetScreenWidth()/2-160,100,20,GRAY);

            int y=160;
            for (size_t i=0;i<players.size()&&i<10;i++){
                Rectangle playerBtn = {GetScreenWidth()/2-200,(float)y,400,60};
                bool hover = CheckCollisionPointRec(GetMousePosition(),playerBtn);
                Color btnColor = hover? PURPLE:DARKPURPLE;
                DrawRectangleRec(playerBtn,btnColor);
                DrawRectangleLinesEx(playerBtn,2,GOLD);

                // username and elo
                std::string display = players[i].username + " (ELO: " + std::to_string(players[i].elo)+ ")";
                DrawText(display.c_str(),playerBtn.x+20,playerBtn.y+40,15,LIGHTGRAY);

                if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    selectedIP = players[i].game_ip;
                    selectedPort = players[i].game_port;
                }
                y+=80;
            }
            if (players.empty()){
                DrawText("No players online",GetScreenWidth()/2-100,200,25,GRAY);
                DrawText("Host a game to appear here",GetScreenWidth()/2-120,240,20,GRAY);
            }
            // back button
            Rectangle backBtn = {50,GetScreenHeight()-80,150,50};
            bool backHover = CheckCollisionPointRec(GetMousePosition(),backBtn);

            DrawRectangleRec(backBtn,backHover?RED:MAROON);

            DrawText("BACK",backBtn.x+45,backBtn.y+12,25,WHITE);
        }

        // homelander happy very large sunlight emoji
}

#endif UI_HELPER_H