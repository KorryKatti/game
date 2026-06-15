// UIHelper.h
#ifndef UI_HELPER_H
#define UI_HELPER_H

#include "raylib.h"
#include <string>
#include <vector>
#include <deque>
#include "GameClient.h"

class UIHelper {
    private:
        static char apiKeyInput[256];
        static bool showPlayerList; // idk about bool
        static int selectedPlayerIndex;
        static float heartbeatTimer; // tbhi would never do this stuff , its good that college had onbject orienterd progtamming or i would never use this. i hate ts
        static char chatInputBuffer[256];
        static std::deque<std::pair<std::string, std::string>> cachedMessages;
        static float lastPollTime;
        static bool isChatOpen;
        static std::string currentChatPartner;
        static bool connectionStatus;
        static float connectionCheckTimer;

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
            Rectangle submitBtn = {GetScreenWidth()/2-100,350,200,50};
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
        static std::string getAPIKeyInput(){
            return std::string(apiKeyInput);
        }

        static void resetAPIKeyInput(){
            apiKeyInput[0]='\0';
        }

        static void updateHeartbeat(float deltaTime,GameClient& client){
            heartbeatTimer+=deltaTime;
            if (heartbeatTimer>=30.0f){
                client.heartbeat();
                heartbeatTimer=0.0f;
            }
        }
        static void resetHeartbeatTimer(){
            heartbeatTimer = 0.0f;
        }

        static void drawPlayerListWithStatus(const std::vector<OnlinePlayer>& players,
                                              std::string& selected_player_for_chat,
                                              bool& open_chat,
                                              bool& ping_trigger) {
            ClearBackground(DARKBLUE);

            std::string statusText = connectionStatus ? "CONNECTED" : "DISCONNECTED";
            Color statusColor = connectionStatus ? GREEN : RED;
            DrawText(statusText.c_str(), GetScreenWidth() - 180, 20, 15, statusColor);

            DrawText("ONLINE PLAYERS", GetScreenWidth()/2 - 150, 50, 40, WHITE);
            DrawText("Ping to alert - Chat to coordinate", GetScreenWidth()/2 - 160, 100, 20, GRAY);

            int y = 160;
            for (size_t i = 0; i < players.size() && i < 8; i++) {
                Rectangle playerCard = {GetScreenWidth()/2 - 300, (float)y, 600, 70};
                bool hover = CheckCollisionPointRec(GetMousePosition(), playerCard);
                DrawRectangleRec(playerCard, hover ? ColorAlpha(PURPLE, 0.3f) : ColorAlpha(DARKPURPLE, 0.2f));
                DrawRectangleLinesEx(playerCard, 1, GOLD);

                std::string display = players[i].username + " (ELO: " + std::to_string(players[i].elo) + ")";
                DrawText(display.c_str(), playerCard.x + 20, playerCard.y + 15, 20, WHITE);

                DrawCircle(playerCard.x + 15, playerCard.y + 45, 5, GREEN);

                Rectangle pingBtn = {playerCard.x + 350, playerCard.y + 15, 80, 40};
                bool pingHover = CheckCollisionPointRec(GetMousePosition(), pingBtn);
                DrawRectangleRec(pingBtn, pingHover ? ORANGE : (Color){180, 100, 0, 255});
                DrawText("PING", pingBtn.x + 18, pingBtn.y + 10, 20, WHITE);

                Rectangle chatBtn = {playerCard.x + 450, playerCard.y + 15, 80, 40};
                bool chatHover = CheckCollisionPointRec(GetMousePosition(), chatBtn);
                DrawRectangleRec(chatBtn, chatHover ? GREEN : DARKGREEN);
                DrawText("CHAT", chatBtn.x + 15, chatBtn.y + 10, 20, WHITE);

                if (pingHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selected_player_for_chat = players[i].username;
                    ping_trigger = true;
                }

                if (chatHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selected_player_for_chat = players[i].username;
                    open_chat = true;
                }

                y += 90;
            }

            if (players.empty()) {
                DrawText("No players online", GetScreenWidth()/2 - 100, 300, 25, GRAY);
                DrawText("Host a game to appear here", GetScreenWidth()/2 - 120, 340, 20, GRAY);
            }

            Rectangle backBtn = {50, GetScreenHeight() - 80, 150, 50};
            bool backHover = CheckCollisionPointRec(GetMousePosition(), backBtn);
            DrawRectangleRec(backBtn, backHover ? RED : MAROON);
            DrawText("BACK", backBtn.x + 45, backBtn.y + 12, 25, WHITE);
        }

