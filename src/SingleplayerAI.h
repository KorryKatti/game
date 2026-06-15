#pragma once
#include "raylib.h"
#include <cmath>
#include <vector>

struct AIState {
  float attackCooldown = 0.0f;
  float moveTimer = 0.0f;
  int spellChoice = 1;
  float strafeDir = 0.0f;
  float strafeTimer = 0.0f;
};

inline void updateSingleplayerAI(Character &ai, const Character &player,
                                 std::vector<Ball> &ball_vec,
                                 uint32_t &next_spell_id, float deltaTime) {
  static AIState aiState;

  aiState.attackCooldown -= deltaTime;
  aiState.moveTimer -= deltaTime;
  aiState.strafeTimer -= deltaTime;

  // Regenerate mana
  if (ai.mana < 300.0f)
    ai.mana += 0.08f;
  if (ai.mana > 300.0f)
    ai.mana = 300.0f;

  // Regenerate health slowly
  if (ai.health < 300.0f && ai.health > 0.0f)
    ai.health += 0.02f;

  if (ai.health <= 0.0f || ai.isDead)
    return;

  float dx = player.pos.x - ai.pos.x;
  float dy = player.pos.y - ai.pos.y;
  float dist = sqrtf(dx * dx + dy * dy);

  // Decide direction to face player
  if (fabsf(dx) > fabsf(dy)) {
    ai.direction = (dx > 0) ? 2 : 1;
  } else {
    ai.direction = (dy > 0) ? 0 : 3;
  }

  // Movement: stay at medium range (200-350)
  float idealDist = 280.0f;
  float moveSpeed = 0.35f;

  if (dist > idealDist + 80.0f) {
    // Too far, move toward player
    if (dist > 1.0f) {
      ai.pos.x += (dx / dist) * moveSpeed;
      ai.pos.y += (dy / dist) * moveSpeed;
    }
    ai.animTimer += deltaTime;
    if (ai.animTimer >= 0.12f) {
      ai.animTimer = 0.0f;
      ai.frame = (ai.frame + 1) % 8;
    }
  } else if (dist < idealDist - 80.0f) {
    // Too close, back away
    if (dist > 1.0f) {
      ai.pos.x -= (dx / dist) * moveSpeed;
      ai.pos.y -= (dy / dist) * moveSpeed;
    }
    ai.animTimer += deltaTime;
    if (ai.animTimer >= 0.12f) {
      ai.animTimer = 0.0f;
      ai.frame = (ai.frame + 1) % 8;
    }
  } else {
    // In range, strafe sideways
    if (aiState.strafeTimer <= 0.0f) {
      aiState.strafeDir = (float)((rand() % 3) - 1); // -1, 0, or 1
      aiState.strafeTimer = 0.5f + (float)(rand() % 100) / 200.0f;
    }

    if (aiState.strafeDir != 0.0f) {
      // Perpendicular to player direction
      float perpX = -dy / dist;
      float perpY = dx / dist;
      ai.pos.x += perpX * aiState.strafeDir * moveSpeed * 0.6f;
      ai.pos.y += perpY * aiState.strafeDir * moveSpeed * 0.6f;

      if (aiState.strafeDir > 0)
        ai.direction = 2;
      else
        ai.direction = 1;

      ai.animTimer += deltaTime;
      if (ai.animTimer >= 0.12f) {
        ai.animTimer = 0.0f;
        ai.frame = (ai.frame + 1) % 8;
      }
    } else {
      ai.frame = 0;
      ai.animTimer = 0.0f;
    }
  }

  ai.rect.x = ai.frame * 77;
  ai.rect.y = ai.direction * 77;

  // Attack: cast spells at the player
  if (aiState.attackCooldown <= 0.0f && dist < 500.0f) {
    float manaCost = 5.0f;

    // Mana Burn: use when player is low on mana (prevent healing) or when AI has low mana
    if (aiState.spellChoice == 3 && ai.mana >= 20.0f) {
      Ball new_ball;
      new_ball.spell_id = next_spell_id++;
      new_ball.owner_id = ai.id;
      new_ball.target_pos = player.pos;
      new_ball.ball_pos = ai.pos;
      new_ball.spellColor = PURPLE;
      new_ball.ball_speed = 3.0f;
      new_ball.ball_r = 20.0f;
      new_ball.damage = 30.0f;
      new_ball.has_hit = false;
      new_ball.mana_burn = true;
      ball_vec.push_back(new_ball);
      ai.mana -= 10.0f;
      ai.is_cast = true;
      aiState.attackCooldown = 1.0f + (float)(rand() % 100) / 200.0f;
      aiState.spellChoice = 1;

    } else if (aiState.spellChoice == 1 && ai.mana >= 25.0f) {
      Ball new_ball;
      new_ball.spell_id = next_spell_id++;
      new_ball.owner_id = ai.id;
      new_ball.target_pos = player.pos;
      new_ball.ball_pos = ai.pos;
      new_ball.spellColor = {128, 0, 0, 255};
      new_ball.ball_speed = 2.0f;
      new_ball.ball_r = 35.0f;
      new_ball.damage = 35.0f;
      new_ball.has_hit = false;
      ball_vec.push_back(new_ball);
      ai.mana -= manaCost;
      ai.is_cast = true;
      aiState.attackCooldown = 1.2f + (float)(rand() % 100) / 200.0f;
      // After blood red, chance to use mana burn if player mana is low
      if (player.mana < 100.0f && (rand() % 3 == 0)) {
        aiState.spellChoice = 3;
      } else {
        aiState.spellChoice = 2;
      }

    } else if (aiState.spellChoice == 2 && ai.mana >= 35.0f) {
      Ball new_ball;
      new_ball.spell_id = next_spell_id++;
      new_ball.owner_id = ai.id;
      new_ball.target_pos = player.pos;
      new_ball.ball_pos = ai.pos;
      new_ball.spellColor = DARKBLUE;
      new_ball.ball_speed = 4.0f;
      new_ball.ball_r = 25.0f;
      new_ball.damage = 25.0f;
      new_ball.has_hit = false;
      ball_vec.push_back(new_ball);
      ai.mana -= manaCost;
      ai.is_cast = true;
      aiState.attackCooldown = 0.8f + (float)(rand() % 100) / 200.0f;
      aiState.spellChoice = 1;
    }
  }

  // Blink: dodge when low health
  if (ai.health < 100.0f && ai.mana >= 40.0f && dist < 400.0f) {
    // Blink away from player
    float blinkDist = 300.0f;
    Vector2 blinkTarget = {
      ai.pos.x - (dx / dist) * blinkDist,
      ai.pos.y - (dy / dist) * blinkDist
    };

    // Clamp to map bounds
    if (blinkTarget.x < 0) blinkTarget.x = 0;
    if (blinkTarget.y < 0) blinkTarget.y = 0;
    if (blinkTarget.x > 3000) blinkTarget.x = 3000;
    if (blinkTarget.y > 3000) blinkTarget.y = 3000;

    ai.pos = blinkTarget;
    ai.mana -= 40.0f;
    aiState.attackCooldown = 0.5f;
  }
}

inline void resetSingleplayerAI() {
  static AIState aiState;
  aiState.attackCooldown = 0.0f;
  aiState.moveTimer = 0.0f;
  aiState.spellChoice = 1;
  aiState.strafeDir = 0.0f;
  aiState.strafeTimer = 0.0f;
}
