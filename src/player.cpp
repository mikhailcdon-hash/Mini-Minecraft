#include "player.hpp"
#include "math_utils.hpp"
#include "world.hpp"
#include <math.h>

// Helper for AABB collision
bool CheckCollision(Vector3 pos, float radius, float height, World *world) {
  // Player AABB relative to camera position
  // Feet at pos.y - 1.5
  // Head at pos.y + 0.3 (Camera is 1.5 up, player height is 1.8)
  float padding = 0.05f; // Slight padding to prevent floating point errors
  float minX = pos.x - radius + padding;
  float maxX = pos.x + radius - padding;
  float minZ = pos.z - radius + padding;
  float maxZ = pos.z + radius - padding;
  float minY = pos.y - 1.5f + padding;
  float maxY = pos.y + 0.3f - padding;

  int startX = (int)floor(minX);
  int endX = (int)floor(maxX);
  int startY = (int)floor(minY);
  int endY = (int)floor(maxY);
  int startZ = (int)floor(minZ);
  int endZ = (int)floor(maxZ);

  for (int x = startX; x <= endX; x++) {
    for (int y = startY; y <= endY; y++) {
      for (int z = startZ; z <= endZ; z++) {
        Block b = world->GetBlock(x, y, z);
        if (b.active && b.type != BLOCK_WATER) // Water is passable
          return true;
      }
    }
  }
  return false;
}

Player::Player() {}

void Player::Init() {
  camera.position = (Vector3){
      32.5f, 150.0f,
      32.5f}; // Center in block, Spawning very high up for new world height
  camera.target = (Vector3){32.5f, 20.0f, 33.5f}; // Look forward Z
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 70.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  velocity = (Vector3){0.0f, 0.0f, 0.0f};
  isGrounded = false;
  gravity = 0.025f;
  jumpForce = 0.26f; // Reduced for ~1.25 block jump height
  moveSpeed = 0.15f;
  radius = 0.3f;
  height = 1.8f;

  // Inventory Init
  selectedSlot = 0;
  for (int i = 0; i < 9; i++) {
    hotbar[i].type = BLOCK_AIR;
    hotbar[i].count = 0;
  }
  // Give some starter items
  hotbar[0] = InventorySlot{BLOCK_WOOD, 64};
  hotbar[1] = InventorySlot{BLOCK_STONE, 64};
  hotbar[2] = InventorySlot{BLOCK_GRASS, 64};
  hotbar[3] = InventorySlot{BLOCK_DIRT, 64};

  DisableCursor();
  isFlying = false;

  walkTime = 0.0f;
  swingTimer = 0.0f;
}

void Player::Respawn() {
  camera.position = (Vector3){32.5f, 150.0f, 32.5f};
  velocity = (Vector3){0, 0, 0};
  isFlying = false;
}

void Player::TriggerSwing() { swingTimer = 1.0f; }

float Player::GetWalkBobbing() {
  return sinf(walkTime * 10.0f) * 0.1f; // Adjust frequency/amplitude
}

Camera3D Player::GetRenderCamera() {
  Camera3D rCam = camera;
  rCam.position.y += GetWalkBobbing();
  rCam.target.y += GetWalkBobbing(); // Look target also bobs
  return rCam;
}

float Player::GetHandBobbing() {
  return sinf(walkTime * 5.0f) * 10.0f; // Hand moves slower but more pixels
}

void Player::AddItem(BlockType type, int count) {
  // 1. Find existing stack
  for (int i = 0; i < 9; i++) {
    if (hotbar[i].type == type && hotbar[i].count < 64) {
      hotbar[i].count += count;
      return;
    }
  }
  // 2. Find empty slot
  for (int i = 0; i < 9; i++) {
    if (hotbar[i].count == 0 || hotbar[i].type == BLOCK_AIR) {
      hotbar[i].type = type;
      hotbar[i].count = count;
      return;
    }
  }
}

bool Player::ConsumeItem() {
  if (hotbar[selectedSlot].count > 0 &&
      hotbar[selectedSlot].type != BLOCK_AIR) {
    hotbar[selectedSlot].count--;
    if (hotbar[selectedSlot].count == 0) {
      hotbar[selectedSlot].type = BLOCK_AIR;
    }
    return true;
  }
  return false;
}

BlockType Player::GetSelectedBlockType() {
  if (hotbar[selectedSlot].count > 0)
    return hotbar[selectedSlot].type;
  return BLOCK_AIR;
}

