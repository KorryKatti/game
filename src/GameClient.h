// GameClient.h
#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "httplib.h"
#include <string>
#include <vector>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

struct OnlinePlayer {
    std::string username;
    std::string game_ip;
    int game_port;
    int elo;
    std::string avatar_url;
};

class GameClient {
private:
    std::string api_key;
    std::string server_url;
    bool is_online;
    std::string public_ip;
    
    std::string getStoragePath() {
        return "wizard_api_key.txt";
    }
    
    std::string fetchPublicIP() {
        httplib::Client client("https://api.ipify.org");
        auto res = client.Get("/");
        if (res && res->status == 200) {
            return res->body;
        }
        return "0.0.0.0";
    }
    
public:
    GameClient(const std::string& url = "http://localhost:3000") 
        : server_url(url), is_online(false), public_ip("0.0.0.0") {
        loadAPIKey();
    }
    
    void loadAPIKey() {
        std::ifstream file(getStoragePath());
        if (file.is_open()) {
            std::getline(file, api_key);
            file.close();
        }
    }
    
    void saveAPIKey(const std::string& key) {
        api_key = key;
        std::ofstream file(getStoragePath());
        if (file.is_open()) {
            file << key;
            file.close();
        }
    }
    
    bool hasAPIKey() {
        return !api_key.empty();
    }
    
    std::string getAPIKey() {
        return api_key;
    }
    
    bool validateAndLogin(const std::string& key) {
        httplib::Client client(server_url.c_str());
        client.set_bearer_token_auth(key);
        
        auto res = client.Get("/v1/profile");
        
        if (res && res->status == 200) {
            saveAPIKey(key);
            return true;
        }
        return false;
    }
    
    bool goOnline(int game_port) {
        if (!hasAPIKey()) return false;
        
        // Get public IP once
        if (public_ip == "0.0.0.0") {
            public_ip = fetchPublicIP();
        }
        
        httplib::Client client(server_url.c_str());
        client.set_bearer_token_auth(api_key);
        
        json body = {
            {"game_ip", public_ip},
            {"game_port", game_port},
            {"status", "online"}
        };
        
        auto res = client.Post("/v1/players/heartbeat", 
                                body.dump(), 
                                "application/json");
        
        if (res && res->status == 200) {
            is_online = true;
            return true;
        }
        return false;
    }
    
    void goOffline() {
        if (!hasAPIKey() || !is_online) return;
        
        httplib::Client client(server_url.c_str());
        client.set_bearer_token_auth(api_key);
        
        json body = {
            {"status", "offline"}
        };
        
        client.Post("/v1/players/heartbeat", body.dump(), "application/json");
        is_online = false;
    }
    
    std::vector<OnlinePlayer> getOnlinePlayers() {
        std::vector<OnlinePlayer> players;
        
        if (!hasAPIKey()) return players;
        
        httplib::Client client(server_url.c_str());
        client.set_bearer_token_auth(api_key);
        
        auto res = client.Get("/v1/players/online");
        
        if (res && res->status == 200) {
            try {
                json data = json::parse(res->body);
                for (auto& p : data["players"]) {
                    OnlinePlayer player;
                    player.username = p["username"];
                    player.game_ip = p["game_ip"];
                    player.game_port = p["game_port"];
                    player.elo = p.value("elo", 1200);
                    player.avatar_url = p.value("avatar_url", "");
                    players.push_back(player);
                }
            } catch (...) {
                printf("Failed to parse player list\n");
            }
        }
        
        return players;
    }
    
    void heartbeat() {
        if (!hasAPIKey() || !is_online) return;
        
        httplib::Client client(server_url.c_str());
        client.set_bearer_token_auth(api_key);
        
        json body = {
            {"status", "online"}
        };
        
        client.Post("/v1/players/heartbeat", body.dump(), "application/json");
    }
    
    bool uploadMatch(const std::string& filename, 
                     const std::string& opponent_name,
                     const std::string& winner,
                     int duration_seconds,
                     const std::string& match_id) {
        
        if (!hasAPIKey()) return false;
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string file_content = buffer.str();
        file.close();
        
        std::string boundary = "----WizardDuelBoundary";
        std::string body;
        
        auto add_field = [&](const std::string& name, const std::string& value) {
            body += "--" + boundary + "\r\n";
            body += "Content-Disposition: form-data; name=\"" + name + "\"\r\n\r\n";
            body += value + "\r\n";
        };
        
        add_field("opponent_name", opponent_name);
        add_field("winner", winner);
        add_field("duration", std::to_string(duration_seconds));
        add_field("match_id", match_id);
        
        body += "--" + boundary + "\r\n";
        body += "Content-Disposition: form-data; name=\"match_file\"; filename=\"" + filename + "\"\r\n";
        body += "Content-Type: text/plain\r\n\r\n";
        body += file_content + "\r\n";
        body += "--" + boundary + "--\r\n";
        
        httplib::Client client(server_url.c_str());
        client.set_bearer_token_auth(api_key);
        
        httplib::Headers headers = {
            {"Content-Type", "multipart/form-data; boundary=" + boundary}
        };
        
        auto res = client.Post("/v1/matches", headers, body, "multipart/form-data; boundary=" + boundary);
        
        return res && res->status == 200;
    }
    
    ~GameClient() {
        if (is_online) {
            goOffline();
        }
    }
};

#endif