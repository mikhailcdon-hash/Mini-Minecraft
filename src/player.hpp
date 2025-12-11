#pragma once
#include "../vendor/raylib/src/raylib.h"
#include "world.hpp"

class World; // Forward declaration

class Player {
public:
  Player();
  void Init();
  void Update(World *world); // Handle input and movement with collision

  Camera3D GetCamera() { return camera; }
  Vector3 GetPosition() { return camera.position; }

  // Inventory
  struct InventorySlot {
    BlockType type;
    int count;
  };

  InventorySlot hotbar[9];
  int selectedSlot;

  void AddItem(BlockType type, int count = 1);
  bool ConsumeItem(); // Returns true if item was consumed (count > 0)
  BlockType GetSelectedBlockType();

private:
  Camera3D camera;
  Vector3 velocity;
  bool isGrounded;
  float gravity;
  float jumpForce;
  float moveSpeed;

  // Simple Bounding Box (AABB) relative to position
  // Size: radius 0.3, height 1.8
  float radius;
  float height;
};
