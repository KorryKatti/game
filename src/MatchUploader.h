// MatchUploader.h
#ifndef MATCH_UPLOADER_H
#define MATCH_UPLOADER_H
#include "httplib.h"
#include <fstream>
#include <sstream>
#include <thread>

class MatchUploader {
private:
  std::string server_url;

public:
  MatchUploader(const std::string &url = "http://localhost:3000")
      : server_url(url) {}

  bool uploadMatch(const std::string &filename, const std::string &player1_name,
                   const std::string &player2_name,
                   const std::string &winner_name, int duration_seconds,
                   const std::string &match_id) {
    // Read file content
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      printf("ERROR: Cannot open match file for upload: %s\n",
             filename.c_str());
      return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_content = buffer.str();
    file.close();

    // Create HTTP client
    httplib::Client client(server_url.c_str());

    // Build multipart form data manually
    std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    std::string body;

    // Add form fields
    auto add_field = [&](const std::string &name, const std::string &value) {
      body += "--" + boundary + "\r\n";
      body += "Content-Disposition: form-data; name=\"" + name + "\"\r\n\r\n";
      body += value + "\r\n";
    };

    add_field("player1", player1_name);
    add_field("player2", player2_name);
    add_field("winner", winner_name);
    add_field("duration", std::to_string(duration_seconds));
    add_field("match_id", match_id);

    // Add file field
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"match_file\"; filename=\"" +
            filename + "\"\r\n";
    body += "Content-Type: text/plain\r\n\r\n";
    body += file_content + "\r\n";
    body += "--" + boundary + "--\r\n";

    // Create headers
    httplib::Headers headers;
    headers.emplace("Content-Type",
                    "multipart/form-data; boundary=" + boundary);

    auto res = client.Post("/upload_match", headers, body,
                           "multipart/form-data; boundary=" + boundary);

    if (res && res->status == 200) {
      printf("Match uploaded successfully!\n");
      return true;
    } else {
      printf("Upload failed: %s\n", res ? "HTTP error" : "Connection failed");
      return false;
    }
  }

  // Async version (non-blocking - call this after match ends)
  void uploadMatchAsync(const std::string &filename,
                        const std::string &player1_name,
                        const std::string &player2_name,
                        const std::string &winner_name, int duration_seconds,
                        const std::string &match_id) {
    std::thread([=]() {
      uploadMatch(filename, player1_name, player2_name, winner_name,
                  duration_seconds, match_id);
    }).detach();
  }
};

#endif