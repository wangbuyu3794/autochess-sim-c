#include "raylib.h"

#include "config.h"
#include "game.h"
#include "hero.h"

enum
{
    GUI_SCREEN_WIDTH = 1280,
    GUI_SCREEN_HEIGHT = 720,
    GUI_BOARD_ROWS = AUTOCHESS_BOARD_ROWS,
    GUI_BOARD_COLS = AUTOCHESS_BOARD_COLS,
    GUI_CELL_SIZE = 58,
    GUI_BOARD_X = 360,
    GUI_BOARD_Y = 88
};

static void gui_draw_status_bar(const GameContext *game)
{
    DrawRectangle(0, 0, GUI_SCREEN_WIDTH, 56, (Color){31, 36, 48, 255});
    DrawText(TextFormat("AutoChess-C GUI Skeleton | Core V%s", AUTOCHESS_VERSION), 24, 16, 20, RAYWHITE);

    if (game == 0)
    {
        return;
    }

    DrawText(TextFormat("Round %d", game->current_round + 1), 520, 17, 18, LIGHTGRAY);
    DrawText(TextFormat("Player HP %d", game->player.health), 640, 17, 18, LIGHTGRAY);
    DrawText(TextFormat("Enemy HP %d", game->enemy.health), 790, 17, 18, LIGHTGRAY);
    DrawText(TextFormat("Gold %d", game->player.gold), 940, 17, 18, GOLD);
    DrawText(TextFormat("Level %d", game->player.level), 1040, 17, 18, LIGHTGRAY);
}

static void gui_draw_panel(int x, int y, int width, int height, const char *title)
{
    DrawRectangle(x, y, width, height, (Color){245, 247, 250, 255});
    DrawRectangleLines(x, y, width, height, (Color){185, 193, 205, 255});
    DrawText(title, x + 14, y + 12, 18, (Color){35, 42, 55, 255});
}

static void gui_draw_board(void)
{
    for (int row = 0; row < GUI_BOARD_ROWS; ++row)
    {
        for (int col = 0; col < GUI_BOARD_COLS; ++col)
        {
            int x = GUI_BOARD_X + col * GUI_CELL_SIZE;
            int y = GUI_BOARD_Y + row * GUI_CELL_SIZE;
            Color fill = row < 4 ? (Color){230, 238, 247, 255} : (Color){232, 243, 235, 255};

            DrawRectangle(x, y, GUI_CELL_SIZE, GUI_CELL_SIZE, fill);
            DrawRectangleLines(x, y, GUI_CELL_SIZE, GUI_CELL_SIZE, (Color){125, 137, 150, 255});
        }
    }

    DrawText("Enemy Side", GUI_BOARD_X, GUI_BOARD_Y - 26, 18, (Color){78, 90, 110, 255});
    DrawText("Player Side", GUI_BOARD_X, GUI_BOARD_Y + GUI_CELL_SIZE * 4 + 8, 18, (Color){78, 110, 88, 255});
}

static void gui_draw_shop_preview(const GameContext *game)
{
    int start_x = 280;
    int start_y = 610;

    DrawText("Shop Preview", start_x, start_y - 28, 18, (Color){35, 42, 55, 255});
    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        int x = start_x + i * 134;
        DrawRectangle(x, start_y, 118, 76, (Color){255, 255, 255, 255});
        DrawRectangleLines(x, start_y, 118, 76, (Color){180, 188, 198, 255});

        if (game != 0 && game->player_shop.slots[i].is_available)
        {
            const HeroTemplate *hero = hero_get_template(game->player_shop.slots[i].template_id);
            if (hero != 0)
            {
                DrawText(hero->name, x + 10, start_y + 14, 18, (Color){30, 38, 50, 255});
                DrawText(TextFormat("Cost %d", hero->cost), x + 10, start_y + 42, 16, (Color){90, 96, 108, 255});
            }
        }
        else
        {
            DrawText("Empty", x + 10, start_y + 28, 18, GRAY);
        }
    }
}

int main(void)
{
    GameContext game;

    game_init(&game, AUTOCHESS_DEFAULT_PLAYER_SEED, AUTOCHESS_DEFAULT_ENEMY_SEED);
    shop_refresh(&game.player_shop, game.player.level);

    InitWindow(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, "AutoChess-C GUI Skeleton");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){236, 240, 245, 255});

        gui_draw_status_bar(&game);
        gui_draw_panel(24, 78, 280, 500, "Info");
        DrawText("V2.1 GUI Skeleton", 42, 122, 20, (Color){40, 48, 62, 255});
        DrawText("Terminal core remains unchanged.", 42, 156, 16, (Color){85, 94, 108, 255});
        DrawText("Next: draw real units and bench.", 42, 184, 16, (Color){85, 94, 108, 255});

        gui_draw_board();

        gui_draw_panel(870, 78, 380, 500, "Selected Unit");
        DrawText("No unit selected.", 900, 124, 18, (Color){85, 94, 108, 255});
        DrawText("This panel will show stats,", 900, 160, 16, (Color){85, 94, 108, 255});
        DrawText("traits, skill and equipment.", 900, 184, 16, (Color){85, 94, 108, 255});

        gui_draw_shop_preview(&game);
        DrawText("Close the window to exit.", 24, 680, 16, (Color){85, 94, 108, 255});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
