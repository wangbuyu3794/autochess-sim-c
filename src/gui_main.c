#include "raylib.h"

#include "ai.h"
#include "config.h"
#include "equipment.h"
#include "game.h"
#include "hero.h"
#include "shop.h"
#include "unit.h"

enum
{
    GUI_SCREEN_WIDTH = 1280,
    GUI_SCREEN_HEIGHT = 720,
    GUI_BOARD_ROWS = AUTOCHESS_BOARD_ROWS,
    GUI_BOARD_COLS = AUTOCHESS_BOARD_COLS,
    GUI_CELL_SIZE = 58,
    GUI_BOARD_X = 360,
    GUI_BOARD_Y = 88,
    GUI_CARD_WIDTH = 118,
    GUI_CARD_HEIGHT = 76
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

static Color gui_side_color(BattleSide side)
{
    return side == BATTLE_SIDE_PLAYER ? (Color){63, 138, 91, 255} : (Color){166, 73, 82, 255};
}

static void gui_draw_unit_at_board_cell(const Unit *unit, BattleSide side)
{
    const HeroTemplate *hero = 0;
    int x = 0;
    int y = 0;
    Color color = gui_side_color(side);

    if (unit == 0 || !unit_is_deployed(unit))
    {
        return;
    }

    hero = hero_get_template(unit->template_id);
    if (hero == 0)
    {
        return;
    }

    x = GUI_BOARD_X + unit->position.col * GUI_CELL_SIZE;
    y = GUI_BOARD_Y + unit->position.row * GUI_CELL_SIZE;

    DrawCircle(x + GUI_CELL_SIZE / 2, y + 22, 16, color);
    DrawCircleLines(x + GUI_CELL_SIZE / 2, y + 22, 16, (Color){30, 38, 50, 255});
    DrawText(TextFormat("H%d", hero->id), x + 13, y + 14, 14, RAYWHITE);
    DrawText(TextFormat("%d*", unit->star), x + 18, y + 39, 14, (Color){45, 54, 70, 255});
    if (unit->equipment_id != EQUIPMENT_NONE)
    {
        DrawText("E", x + 39, y + 39, 14, GOLD);
    }
}

static void gui_draw_player_units(const Player *player)
{
    if (player == 0)
    {
        return;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        gui_draw_unit_at_board_cell(&player->units[i], player->side);
    }
}

static void gui_draw_board(const GameContext *game)
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

    if (game != 0)
    {
        gui_draw_player_units(&game->enemy);
        gui_draw_player_units(&game->player);
    }
}

static void gui_draw_unit_card(int x, int y, const Unit *unit)
{
    const HeroTemplate *hero = 0;

    DrawRectangle(x, y, GUI_CARD_WIDTH, GUI_CARD_HEIGHT, (Color){255, 255, 255, 255});
    DrawRectangleLines(x, y, GUI_CARD_WIDTH, GUI_CARD_HEIGHT, (Color){180, 188, 198, 255});

    if (unit == 0 || !unit->is_active)
    {
        DrawText("Empty", x + 10, y + 28, 18, GRAY);
        return;
    }

    hero = hero_get_template(unit->template_id);
    if (hero == 0)
    {
        DrawText("Invalid", x + 10, y + 28, 18, RED);
        return;
    }

    DrawText(TextFormat("Hero %d", hero->id), x + 10, y + 12, 18, (Color){30, 38, 50, 255});
    DrawText(TextFormat("%d star | cost %d", unit->star, hero->cost), x + 10, y + 38, 14, (Color){90, 96, 108, 255});
    if (unit->equipment_id != EQUIPMENT_NONE)
    {
        DrawText("Equipped", x + 10, y + 56, 12, (Color){170, 120, 20, 255});
    }
}

static void gui_draw_shop_preview(const GameContext *game)
{
    int start_x = 280;
    int start_y = 610;

    DrawText("Shop Preview", start_x, start_y - 28, 18, (Color){35, 42, 55, 255});
    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        int x = start_x + i * 134;
        DrawRectangle(x, start_y, GUI_CARD_WIDTH, GUI_CARD_HEIGHT, (Color){255, 255, 255, 255});
        DrawRectangleLines(x, start_y, GUI_CARD_WIDTH, GUI_CARD_HEIGHT, (Color){180, 188, 198, 255});

        if (game != 0 && game->player_shop.slots[i].is_available)
        {
            const HeroTemplate *hero = hero_get_template(game->player_shop.slots[i].template_id);
            if (hero != 0)
            {
                DrawText(TextFormat("Hero %d", hero->id), x + 10, start_y + 14, 18, (Color){30, 38, 50, 255});
                DrawText(TextFormat("Cost %d", hero->cost), x + 10, start_y + 42, 16, (Color){90, 96, 108, 255});
            }
        }
        else
        {
            DrawText("Empty", x + 10, start_y + 28, 18, GRAY);
        }
    }
}

