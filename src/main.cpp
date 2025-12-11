#include "math_utils.hpp"
#include "player.hpp"
#include "world.hpp"

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Mini Minecraft - C++");

  Player player;
  player.Init();

  World world;
  world.Init();

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    // Update
    player.Update(&world);
    world.Update();

    // Interaction
    // Breaking (Left Click)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Ray ray = {player.GetCamera().position,
                 Vector3Subtract(player.GetCamera().target,
                                 player.GetCamera().position)};
      World::WorldRayHit hit = world.GetRayCollision(ray);
      if (hit.hit) {
        Block b = world.GetBlock(hit.x, hit.y, hit.z);
        if (b.active) {
          player.AddItem(b.type, 1);
          world.SetBlock(hit.x, hit.y, hit.z, false, BLOCK_AIR);
        }
      }
    }

    // Placing (Right Click)
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      Ray ray = {player.GetCamera().position,
                 Vector3Normalize(Vector3Subtract(
                     player.GetCamera().target, player.GetCamera().position))};
      World::WorldRayHit hit = world.GetRayCollision(ray);
      if (hit.hit) {
        // Use grid coordinates directly for precision
        int nx = hit.x + (int)round(hit.normal.x);
        int ny = hit.y + (int)round(hit.normal.y);
        int nz = hit.z + (int)round(hit.normal.z);

        // Check collision with player AABB
        // Player radius 0.3, height 1.8. Eye at Y. Feet at Y-1.5.
        Vector3 pPos = player.GetPosition();

        float pMinX = pPos.x - 0.3f;
        float pMaxX = pPos.x + 0.3f;
        float pMinZ = pPos.z - 0.3f;
        float pMaxZ = pPos.z + 0.3f;
        float pMinY = pPos.y - 1.5f;
        float pMaxY = pPos.y + 0.3f;

        // Block AABB
        float bMinX = (float)nx - 0.5f;
        float bMaxX = (float)nx + 0.5f;
        float bMinY = (float)ny - 0.5f;
        float bMaxY = (float)ny + 0.5f;
        float bMinZ = (float)nz - 0.5f;
        float bMaxZ = (float)nz + 0.5f;

        bool collision = (pMinX < bMaxX && pMaxX > bMinX && pMinY < bMaxY &&
                          pMaxY > bMinY && pMinZ < bMaxZ && pMaxZ > bMinZ);

        if (!collision) {
          BlockType toPlace = player.GetSelectedBlockType();
          if (toPlace != BLOCK_AIR) {
            world.SetBlock(nx, ny, nz, true, toPlace);
            player.ConsumeItem();
          }
        }
      }
    }

    // Hotbar Selection
    if (IsKeyPressed(KEY_ONE))
      player.selectedSlot = 0;
    if (IsKeyPressed(KEY_TWO))
      player.selectedSlot = 1;
    if (IsKeyPressed(KEY_THREE))
      player.selectedSlot = 2;
    if (IsKeyPressed(KEY_FOUR))
      player.selectedSlot = 3;
    if (IsKeyPressed(KEY_FIVE))
      player.selectedSlot = 4;
    if (IsKeyPressed(KEY_SIX))
      player.selectedSlot = 5;
    if (IsKeyPressed(KEY_SEVEN))
      player.selectedSlot = 6;
    if (IsKeyPressed(KEY_EIGHT))
      player.selectedSlot = 7;
    if (IsKeyPressed(KEY_NINE))
      player.selectedSlot = 8;

    // Scroll Wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      player.selectedSlot -= (int)wheel;
      if (player.selectedSlot < 0)
        player.selectedSlot = 8;
      if (player.selectedSlot > 8)
        player.selectedSlot = 0;
    }

    // DRAWING
    BeginDrawing();
    ClearBackground((Color){135, 206, 235, 255}); // Sky Blue

    BeginMode3D(player.GetCamera());
    world.Draw();

    // Selection outline
    Ray ray = {player.GetCamera().position,
               Vector3Subtract(player.GetCamera().target,
                               player.GetCamera().position)};
    World::WorldRayHit hit = world.GetRayCollision(ray);
    if (hit.hit) {
      DrawCubeWires(Vector3Add(hit.position, (Vector3){0, 0, 0}), 1.01f, 1.01f,
                    1.01f, BLACK);
    }
    EndMode3D();

    // UI: Hotbar
    int startX = screenWidth / 2 - (9 * 45) / 2;
    int bottomY = screenHeight - 60;

    for (int i = 0; i < 9; i++) {
      int x = startX + i * 45;
      // Draw Slot Background
      Color slotColor = (i == player.selectedSlot) ? WHITE : LIGHTGRAY;
      DrawRectangleLines(x, bottomY, 40, 40, slotColor);
      DrawRectangle(x, bottomY, 40, 40, (Color){0, 0, 0, 100});

      // Draw Item
      if (player.hotbar[i].count > 0) {
        Texture2D tex = world.GetBlockTexture(player.hotbar[i].type);
        DrawTexturePro(tex,
                       (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                       (Rectangle){(float)x + 4, (float)bottomY + 4, 32, 32},
                       (Vector2){0, 0}, 0.0f, WHITE);

        // Draw Count
        DrawText(TextFormat("%d", player.hotbar[i].count), x + 22, bottomY + 24,
                 10, WHITE);
      }
    }

    DrawFPS(10, 10);

    // Crosshair
    DrawText("+", screenWidth / 2 - 5, screenHeight / 2 - 10, 20, WHITE);

    EndDrawing();
  }

  world.Unload();
  CloseWindow();

  return 0;
}
