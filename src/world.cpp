#include "world.hpp"
#include <math.h>
#include <stdlib.h> // rand

World::World() {
  // Constructor
}

void World::Init() {
  // 1. Generate Textures (Procedural)
  // We create simple patterns to mimic textures

  // GRASS
  Image imgGrass = GenImageColor(16, 16, GREEN);
  ImageColorReplace(&imgGrass, GREEN,
                    (Color){34, 139, 34, 255}); // Forest Green
  // Add noise
  for (int i = 0; i < 64; i++)
    ImageDrawPixel(&imgGrass, rand() % 16, rand() % 16,
                   (Color){50, 205, 50, 255}); // Lime accents
  blockTextures[BLOCK_GRASS] = LoadTextureFromImage(imgGrass);
  UnloadImage(imgGrass);

  // DIRT
  Image imgDirt = GenImageColor(16, 16, BROWN);
  for (int i = 0; i < 64; i++)
    ImageDrawPixel(&imgDirt, rand() % 16, rand() % 16,
                   (Color){100, 50, 20, 255}); // Darker brown
  blockTextures[BLOCK_DIRT] = LoadTextureFromImage(imgDirt);
  UnloadImage(imgDirt);

  // STONE
  Image imgStone = GenImageColor(16, 16, DARKGRAY);
  for (int i = 0; i < 100; i++)
    ImageDrawPixel(&imgStone, rand() % 16, rand() % 16,
                   (Color){100, 100, 100, 255}); // Light gray noise
  blockTextures[BLOCK_STONE] = LoadTextureFromImage(imgStone);
  UnloadImage(imgStone);

  // WOOD
  Image imgWood =
      GenImageColor(16, 16, (Color){139, 69, 19, 255}); // SaddleBrown
  ImageDrawLine(&imgWood, 4, 0, 4, 16,
                (Color){160, 82, 45, 255}); // Vertical streaks
  ImageDrawLine(&imgWood, 10, 0, 10, 16, (Color){120, 50, 10, 255});
  blockTextures[BLOCK_WOOD] = LoadTextureFromImage(imgWood);
  UnloadImage(imgWood);

  // SAND
  Image imgSand = GenImageColor(16, 16, BEIGE);
  for (int i = 0; i < 50; i++)
    ImageDrawPixel(&imgSand, rand() % 16, rand() % 16, GOLD);
  blockTextures[BLOCK_SAND] = LoadTextureFromImage(imgSand);
  UnloadImage(imgSand);

  // LEAVES
  Image imgLeaves = GenImageColor(
      16, 16,
      (Color){0, 100, 0,
              255}); // Dark Green transparent-ish? No, opaque for now
  for (int i = 0; i < 40; i++)
    ImageDrawPixel(&imgLeaves, rand() % 16, rand() % 16,
                   (Color){0, 150, 0, 255});
  blockTextures[BLOCK_LEAVES] = LoadTextureFromImage(imgLeaves);
  UnloadImage(imgLeaves);

  // Use Dirt texture as default/fallback for others
  blockTextures[BLOCK_AIR] = blockTextures[BLOCK_DIRT]; // Should never draw air

  // Initialize Cube Model for textured drawing
  Mesh mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
  cubeModel = LoadModelFromMesh(mesh);

  // 2. Clear World
  for (int x = 0; x < WORLD_WIDTH; x++) {
    for (int y = 0; y < WORLD_HEIGHT; y++) {
      for (int z = 0; z < WORLD_DEPTH; z++) {
        grid[x][y][z].active = false;
        grid[x][y][z].type = BLOCK_AIR;
      }
    }
  }

  // 3. Generate Terrain
  GenerateTerrain();
}

void World::Unload() {
  for (int i = 1; i < 7; i++) {
    UnloadTexture(blockTextures[i]);
  }
  UnloadModel(cubeModel);
}

Texture2D World::GetBlockTexture(BlockType type) {
  if (type < 0 || type > 9)
    return blockTextures[BLOCK_DIRT];
  return blockTextures[type];
}

void World::GenerateTerrain() {
  // Simple Perlin-like noise using PerlinNoise from Raylib or just Sine waves
  // Since Perlin might need extra includes, let's just use Sin waves for hills

  // Smoother noise for hills
  Image noiseMap = GenImagePerlinNoise(WORLD_WIDTH, WORLD_DEPTH, 0, 0, 0.5f);
  Color *pixels = LoadImageColors(noiseMap);

  for (int x = 0; x < WORLD_WIDTH; x++) {
    for (int z = 0; z < WORLD_DEPTH; z++) {
      // Get height from noise (0-255)
      // Scale reduced to 0.5 for smoother hills
      // Amplitude increased slightly: 0-16 height variance
      int height = (pixels[z * WORLD_WIDTH + x].r / 255.0f) * 12 + 2;

      for (int y = 0; y <= height; y++) {
        BlockType type = BLOCK_DIRT;
        if (y == height)
          type = BLOCK_GRASS;
        if (y < height - 3)
          type = BLOCK_STONE;

        // Sand near water level (let's say y=4 is water level, though we have
        // no water yet)
        if (y == height && y <= 4)
          type = BLOCK_SAND;

        SetBlock(x, y, z, true, type);
      }

      // Random Trees
      if (grid[x][height][z].type == BLOCK_GRASS &&
          (rand() % 100) < 5) { // 5% chance
        GenerateTree(x, height + 1, z);
      }
    }
  }

  UnloadImageColors(pixels);
  UnloadImage(noiseMap);
}