static void gui_draw_bench_preview(const GameContext *game)
{
    int start_x = 24;
    int start_y = 610;

    DrawText("Bench Preview", start_x, start_y - 28, 18, (Color){35, 42, 55, 255});
    for (int i = 0; i < 3; ++i)
    {
        const Unit *unit = 0;
        int x = start_x + i * 132;
        if (game != 0 && i < AUTOCHESS_BENCH_SIZE && game->player.bench_slots[i] >= 0)
        {
            unit = &game->player.units[game->player.bench_slots[i]];
        }

        gui_draw_unit_card(x, start_y, unit);
    }
}

static void gui_draw_info_panel(const GameContext *game)
{
    int deployed = game != 0 ? player_count_deployed_units(&game->player) : 0;
    int active = game != 0 ? player_count_active_units(&game->player) : 0;

    gui_draw_panel(24, 78, 280, 500, "Info");
    DrawText("V2.2 Board View", 42, 122, 20, (Color){40, 48, 62, 255});
    DrawText("Read-only GUI preview.", 42, 156, 16, (Color){85, 94, 108, 255});
    DrawText(TextFormat("Player units: %d", active), 42, 198, 16, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Deployed: %d", deployed), 42, 224, 16, (Color){55, 64, 78, 255});
    DrawText("Legend:", 42, 270, 16, (Color){55, 64, 78, 255});
    DrawText("H# = hero template id", 42, 296, 16, (Color){85, 94, 108, 255});
    DrawText("* = star level", 42, 322, 16, (Color){85, 94, 108, 255});
    DrawText("E = equipped", 42, 348, 16, (Color){85, 94, 108, 255});
    DrawText("Next: click interaction.", 42, 410, 16, (Color){85, 94, 108, 255});
}

static void gui_draw_detail_panel(const GameContext *game)
{
    const Unit *unit = 0;
    const HeroTemplate *hero = 0;

    gui_draw_panel(870, 78, 380, 500, "Preview Unit");

    if (game != 0 && game->player.unit_count > 0)
    {
        unit = &game->player.units[0];
        hero = hero_get_template(unit->template_id);
    }

    if (unit == 0 || hero == 0)
    {
        DrawText("No unit available.", 900, 124, 18, (Color){85, 94, 108, 255});
        return;
    }

    DrawText(TextFormat("Hero %d", hero->id), 900, 124, 24, (Color){40, 48, 62, 255});
    DrawText(TextFormat("Cost %d | Star %d", hero->cost, unit->star), 900, 164, 18, (Color){85, 94, 108, 255});
    DrawText(TextFormat("HP %d  ATK %d", hero->base_hp, hero->base_attack), 900, 204, 18, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Armor %d  MR %d", hero->armor, hero->magic_resist), 900, 234, 18, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Crit %d%% / %d%%", hero->crit_chance, hero->crit_damage), 900, 264, 18, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Mana %d / %d", hero->initial_mana, hero->max_mana), 900, 294, 18, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Equipment: %s", equipment_name(unit->equipment_id)), 900, 334, 18, (Color){140, 100, 20, 255});
}

int main(void)
{
    GameContext game;

    game_init(&game, AUTOCHESS_DEFAULT_PLAYER_SEED, AUTOCHESS_DEFAULT_ENEMY_SEED);
    game_seed_player_demo_units(&game);
    ai_run_preparation(&game.enemy, &game.enemy_shop, &game.enemy_next_instance_id);
    shop_refresh(&game.player_shop, game.player.level);

    InitWindow(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, "AutoChess-C Board View");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){236, 240, 245, 255});

        gui_draw_status_bar(&game);
        gui_draw_info_panel(&game);

        gui_draw_board(&game);

        gui_draw_detail_panel(&game);

        gui_draw_bench_preview(&game);
        gui_draw_shop_preview(&game);
        DrawText("Close the window to exit.", 24, 680, 16, (Color){85, 94, 108, 255});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