void Player::Update(World *world) {
  // Toggle Flight
  if (IsKeyPressed(KEY_F))
    isFlying = !isFlying;

  // Respawn / Reset
  if (IsKeyPressed(KEY_R) || camera.position.y < -50.0f) {
    Respawn();
  }

  // 1. Mouse Rotate
  Vector2 mouseDelta = GetMouseDelta();
  float sensitivity = 0.003f;

  // YAW
  Vector3 forward = Vector3Subtract(camera.target, camera.position);
  Vector3 up = camera.up;
  forward = Vector3RotateByAxisAngle(forward, Vector3{0, 1, 0},
                                     -mouseDelta.x * sensitivity);

  // PITCH
  Vector3 right = Vector3CrossProduct(forward, up);
  right = Vector3Normalize(right);
  forward =
      Vector3RotateByAxisAngle(forward, right, -mouseDelta.y * sensitivity);

  forward = Vector3Normalize(forward);

  // Clamp Pitch
  if (forward.y > 0.95f)
    forward.y = 0.95f;
  if (forward.y < -0.95f)
    forward.y = -0.95f;
  forward = Vector3Normalize(forward);

  camera.target = Vector3Add(camera.position, forward);

  // 2. Movement
  if (isFlying) {
    float flySpeed = 0.5f;
    if (IsKeyDown(KEY_LEFT_SHIFT))
      flySpeed *= 2.0f;

    Vector3 moveDir = {0};
    if (IsKeyDown(KEY_W))
      moveDir = Vector3Add(moveDir, forward);
    if (IsKeyDown(KEY_S))
      moveDir = Vector3Subtract(moveDir, forward);
    if (IsKeyDown(KEY_A))
      moveDir = Vector3Subtract(moveDir, right);
    if (IsKeyDown(KEY_D))
      moveDir = Vector3Add(moveDir, right);

    // Normalize logic for flight
    if (Vector3Length(moveDir) > 0.01f)
      moveDir = Vector3Normalize(moveDir);

    camera.position =
        Vector3Add(camera.position, Vector3Scale(moveDir, flySpeed));

    if (IsKeyDown(KEY_SPACE))
      camera.position.y += flySpeed;
    if (IsKeyDown(KEY_LEFT_CONTROL))
      camera.position.y -= flySpeed;

    velocity = Vector3{0, 0, 0};
    camera.target = Vector3Add(camera.position, forward);
    return;
  }

  // WALKING PHYSICS
  Vector3 direction = Vector3{0.0f, 0.0f, 0.0f};
  if (IsKeyDown(KEY_W))
    direction.z += 1.0f;
  if (IsKeyDown(KEY_S))
    direction.z -= 1.0f;
  if (IsKeyDown(KEY_A))
    direction.x += 1.0f; // Inverted: Was -, now +
  if (IsKeyDown(KEY_D))
    direction.x -= 1.0f; // Inverted: Was +, now -

  // Move relative to Yaw only
  Vector3 flatForward = forward;
  flatForward.y = 0;
  flatForward = Vector3Normalize(flatForward);
  Vector3 flatRight = Vector3CrossProduct(flatForward, Vector3{0, 1, 0});

  Vector3 moveDir = Vector3Add(Vector3Scale(flatForward, direction.z),
                               Vector3Scale(flatRight, direction.x));
  if (Vector3Length(moveDir) > 0)
    moveDir = Vector3Normalize(moveDir);

  // Sprint Logic
  float currentSpeed = moveSpeed;
  if (IsKeyDown(KEY_LEFT_SHIFT)) {
    currentSpeed *= 1.7f; // Sprint multiplier
  }

  velocity.x = moveDir.x * currentSpeed;
  velocity.z = moveDir.z * currentSpeed;

  // Animation Update
  if (Vector3Length(velocity) > 0.01f && isGrounded && !isFlying) {
    walkTime += GetFrameTime() * (IsKeyDown(KEY_LEFT_SHIFT) ? 1.5f : 1.0f);
  } else {
    // Dampen back to 0 or just stop incrementing?
    // For simple bobbing, just stop incrementing is fine, maybe snap to 0
    // slowly? taking simple approach:
    if (walkTime > 0)
      walkTime = 0; // Reset for now so we don't stop mid-bob ideally
  }

  if (swingTimer > 0)
    swingTimer -= GetFrameTime() * 5.0f; // Swing speed

  velocity.y -= gravity;
  if (velocity.y < -1.0f)
    velocity.y = -1.0f;

  if (isGrounded && IsKeyPressed(KEY_SPACE)) {
    velocity.y = jumpForce;
    isGrounded = false;
  }

  // Collision & Integration
  Vector3 pos = camera.position;

  // Use sub-stepping to prevent tunneling
  int steps = 5;
  Vector3 stepVel = Vector3Scale(velocity, 1.0f / steps);

  for (int i = 0; i < steps; i++) {
    // X Axis
    pos.x += stepVel.x;
    if (CheckCollision(pos, radius, height, world)) {
      pos.x -= stepVel.x;
      stepVel.x = 0;
      velocity.x = 0;
    }

    // Z Axis
    pos.z += stepVel.z;
    if (CheckCollision(pos, radius, height, world)) {
      pos.z -= stepVel.z;
      stepVel.z = 0;
      velocity.z = 0;
    }

    // Y Axis
    pos.y += stepVel.y;
    if (CheckCollision(pos, radius, height, world)) {
      // If moving down (falling)
      if (stepVel.y < 0) {
        isGrounded = true;
      }
      // If moving up (jumping)
      if (stepVel.y > 0) {
        velocity.y = 0; // Stop upward momentum
      }
      pos.y -= stepVel.y;
      stepVel.y = 0;
      velocity.y = 0;
    }
  }

  // Update camera
  camera.position = pos;
  camera.target = Vector3Add(camera.position, forward);

  // Apply View Bobbing to Camera Target (look) or Position?
  // Usually modifies Y position visually.
  // Note: We modify camera.position temporarily for rendering?
  // Actually, standard FPS bobbing modifies the eye height.
  // We can just add it to camera.position.y here, but we need to remove it next
  // frame or reset it before collision checks. Better approach: Since 'pos' is
  // the physics position, we set camera.position = pos, THEN add bob. BUT next
  // frame we use camera.position as physics start... So we should store physics
  // position separate from camera position? Current architecture uses
  // camera.position as physics position. Fix: Add bobbing only when setting
  // final camera properties, but ensure we strip it or ignore it for physics.
  // Actually, simple hack: Add bobbing to 'camera.position.y' here,
  // and in next frame CheckCollision, we use (pos.y - 1.5 - bob) ? No that's
  // messy.

  // Correct way with current simple setup:
  // We won't modify camera.position permanently.
  // We will leave physics position in camera.position.
  // This means the bobbing is NOT applied to the actual camera object here,
  // but we might need to apply it in the DRAW cycle?
  // Raylib's BeginMode3D uses the camera object passed to it.
  // So we can just create a "renderCamera" in main.cpp that adds the bob.
  // OR we add a method GetRenderCamera().
}
