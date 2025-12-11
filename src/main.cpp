#include "math_utils.hpp"
#include "player.hpp"
#include "world.hpp"

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Mini Minecraft - C++");

  Player player;
  player.Init();

  // Allocate World on the Heap to avoid Stack Limit with 256x256x256
  // (256*256*256 * sizeof(Block) is large)
  World *world = new World();
  world->Init();

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    // State Machine
    static enum { TITLE, GAMEPLAY } currentScreen = TITLE;

    // Update Music/Audio (if any) here

    if (currentScreen == TITLE) {
      // Title Screen Logic
      if (IsKeyPressed(KEY_ENTER)) {
        currentScreen = GAMEPLAY;
        player.Respawn(); // Ensure clean start? Or just continue
        DisableCursor();
      }
    } else {
      // Gameplay Logic
      // Update
      player.Update(world);
      world->Update(player.GetPosition());

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

      // Return to Menu?
      // if (IsKeyPressed(KEY_M)) { EnableCursor(); currentScreen = TITLE; }
    }

    // DRAWING
    BeginDrawing();
    ClearBackground((Color){110, 180, 220, 255}); // Softer Sky Blue

    if (currentScreen == TITLE) {
      DrawText("MINI MINECRAFT",
               screenWidth / 2 - MeasureText("MINI MINECRAFT", 40) / 2,
               screenHeight / 2 - 60, 40, WHITE);
      DrawText("Press ENTER to Start",
               screenWidth / 2 - MeasureText("Press ENTER to Start", 20) / 2,
               screenHeight / 2 + 10, 20, LIGHTGRAY);
      DrawText("WASD to Move, SPACE to Jump, CLICK to Mine/Place",
               screenWidth / 2 -
                   MeasureText(
                       "WASD to Move, SPACE to Jump, CLICK to Mine/Place", 15) /
                       2,
               screenHeight - 60, 15, WHITE);
      DrawText("R to Respawn, F to Fly",
               screenWidth / 2 - MeasureText("R to Respawn, F to Fly", 15) / 2,
               screenHeight - 40, 15, WHITE);
    } else {
      // GAMEPLAY DRAWING
      // Use RenderCamera for View Bobbing
      BeginMode3D(player.GetRenderCamera());
      world->Draw(player.GetPosition());

      // Selection outline
      Ray ray = {player.GetRenderCamera().position,
                 Vector3Subtract(player.GetRenderCamera().target,
                                 player.GetRenderCamera().position)};
      World::WorldRayHit hit = world->GetRayCollision(ray);
      if (hit.hit) {
        DrawCubeWires(Vector3Add(hit.position, (Vector3){0, 0, 0}), 1.01f,
                      1.01f, 1.01f, BLACK);
      }
      EndMode3D();

      // Interaction Logic inside Drawing loop?
      // Ideally Logic should be separate, but for single-threaded simple games
      // it's often mixed or placed before drawing. We moved the Update logic
      // above. Now we just need to verify the Interaction code
      // (Breaking/Placing) which was effectively 'Update' logic but using
      // rendering data (Ray). Since we need Ray collision for interaction, it's
      // easier to keep it here OR duplicate the ray logic. Let's keep
      // interaction here but wrap it in "if (currentScreen == GAMEPLAY)"

      // INTERACTION (Moved inside checking)
      // Breaking (Left Click)
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        player.TriggerSwing(); // ANIMATION
        // recalculate ray for logic
        Ray logicRay = {player.GetRenderCamera().position,
                        Vector3Subtract(player.GetRenderCamera().target,
                                        player.GetRenderCamera().position)};
        World::WorldRayHit hitData = world->GetRayCollision(logicRay);
        if (hitData.hit) {
          Block b = world->GetBlock(hitData.x, hitData.y, hitData.z);
          if (b.active) {
            player.AddItem(b.type, 1);
            world->SetBlock(hitData.x, hitData.y, hitData.z, false, BLOCK_AIR);
          }
        }
      }

      // Placing (Right Click)
      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        player.TriggerSwing(); // ANIMATION
        Ray logicRay = {player.GetRenderCamera().position,
                        Vector3Normalize(Vector3Subtract(
                            player.GetRenderCamera().target,
                            player.GetRenderCamera().position))};
        World::WorldRayHit hitData = world->GetRayCollision(logicRay);
        if (hitData.hit) {
          // Use grid coordinates directly for precision
          int nx = hitData.x + (int)round(hitData.normal.x);
          int ny = hitData.y + (int)round(hitData.normal.y);
          int nz = hitData.z + (int)round(hitData.normal.z);

          // Check collision with player AABB
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
              world->SetBlock(nx, ny, nz, true, toPlace);
              player.ConsumeItem();
            }
          }
        }
      }

      // HAND ANIMATION
      if (player.GetSelectedBlockType() != BLOCK_AIR) {
        Texture2D tex = world->GetBlockTexture(player.GetSelectedBlockType());

        float handBob = player.GetHandBobbing();
        float t = player.swingTimer;

        float curve = 1.0f - (1.0f - t) * (1.0f - t);

        float swingRot = sinf(t * 3.14159f) * -80.0f;
        float swingX = sinf(t * 3.14159f) * 20.0f;
        float swingY = sinf(t * 3.14159f) * 60.0f;

        float scale = 4.0f;
        float handX =
            screenWidth - (tex.width * scale) - 60 + swingRot * 0.5f + swingX;

        float handY =
            screenHeight - (tex.height * scale) + handBob + swingY + 20;

        Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
        Rectangle dest = {handX, handY, (float)tex.width * scale,
                          (float)tex.height * scale};

        DrawTexturePro(tex, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
      }

      // UI: Hotbar (Logic mostly duplicated from before but needs to be inside
      // GAMEPLAY check)
      int startX = screenWidth / 2 - (9 * 45) / 2;
      int bottomY = screenHeight - 60;

      // Draw Selected Block Name
      BlockType currentType = player.GetSelectedBlockType();
      const char *blockName = "";
      switch (currentType) {
      case BLOCK_DIRT:
        blockName = "Dirt";
        break;
      case BLOCK_GRASS:
        blockName = "Grass";
        break;
      case BLOCK_STONE:
        blockName = "Stone";
        break;
      case BLOCK_WOOD:
        blockName = "Wood";
        break;
      case BLOCK_SAND:
        blockName = "Sand";
        break;
      case BLOCK_LEAVES:
        blockName = "Leaves";
        break;
      case BLOCK_WATER:
        blockName = "Water bucket";
        break;
      default:
        blockName = "";
        break;
      }
      if (player.GetSelectedBlockType() != BLOCK_AIR) {
        int textWidth = MeasureText(blockName, 20);
        int tx = screenWidth / 2 - textWidth / 2;
        int ty = bottomY - 30;
        DrawRectangle(tx - 5, ty - 2, textWidth + 10, 24,
                      (Color){0, 0, 0, 150});
        DrawText(blockName, tx, ty, 20, WHITE);
      }

      for (int i = 0; i < 9; i++) {
        int x = startX + i * 45;
        bool isSelected = (i == player.selectedSlot);
        Color slotColor = isSelected ? YELLOW : LIGHTGRAY;

        DrawRectangle(x, bottomY, 40, 40, (Color){0, 0, 0, 100});

        if (isSelected) {
          DrawRectangleLinesEx(
              (Rectangle){(float)x, (float)bottomY, 40.0f, 40.0f}, 4.0f,
              slotColor);
        } else {
          DrawRectangleLines(x, bottomY, 40, 40, slotColor);
        }

        if (player.hotbar[i].count > 0) {
          Texture2D tex = world->GetBlockTexture(player.hotbar[i].type);
          DrawTexturePro(tex,
                         (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                         (Rectangle){(float)x + 4, (float)bottomY + 4, 32, 32},
                         (Vector2){0, 0}, 0.0f, WHITE);

          DrawText(TextFormat("%d", player.hotbar[i].count), x + 22,
                   bottomY + 24, 10, WHITE);
        }
      }

      // Crosshair
      DrawText("+", screenWidth / 2 - 5, screenHeight / 2 - 10, 20, WHITE);
    }
    // Shared FPS
    DrawFPS(10, 10);

    EndDrawing();
  }

  world->Unload();
  delete world;
  CloseWindow();

  return 0;
}