void World::GenerateTree(int x, int y, int z) {
  if (x < 2 || x >= WORLD_WIDTH - 2 || z < 2 || z >= WORLD_DEPTH - 2 ||
      y >= WORLD_HEIGHT - 6)
    return;

  int treeHeight = 4 + rand() % 3;

  // Trunk
  for (int i = 0; i < treeHeight; i++) {
    SetBlock(x, y + i, z, true, BLOCK_WOOD);
  }

  // Leaves
  for (int lx = x - 2; lx <= x + 2; lx++) {
    for (int lz = z - 2; lz <= z + 2; lz++) {
      for (int ly = y + treeHeight - 2; ly <= y + treeHeight + 1; ly++) {
        // Sphere-ish logic
        if (abs(lx - x) + abs(ly - (y + treeHeight)) + abs(lz - z) <= 3) {
          if (!grid[lx][ly][lz].active) {
            SetBlock(lx, ly, lz, true, BLOCK_LEAVES);
          }
        }
      }
    }
  }
}

void World::Update() {
  // Loop
}

bool World::IsBlockHidden(int x, int y, int z) {
  // Return true if surrounded by opaque blocks
  // Check bounds. If on boundary, not hidden (unless we wrap, but we don't)
  if (x == 0 || x == WORLD_WIDTH - 1 || y == 0 || y == WORLD_HEIGHT - 1 ||
      z == 0 || z == WORLD_DEPTH - 1)
    return false;

  // Check 6 neighbors
  // We assume all non-air blocks are opaque (leaves are technically transparent
  // in MC, but opaque here) Actually, leaves should verify visibility. But
  // let's keep it simple: assume all active blocks occlude.

  if (!grid[x + 1][y][z].active)
    return false;
  if (!grid[x - 1][y][z].active)
    return false;
  if (!grid[x][y + 1][z].active)
    return false;
  if (!grid[x][y - 1][z].active)
    return false;
  if (!grid[x][y][z + 1].active)
    return false;
  if (!grid[x][y][z - 1].active)
    return false;

  return true;
}

void World::Draw() {
  // Culling distance (optional)
  // For now, just draw grid

  for (int x = 0; x < WORLD_WIDTH; x++) {
    for (int y = 0; y < WORLD_HEIGHT; y++) {
      for (int z = 0; z < WORLD_DEPTH; z++) {
        if (grid[x][y][z].active) {
          // Optimization: Hidden surface removal
          if (IsBlockHidden(x, y, z))
            continue;

          Vector3 pos = {(float)x, (float)y, (float)z};
          BlockType type = grid[x][y][z].type;

          // Set texture for the model
          // We only have one material, so we change it per block (inefficient
          // but works for 64x64) Better approach would be batching but that's
          // complex.
          cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
              blockTextures[type];

          DrawModel(cubeModel, pos, 1.0f, WHITE);
        }
      }
    }
  }
}

Block World::GetBlock(int x, int y, int z) {
  if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT && z >= 0 &&
      z < WORLD_DEPTH) {
    return grid[x][y][z];
  }
  return {false, BLOCK_AIR};
}

World::WorldRayHit World::GetRayCollision(Ray ray) {
  WorldRayHit closestHit = {false};
  closestHit.distance = 999999.0f;

  // TODO: Optimization - Voxel Traversal Algorithm (DDA) is much better than
  // iterating 131072 blocks. However, for this step, we can limit the brute
  // force range to the player's vicinity. Or iterate only a small window around
  // the ray?

  // Let's stick to the current brute force but Optimized range?
  // Ray origin is player pos. We only need to check blocks within radius 8.

  int cx = (int)ray.position.x;
  int cy = (int)ray.position.y;
  int cz = (int)ray.position.z;
  int radius = 10;

  int minX = cx - radius;
  if (minX < 0)
    minX = 0;
  int maxX = cx + radius;
  if (maxX >= WORLD_WIDTH)
    maxX = WORLD_WIDTH - 1;
  int minY = cy - radius;
  if (minY < 0)
    minY = 0;
  int maxY = cy + radius;
  if (maxY >= WORLD_HEIGHT)
    maxY = WORLD_HEIGHT - 1;
  int minZ = cz - radius;
  if (minZ < 0)
    minZ = 0;
  int maxZ = cz + radius;
  if (maxZ >= WORLD_DEPTH)
    maxZ = WORLD_DEPTH - 1;

  for (int x = minX; x <= maxX; x++) {
    for (int y = minY; y <= maxY; y++) {
      for (int z = minZ; z <= maxZ; z++) {
        if (!grid[x][y][z].active)
          continue;

        BoundingBox box = {
            (Vector3){(float)x - 0.5f, (float)y - 0.5f, (float)z - 0.5f},
            (Vector3){(float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f}};

        RayCollision collision = GetRayCollisionBox(ray, box);
        if (collision.hit && collision.distance < closestHit.distance) {
          closestHit.hit = true;
          closestHit.distance = collision.distance;
          closestHit.x = x;
          closestHit.y = y;
          closestHit.z = z;
          closestHit.normal = collision.normal;
        }
      }
    }
  }
  return closestHit;
}

void World::SetBlock(int x, int y, int z, bool active, BlockType type) {
  if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT && z >= 0 &&
      z < WORLD_DEPTH) {
    grid[x][y][z].active = active;
    grid[x][y][z].type = type;
  }
}
