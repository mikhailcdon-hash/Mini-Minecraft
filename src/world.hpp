#pragma once
#include "../vendor/raylib/src/raylib.h"

#define WORLD_WIDTH 1024
#define WORLD_HEIGHT 256
#define WORLD_DEPTH 1024
#define CHUNK_SIZE 16

enum BlockType {
  BLOCK_AIR = 0,
  BLOCK_DIRT,
  BLOCK_GRASS,
  BLOCK_STONE,
  BLOCK_WOOD,
  BLOCK_SAND,
  BLOCK_LEAVES,
  BLOCK_WATER
};

struct Block {
  bool active;
  BlockType type;
};

struct Chunk {
  Model model;
  bool active; // If it has any blocks
  bool dirty;  // Needs rebuild
  // We don't store blocks here, we keep the global grid for simplicity of logic
  // Just use the Chunk to hold the simplified mesh
};

class World {
public:
  World();
  void Init();
  void Update(Vector3 playerPos); // Added playerPos for future loading logic
  void Draw(Vector3 playerPos);   // Added playerPos for culling logic
  void Unload();

  Texture2D GetBlockTexture(BlockType type);

  Block GetBlock(int x, int y, int z);
  void SetBlock(int x, int y, int z, bool active, BlockType type);

  // Raycast support
  struct WorldRayHit {
    bool hit;
    Vector3 position; // Center of the block hit
    Vector3 normal;   // Normal of the face hit
    int x, y, z;      // Grid indices
    float distance;
  };

  WorldRayHit GetRayCollision(Ray ray);

private:
  void GenerateTerrain();
  void GenerateTree(int x, int y, int z);
  void RebuildChunk(int cx, int cy, int cz);

  // Textures
  Texture2D blockTextures[10];
  Texture2D atlasTexture; // Combined texture for chunks

  // Helper to check if a block is hidden (surrounded by solids)
  bool IsBlockHidden(int x, int y, int z);

  Block grid[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];
  Chunk chunks[WORLD_WIDTH / CHUNK_SIZE][WORLD_HEIGHT / CHUNK_SIZE]
              [WORLD_DEPTH / CHUNK_SIZE];
};
