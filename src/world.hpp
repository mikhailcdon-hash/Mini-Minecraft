#pragma once
#include "../vendor/raylib/src/raylib.h"

#define WORLD_WIDTH 64
#define WORLD_HEIGHT 32
#define WORLD_DEPTH 64

enum BlockType {
  BLOCK_AIR = 0,
  BLOCK_DIRT,
  BLOCK_GRASS,
  BLOCK_STONE,
  BLOCK_WOOD,
  BLOCK_SAND,
  BLOCK_LEAVES
};

struct Block {
  bool active;
  BlockType type;
};

class World {
public:
  World();
  void Init();
  void Update();
  void Draw();
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

  // Textures & Model
  Texture2D blockTextures[10];
  Model cubeModel;

  // Helper to check if a block is hidden (surrounded by solids)
  bool IsBlockHidden(int x, int y, int z);

  Block grid[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];
};
