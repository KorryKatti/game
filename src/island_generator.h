#ifndef ISLAND_GENERATOR_H
#define ISLAND_GENERATOR_H

#include "raylib.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

#define ISLAND_WIDTH 3000
#define ISLAND_HEIGHT 3000
#define PERLIN_OCTAVES 6

// Simple permutation table for Perlin noise
static int perlin_p[512];

void init_perlin(unsigned int seed) {
  srand(seed);
  
  // Initialize permutation table
  for (int i = 0; i < 256; i++) {
    perlin_p[i] = i;
  }
  
  // Shuffle it
  for (int i = 255; i > 0; i--) {
    int j = rand() % (i + 1);
    int temp = perlin_p[i];
    perlin_p[i] = perlin_p[j];
    perlin_p[j] = temp;
  }
  
  // Duplicate for wrapping
  for (int i = 0; i < 256; i++) {
    perlin_p[i + 256] = perlin_p[i];
  }
}

double fade(double t) {
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double lerp(double t, double a, double b) {
  return a + t * (b - a);
}

double grad(int hash, double x, double y) {
  int h = hash & 15;
  double u = h < 8 ? x : y;
  double v = h < 8 ? y : x;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double perlin_noise(double x, double y) {
  int xi = (int)floor(x) & 255;
  int yi = (int)floor(y) & 255;
  
  double xf = x - floor(x);
  double yf = y - floor(y);
  
  double u = fade(xf);
  double v = fade(yf);
  
  int aa = perlin_p[perlin_p[xi] + yi];
  int ab = perlin_p[perlin_p[xi] + yi + 1];
  int ba = perlin_p[perlin_p[xi + 1] + yi];
  int bb = perlin_p[perlin_p[xi + 1] + yi + 1];
  
  double x1 = lerp(u, grad(aa, xf, yf), grad(ba, xf - 1.0, yf));
  double x2 = lerp(u, grad(ab, xf, yf - 1.0), grad(bb, xf - 1.0, yf - 1.0));
  
  return lerp(v, x1, x2);
}

Color get_terrain_color(float height) {
  if (height < 0.15f) {
    // Deep water
    return (Color){25, 60, 120, 255};
  } else if (height < 0.30f) {
    // Shallow water
    return (Color){65, 105, 180, 255};
  } else if (height < 0.35f) {
    // Beach
    return (Color){238, 214, 175, 255};
  } else if (height < 0.60f) {
    // Grass
    return (Color){34, 139, 34, 255};
  } else if (height < 0.80f) {
    // Forest
    return (Color){25, 100, 25, 255};
  } else {
    // Mountains
    return (Color){139, 139, 139, 255};
  }
}

Texture2D generate_island(unsigned int seed) {
  init_perlin(seed);
  
  Image island = GenImageColor(ISLAND_WIDTH, ISLAND_HEIGHT, BLACK);
  
  float centerX = ISLAND_WIDTH / 2.0f;
  float centerY = ISLAND_HEIGHT / 2.0f;
  float maxDist = sqrt(centerX * centerX + centerY * centerY) * 0.7f;
  float scale = 50.0f;
  
  for (int y = 0; y < ISLAND_HEIGHT; y++) {
    for (int x = 0; x < ISLAND_WIDTH; x++) {
      // Multi-octave Perlin noise
      float value = 0.0f;
      float amplitude = 1.0f;
      float frequency = 1.0f;
      float maxValue = 0.0f;
      
      for (int i = 0; i < PERLIN_OCTAVES; i++) {
        value += perlin_noise(x / scale * frequency, y / scale * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
      }
      
      value /= maxValue;
      value = (value + 1.0f) / 2.0f;  // Normalize to 0-1
      
      // Island falloff (make edges water)
      float dx = (x - centerX) / maxDist;
      float dy = (y - centerY) / maxDist;
      float dist = sqrt(dx * dx + dy * dy);
      float falloff = fmax(0.0f, 1.0f - dist * dist);
      
      // Combine noise with falloff - more land, less water
      value = value * 0.6f + falloff * 0.4f;
      value *= falloff * 1.3f;  // Increased land mass
      
      Color color = get_terrain_color(value);
      ImageDrawPixel(&island, x, y, color);
    }
  }
  
  Texture2D texture = LoadTextureFromImage(island);
  UnloadImage(island);
  return texture;
}

#endif