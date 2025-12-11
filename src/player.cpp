#include "player.hpp"
#include "math_utils.hpp"
#include "world.hpp"
#include <math.h>

// Helper for AABB collision
bool CheckCollision(Vector3 pos, float radius, float height, World *world) {
  float minX = pos.x - radius;
  float maxX = pos.x + radius;
  float minY = pos.y - 1.5f;
  float maxY = pos.y + 0.3f;
  float minZ = pos.z - radius;
  float maxZ = pos.z + radius;

  for (int x = (int)floor(minX); x <= (int)floor(maxX); x++) {
    for (int y = (int)floor(minY); y <= (int)floor(maxY); y++) {
      for (int z = (int)floor(minZ); z <= (int)floor(maxZ); z++) {
        Block b = world->GetBlock(x, y, z);
        if (b.active)
          return true;
      }
    }
  }
  return false;
}

Player::Player() {}

void Player::Init() {
  camera.position = (Vector3){32.0f, 20.0f, 32.0f};
  camera.target = (Vector3){32.0f, 20.0f, 33.0f}; // Look forward Z
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 70.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  velocity = (Vector3){0.0f, 0.0f, 0.0f};
  isGrounded = false;
  gravity = 0.025f;
  jumpForce = 0.4f;
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
  hotbar[0] = {BLOCK_WOOD, 10};
  hotbar[1] = {BLOCK_STONE, 10};

  DisableCursor();
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
  // 1. Mouse Rotate
  Vector2 mouseDelta = GetMouseDelta();
  float sensitivity = 0.003f;

  // YAW (Rotate around Y axis)
  Vector3 forward = Vector3Subtract(camera.target, camera.position);
  Vector3 up = camera.up;

  // Rotate Yaw (around global Up if we want FPS style, or local up)
  // FPS style: rotate around 0,1,0
  forward = Vector3RotateByAxisAngle(forward, (Vector3){0, 1, 0},
                                     -mouseDelta.x * sensitivity);

  // PITCH (Rotate around Right axis)
  Vector3 right = Vector3CrossProduct(forward, up);
  right = Vector3Normalize(right);
  forward =
      Vector3RotateByAxisAngle(forward, right, -mouseDelta.y * sensitivity);

  // Re-apply to target (keep distance 1.0 for stability)
  forward = Vector3Normalize(forward);

  // Clamp Pitch (Prevent flipping)
  // Dot product with Up. If too close to 1 or -1, clamp.
  // Normalized Forward dot Up is sin(pitch).
  // Let's just trust it for now or add clamping.
  if (forward.y > 0.95f)
    forward.y = 0.95f;
  if (forward.y < -0.95f)
    forward.y = -0.95f;
  forward = Vector3Normalize(forward); // Re-normalize after clamp

  camera.target = Vector3Add(camera.position, forward);

  // 2. Physics Movement
  Vector3 direction = {0.0f, 0.0f, 0.0f};
  if (IsKeyDown(KEY_W))
    direction.z += 1.0f;
  if (IsKeyDown(KEY_S))
    direction.z -= 1.0f;
  if (IsKeyDown(KEY_A))
    direction.x -= 1.0f;
  if (IsKeyDown(KEY_D))
    direction.x += 1.0f;

  // Move relative to Yaw only (flattened forward)
  Vector3 flatForward = forward;
  flatForward.y = 0;
  flatForward = Vector3Normalize(flatForward);
  Vector3 flatRight = Vector3CrossProduct(flatForward, (Vector3){0, 1, 0});

  Vector3 moveDir = Vector3Add(Vector3Scale(flatForward, direction.z),
                               Vector3Scale(flatRight, direction.x));
  if (Vector3Length(moveDir) > 0)
    moveDir = Vector3Normalize(moveDir);

  velocity.x = moveDir.x * moveSpeed;
  velocity.z = moveDir.z * moveSpeed;

  // Gravity
  velocity.y -= gravity;
  if (velocity.y < -1.0f)
    velocity.y = -1.0f;

  // Jump
  if (isGrounded && IsKeyPressed(KEY_SPACE)) {
    velocity.y = jumpForce;
    isGrounded = false;
  }

  // Collision & Integration
  Vector3 pos = camera.position;

  // X
  pos.x += velocity.x;
  if (CheckCollision(pos, radius, height, world)) {
    pos.x -= velocity.x;
    velocity.x = 0;
  }

  // Z
  pos.z += velocity.z;
  if (CheckCollision(pos, radius, height, world)) {
    pos.z -= velocity.z;
    velocity.z = 0;
  }

  // Y
  pos.y += velocity.y;
  isGrounded = false;
  if (CheckCollision(pos, radius, height, world)) {
    if (velocity.y < 0)
      isGrounded = true;
    if (velocity.y > 0)
      velocity.y = 0;
    pos.y -= velocity.y;
    velocity.y = 0;
  }

  // Set final
  camera.position = pos;
  camera.target = Vector3Add(camera.position, forward);
}
