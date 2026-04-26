#ifndef MATCH_RECORDER_H
#define MATCH_RECORDER_H

#include "raylib.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class MatchRecorder {
private:
  std::ofstream file;
  std::chrono::steady_clock::time_point match_start_time;
  bool is_recording = false;
  std::string filename;

  int64_t getElapsedMs() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               now - match_start_time)
        .count();
  }

  std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &time_t); // safe on Windows
#else
    localtime_r(&time_t, &tm_info); // safe on POSIX/Linux
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_info, "%Y%m%d_%H%M%S");
    return oss.str();
  }

public:
  void startMatch(int player1_id, const std::string &player1_name,
                  int player2_id, const std::string &player2_name,
                  const std::string &external_id = "") {

    if (is_recording)
      return;

    std::string m_id = external_id.empty() ? getTimestamp() : external_id;
    filename =
        "duel_" + m_id + "_" + player1_name + "_vs_" + player2_name + ".wzrd";
    file.open(filename);

    if (!file.is_open()) {
      printf("ERROR: Could not create match log file!\n");
      return;
    }

    match_start_time = std::chrono::steady_clock::now();
    is_recording = true;

    // Write header
    file << "# WIZARD DUEL MATCH LOG v1.0\n";
    file << "# START_TIME: " << getTimestamp() << "\n";
    file << "# Format: CSV with timestamps\n\n";

    // Write metadata
    file << "0,METADATA," << player1_id << "," << player1_name << ","
         << player2_id << "," << player2_name << "\n";

    printf("Match recording started: %s\n", filename.c_str());
  }

  void logTrees(const std::vector<Vector2> &tree_positions) {
    if (!is_recording)
      return;

    for (size_t i = 0; i < tree_positions.size(); i++) {
      file << "0,TREE," << i << "," << tree_positions[i].x << ","
           << tree_positions[i].y << "\n";
    }
    file.flush();
  }

  void logSpellCast(int player_id, int spell_type, float target_x,
                    float target_y, float pos_x, float pos_y, float damage,
                    uint32_t spell_id) {
    if (!is_recording)
      return;

    int64_t elapsed = getElapsedMs();
    file << elapsed << ",SPELL," << player_id << "," << spell_type << ","
         << target_x << "," << target_y << "," << pos_x << "," << pos_y << ","
         << damage << "," << spell_id << "\n";
    file.flush(); // Important: flush so data isn't lost if game crashes
  }

  void logHit(int attacker_id, int target_id, float damage, uint32_t spell_id) {
    if (!is_recording)
      return;

    int64_t elapsed = getElapsedMs();
    file << elapsed << ",HIT," << attacker_id << "," << target_id << ","
         << damage << "," << spell_id << "\n";
    file.flush();
  }

  void logDeath(int victim_id, int killer_id) {
    if (!is_recording)
      return;

    int64_t elapsed = getElapsedMs();
    file << elapsed << ",DEATH," << victim_id << "," << killer_id << "\n";
    file.flush();
  }

  void logPosition(int player1_id, float p1_x, float p1_y, float p1_h,
                   float p1_m, int player2_id, float p2_x, float p2_y,
                   float p2_h, float p2_m) {
    if (!is_recording)
      return;

    int64_t elapsed = getElapsedMs();
    file << elapsed << ",POSITION," << player1_id << "," << p1_x << "," << p1_y
         << "," << p1_h << "," << p1_m << "," << player2_id << "," << p2_x
         << "," << p2_y << "," << p2_h << "," << p2_m << "\n";
  }

  void logHealthSync(int player_id, float health, float mana) {
    if (!is_recording)
      return;

    int64_t elapsed = getElapsedMs();
    file << elapsed << ",HEALTH_SYNC," << player_id << "," << health << ","
         << mana << "\n";
  }

  void endMatch(int winner_id, int loser_id) {
    if (!is_recording)
      return;

    int64_t elapsed = getElapsedMs();
    file << elapsed << ",MATCH_END," << winner_id << "," << loser_id << "\n";
    file.close();
    is_recording = false;

    printf("Match recording saved: %s\n", filename.c_str());
    printf("Duration: %.2f seconds\n", elapsed / 1000.0f);
  }

  bool isRecording() const { return is_recording; }
  const std::string &getFilename() const { return filename; }
  std::string getMatchId() {
    // Extract match ID from filename or just store it.
    // For simplicity, let's just use the timestamp part.
    size_t start = filename.find("duel_") + 5;
    size_t end = filename.find("_", start);
    if (start != std::string::npos && end != std::string::npos) {
      return filename.substr(start, end - start);
    }
    return "unknown";
  }

  ~MatchRecorder() {
    if (is_recording && file.is_open()) {
      file.close();
    }
  }
};

#endif