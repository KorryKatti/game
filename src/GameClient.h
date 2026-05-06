#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "httplib.h"
#include <string>
#include <vector>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

struct OnlinePlayer{
    std::string username;
    std::string game_ip;
    int game_port;
    int elo;
    std::string avatar_url;
};

class GameClient{
    private:
        std::string api_key;
        std::string server_url;
        bool is_online;
        std::string public_ip;
        std::string getStoragePath(){
            return "wizard_api_key.txt"
        }

        std::string fetchPublicIP(){
            httplib::Client client("https://api.ipify.org");
            auto res = client.Get("/");
            if (res && res->status == 200){
                return res->body;
            }
            return "0.0.0.0";
        }
    
    public:
        GameClient(cosnt std::string& url="httpL//localhost:3000") : server_url(url) , is_online(false),public_ip("0.0.0.0"){
            loadAPIKey();
        }

        void loadAPIKey(){
            std::ifstream file(getStoragePath());
            if (file.is_open()){
                std::getline(file,api_key);
                file.close();
            }
        }

        void saveAPIKey(const std::string& key){
            api_key = key;
            std::ofstream file(getStoragePath());
            if (file.is_open()){
                file << key;
                file.close();
            }
        }

        bool hasAPIKey(){
            return !api_key.empty();
        }

        std::string getAPIKey(){
            return api_key;
        }

        bool validateAndLogin(const std::string& key){
            httplib::Client client(server_url.c_str());
            client.set_bearer_token_auth(key);

            auto res = client.Get("/v1/profile");

            if (res && res->status==200){
                saveAPIKey(key);
                return true;
            }
            return false;
        }

        bool goOnline(int game_port){
            if (!hasAPIKey()) return false;

            if (public_ip=="0.0.0.0"){
                public_ip = fetchPublicIP();
            }

            httplib::Client client(server_url.c_str());
            client.set_bearer_token_auth(api_key);

            json_body = {
                {"game_ip",public_ip},
                {"game_port",game_port},
                {"status","online"}
            };

            auto res = client.Post("/v1/players/heartbeat",body.dump(),"application/json");

            if (res && res-> status == 200){
                is_online = true;
                return true;
            }
            return false;
        }

        void goOffline(){
            if (!hasAPIKey() || !is_online) return;

            httplib::Client client(server_url.c_str());
            client.set_bearer_token_auth(api_key);

            json body = {
                {"status","offline"}
            };

            client.Post("/v1/players/heartbeat",body.dump(),"application/json");
            is_online = false;
        }

            
}