        static void drawChatWindow(const std::string& with_player,
                                   bool& close_chat,
                                   GameClient& client) {
            ClearBackground(DARKBLUE);

            DrawText(("Chat with " + with_player).c_str(), 50, 30, 30, WHITE);
            DrawLine(50, 70, GetScreenWidth() - 50, 70, GRAY);

            std::string statusText = connectionStatus ? "Connected to server" : "Disconnected";
            Color statusColor = connectionStatus ? GREEN : RED;
            DrawText(statusText.c_str(), GetScreenWidth() - 200, 35, 15, statusColor);

            float now = GetTime();
            if (now - lastPollTime >= 2.0f && connectionStatus) {
                auto newMessages = client.getMessages(with_player, 30);
                if (!newMessages.empty()) {
                    cachedMessages.clear();
                    for (auto& msg : newMessages) {
                        cachedMessages.push_back(msg);
                    }
                }
                lastPollTime = now;
            }

            int y = 100;
            int maxMessages = 15;
            int startIndex = (cachedMessages.size() > maxMessages) ? cachedMessages.size() - maxMessages : 0;

            for (size_t i = startIndex; i < cachedMessages.size(); i++) {
                std::string sender = (cachedMessages[i].first == client.getUsername()) ? "You" : cachedMessages[i].first;
                std::string displayMsg = sender + ": " + cachedMessages[i].second;

                bool isOwn = (cachedMessages[i].first == client.getUsername());
                Color bgColor = isOwn ? ColorAlpha(BLUE, 0.3f) : ColorAlpha(DARKGRAY, 0.3f);

                Rectangle msgBox = {80, (float)y, GetScreenWidth() - 160, 35};
                DrawRectangleRec(msgBox, bgColor);
                DrawText(displayMsg.c_str(), msgBox.x + 10, msgBox.y + 8, 18, WHITE);

                y += 45;
                if (y > GetScreenHeight() - 150) break;
            }

            Rectangle inputBox = {80, GetScreenHeight() - 80, GetScreenWidth() - 250, 45};
            DrawRectangleRec(inputBox, LIGHTGRAY);
            DrawRectangleLinesEx(inputBox, 2, DARKGRAY);
            DrawText(chatInputBuffer, inputBox.x + 10, inputBox.y + 12, 18, BLACK);

            Rectangle sendBtn = {GetScreenWidth() - 150, GetScreenHeight() - 80, 100, 45};
            bool sendHover = CheckCollisionPointRec(GetMousePosition(), sendBtn);
            DrawRectangleRec(sendBtn, sendHover ? GREEN : DARKGREEN);
            DrawText("SEND", sendBtn.x + 25, sendBtn.y + 12, 20, WHITE);

            if (IsKeyPressed(KEY_BACKSPACE) && strlen(chatInputBuffer) > 0) {
                chatInputBuffer[strlen(chatInputBuffer) - 1] = '\0';
            }

            int key = GetCharPressed();
            while (key > 0 && strlen(chatInputBuffer) < 255) {
                if (isalnum(key) || isspace(key) || ispunct(key)) {
                    chatInputBuffer[strlen(chatInputBuffer)] = (char)key;
                    chatInputBuffer[strlen(chatInputBuffer) + 1] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_ENTER) && strlen(chatInputBuffer) > 0) {
                if (client.sendMessage(with_player, std::string(chatInputBuffer))) {
                    cachedMessages.push_back({client.getUsername(), std::string(chatInputBuffer)});
                    chatInputBuffer[0] = '\0';
                }
            }

            if (sendHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && strlen(chatInputBuffer) > 0) {
                if (client.sendMessage(with_player, std::string(chatInputBuffer))) {
                    cachedMessages.push_back({client.getUsername(), std::string(chatInputBuffer)});
                    chatInputBuffer[0] = '\0';
                }
            }

            Rectangle backBtn = {30, GetScreenHeight() - 80, 80, 45};
            bool backHover = CheckCollisionPointRec(GetMousePosition(), backBtn);
            DrawRectangleRec(backBtn, backHover ? RED : MAROON);
            DrawText("BACK", backBtn.x + 15, backBtn.y + 12, 20, WHITE);

            if (backHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                close_chat = true;
            }
        }

        static void updateConnectionStatus(GameClient& client) {
            connectionCheckTimer += GetFrameTime();
            if (connectionCheckTimer >= 5.0f) {
                connectionStatus = client.checkConnection();
                connectionCheckTimer = 0.0f;
            }
        }

        static void resetChatState() {
            chatInputBuffer[0] = '\0';
            cachedMessages.clear();
            lastPollTime = 0.0f;
            isChatOpen = false;
            currentChatPartner = "";
        }
};

// static members
char UIHelper::apiKeyInput[256]={0};
bool UIHelper::showPlayerList=false;
int UIHelper::selectedPlayerIndex=-1;
float UIHelper::heartbeatTimer=0.0f;
char UIHelper::chatInputBuffer[256] = {0};
std::deque<std::pair<std::string, std::string>> UIHelper::cachedMessages;
float UIHelper::lastPollTime = 0.0f;
bool UIHelper::isChatOpen = false;
std::string UIHelper::currentChatPartner = "";
bool UIHelper::connectionStatus = false;
float UIHelper::connectionCheckTimer = 0.0f;

#endif