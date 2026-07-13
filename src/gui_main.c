#include "raylib.h"

#include <stdio.h>

#include "ai.h"
#include "board.h"
#include "config.h"
#include "economy.h"
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
    GUI_BOARD_X = 336,
    GUI_BOARD_Y = 88,
    GUI_BENCH_CARD_WIDTH = 76,
    GUI_SHOP_CARD_WIDTH = 78,
    GUI_CARD_HEIGHT = 74,
    GUI_CARD_GAP = 8,
    GUI_BENCH_X = 24,
    GUI_SHOP_X = 824,
    GUI_BOTTOM_Y = 604
};

typedef enum
{
    GUI_SELECTION_NONE = 0,
    GUI_SELECTION_BENCH,
    GUI_SELECTION_BOARD
} GuiSelectionType;

typedef struct
{
    GuiSelectionType type;
    int bench_index;
    BoardPosition position;
} GuiSelection;

typedef struct
{
    int has_battle;
    int round;
    BattleResult result;
    int player_hp_before;
    int player_hp_after;
    int enemy_hp_before;
    int enemy_hp_after;
    int player_deployed;
    int enemy_deployed;
    EquipmentId reward_equipment;
    BattleSide reward_side;
} GuiBattleSummary;

typedef struct
{
    GuiSelection selection;
    GuiBattleSummary battle;
    char message[160];
} GuiState;

static Rectangle gui_make_rect(int x, int y, int width, int height)
{
    Rectangle rect = {(float)x, (float)y, (float)width, (float)height};
    return rect;
}

static void gui_set_message(GuiState *state, const char *message)
{
    if (state == 0 || message == 0)
    {
        return;
    }

    snprintf(state->message, sizeof(state->message), "%s", message);
}

static void gui_set_message_format(GuiState *state, const char *format, int value)
{
    if (state == 0 || format == 0)
    {
        return;
    }

    snprintf(state->message, sizeof(state->message), format, value);
}

static void gui_clear_selection(GuiState *state)
{
    if (state == 0)
    {
        return;
    }

    state->selection.type = GUI_SELECTION_NONE;
    state->selection.bench_index = -1;
    state->selection.position.row = -1;
    state->selection.position.col = -1;
}

static void gui_init_state(GuiState *state)
{
    if (state == 0)
    {
        return;
    }

    gui_clear_selection(state);
    state->battle.has_battle = 0;
    state->battle.round = 0;
    state->battle.result = BATTLE_RESULT_ONGOING;
    state->battle.player_hp_before = 0;
    state->battle.player_hp_after = 0;
    state->battle.enemy_hp_before = 0;
    state->battle.enemy_hp_after = 0;
    state->battle.player_deployed = 0;
    state->battle.enemy_deployed = 0;
    state->battle.reward_equipment = EQUIPMENT_NONE;
    state->battle.reward_side = BATTLE_SIDE_PLAYER;
    gui_set_message(state, "V2.4 ready: click Battle to record a battle summary.");
}

static const char *gui_shop_result_label(ShopResult result)
{
    switch (result)
    {
    case SHOP_OK:
        return "success";
    case SHOP_ERROR_INVALID_SLOT:
        return "invalid shop slot";
    case SHOP_ERROR_EMPTY_SLOT:
        return "empty shop slot";
    case SHOP_ERROR_NOT_ENOUGH_GOLD:
        return "not enough gold";
    case SHOP_ERROR_BENCH_FULL:
        return "bench full";
    case SHOP_ERROR_NO_TEMPLATE:
        return "missing hero template";
    case SHOP_ERROR_INVALID_ARGUMENT:
    default:
        return "invalid action";
    }
}

static const char *gui_player_result_label(PlayerResult result)
{
    switch (result)
    {
    case PLAYER_OK:
        return "success";
    case PLAYER_ERROR_BENCH_FULL:
        return "bench full";
    case PLAYER_ERROR_INVALID_BENCH_SLOT:
        return "invalid bench slot";
    case PLAYER_ERROR_EMPTY_BENCH_SLOT:
        return "empty bench slot";
    case PLAYER_ERROR_INVALID_DEPLOY_POSITION:
        return "invalid deploy position";
    case PLAYER_ERROR_POSITION_OCCUPIED:
        return "position occupied";
    case PLAYER_ERROR_DEPLOY_LIMIT_REACHED:
        return "deploy limit reached";
    case PLAYER_ERROR_UNIT_NOT_FOUND:
        return "unit not found";
    case PLAYER_ERROR_INVALID_ARGUMENT:
    default:
        return "invalid action";
    }
}

static const char *gui_battle_result_label(BattleResult result)
{
    switch (result)
    {
    case BATTLE_RESULT_PLAYER_WIN:
        return "player win";
    case BATTLE_RESULT_ENEMY_WIN:
        return "enemy win";
    case BATTLE_RESULT_DRAW:
        return "draw";
    case BATTLE_RESULT_ONGOING:
    default:
        return "ongoing";
    }
}

static const char *gui_equipment_label(EquipmentId equipment_id)
{
    switch (equipment_id)
    {
    case EQUIPMENT_BROADSWORD:
        return "Broadsword";
    case EQUIPMENT_GUARDIAN_VEST:
        return "Guardian Vest";
    case EQUIPMENT_MANA_GEM:
        return "Mana Gem";
    case EQUIPMENT_CRIT_GLOVES:
        return "Crit Gloves";
    case EQUIPMENT_NONE:
    default:
        return "None";
    }
}

static const char *gui_side_label(BattleSide side)
{
    return side == BATTLE_SIDE_PLAYER ? "player" : "enemy";
}

static Color gui_side_color(BattleSide side)
{
    return side == BATTLE_SIDE_PLAYER ? (Color){63, 138, 91, 255} : (Color){166, 73, 82, 255};
}

static Rectangle gui_board_cell_rect(BoardPosition position)
{
    return gui_make_rect(GUI_BOARD_X + position.col * GUI_CELL_SIZE,
                         GUI_BOARD_Y + position.row * GUI_CELL_SIZE,
                         GUI_CELL_SIZE,
                         GUI_CELL_SIZE);
}

static Rectangle gui_bench_rect(int bench_index)
{
    return gui_make_rect(GUI_BENCH_X + bench_index * (GUI_BENCH_CARD_WIDTH + GUI_CARD_GAP),
                         GUI_BOTTOM_Y,
                         GUI_BENCH_CARD_WIDTH,
                         GUI_CARD_HEIGHT);
}

static Rectangle gui_shop_rect(int slot_index)
{
    return gui_make_rect(GUI_SHOP_X + slot_index * (GUI_SHOP_CARD_WIDTH + GUI_CARD_GAP),
                         GUI_BOTTOM_Y,
                         GUI_SHOP_CARD_WIDTH,
                         GUI_CARD_HEIGHT);
}

static int gui_hit_board_cell(Vector2 mouse, BoardPosition *position)
{
    Rectangle board = gui_make_rect(GUI_BOARD_X,
                                    GUI_BOARD_Y,
                                    GUI_BOARD_COLS * GUI_CELL_SIZE,
                                    GUI_BOARD_ROWS * GUI_CELL_SIZE);

    if (position == 0 || !CheckCollisionPointRec(mouse, board))
    {
        return 0;
    }

    position->col = (int)(mouse.x - GUI_BOARD_X) / GUI_CELL_SIZE;
    position->row = (int)(mouse.y - GUI_BOARD_Y) / GUI_CELL_SIZE;
    return board_is_position_valid(*position);
}

static int gui_hit_bench(Vector2 mouse)
{
    for (int i = 0; i < AUTOCHESS_BENCH_SIZE; ++i)
    {
        if (CheckCollisionPointRec(mouse, gui_bench_rect(i)))
        {
            return i;
        }
    }

    return -1;
}

static int gui_hit_shop(Vector2 mouse)
{
    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        if (CheckCollisionPointRec(mouse, gui_shop_rect(i)))
        {
            return i;
        }
    }

    return -1;
}

static int gui_get_selected_unit_index(const GameContext *game, const GuiState *state)
{
    if (game == 0 || state == 0)
    {
        return -1;
    }

    if (state->selection.type == GUI_SELECTION_BENCH)
    {
        int bench_index = state->selection.bench_index;
        if (bench_index < 0 || bench_index >= AUTOCHESS_BENCH_SIZE)
        {
            return -1;
        }

        return game->player.bench_slots[bench_index];
    }

    if (state->selection.type == GUI_SELECTION_BOARD)
    {
        return player_find_deployed_unit_index(&game->player, state->selection.position);
    }

    return -1;
}

static const Unit *gui_get_selected_unit(const GameContext *game, const GuiState *state)
{
    int unit_index = gui_get_selected_unit_index(game, state);

    if (game == 0 || unit_index < 0 || unit_index >= game->player.unit_count)
    {
        return 0;
    }

    return &game->player.units[unit_index];
}

static void gui_select_bench(GuiState *state, int bench_index)
{
    if (state == 0)
    {
        return;
    }

    state->selection.type = GUI_SELECTION_BENCH;
    state->selection.bench_index = bench_index;
    state->selection.position.row = -1;
    state->selection.position.col = -1;
    gui_set_message_format(state, "Selected bench slot %d.", bench_index + 1);
}

static void gui_select_board(GuiState *state, BoardPosition position)
{
    if (state == 0)
    {
        return;
    }

    state->selection.type = GUI_SELECTION_BOARD;
    state->selection.bench_index = -1;
    state->selection.position = position;
    gui_set_message(state, "Selected deployed unit.");
}

static void gui_handle_shop_click(GameContext *game, GuiState *state, int slot_index)
{
    ShopResult result = SHOP_OK;

    if (game == 0 || state == 0)
    {
        return;
    }

    result = shop_buy_slot(&game->player_shop, &game->player, slot_index, game->player_next_instance_id);
    if (result == SHOP_OK)
    {
        game->player_next_instance_id += 1;
        gui_clear_selection(state);
        gui_set_message_format(state, "Bought shop slot %d.", slot_index + 1);
        return;
    }

    snprintf(state->message, sizeof(state->message), "Buy failed: %s.", gui_shop_result_label(result));
}

static void gui_handle_bench_click(GameContext *game, GuiState *state, int bench_index)
{
    int unit_index = -1;

    if (game == 0 || state == 0)
    {
        return;
    }

    unit_index = game->player.bench_slots[bench_index];
    if (state->selection.type == GUI_SELECTION_BOARD && unit_index < 0)
    {
        PlayerResult result = player_return_unit_to_bench(&game->player, state->selection.position);
        if (result == PLAYER_OK)
        {
            gui_clear_selection(state);
            gui_set_message(state, "Returned unit to bench.");
            return;
        }

        snprintf(state->message, sizeof(state->message), "Return failed: %s.", gui_player_result_label(result));
        return;
    }

    if (unit_index >= 0)
    {
        gui_select_bench(state, bench_index);
        return;
    }

    gui_clear_selection(state);
    gui_set_message_format(state, "Bench slot %d is empty.", bench_index + 1);
}

static void gui_handle_board_click(GameContext *game, GuiState *state, BoardPosition position)
{
    int player_unit_index = -1;
    int enemy_unit_index = -1;

    if (game == 0 || state == 0)
    {
        return;
    }

    player_unit_index = player_find_deployed_unit_index(&game->player, position);
    if (player_unit_index >= 0)
    {
        gui_select_board(state, position);
        return;
    }

    enemy_unit_index = player_find_deployed_unit_index(&game->enemy, position);
    if (enemy_unit_index >= 0)
    {
        gui_clear_selection(state);
        gui_set_message(state, "Enemy unit preview only.");
        return;
    }

    if (state->selection.type == GUI_SELECTION_BENCH)
    {
        PlayerResult result = player_deploy_from_bench(&game->player, state->selection.bench_index, position);
        if (result == PLAYER_OK)
        {
            gui_clear_selection(state);
            gui_set_message(state, "Deployed selected bench unit.");
            return;
        }

        snprintf(state->message, sizeof(state->message), "Deploy failed: %s.", gui_player_result_label(result));
        return;
    }

    if (state->selection.type == GUI_SELECTION_BOARD)
    {
        BoardPosition from = state->selection.position;
        PlayerResult result = player_move_deployed_unit(&game->player, from, position);
        if (result == PLAYER_OK)
        {
            gui_select_board(state, position);
            gui_set_message(state, "Moved selected unit.");
            return;
        }

        snprintf(state->message, sizeof(state->message), "Move failed: %s.", gui_player_result_label(result));
        return;
    }

    gui_set_message(state, "Select a bench or board unit first.");
}

static void gui_prepare_next_round(GameContext *game)
{
    if (game == 0 || game->result != GAME_RESULT_ONGOING)
    {
        return;
    }

    economy_apply_round_income(&game->player);
    shop_prepare_round(&game->player_shop, game->player.level);
    ai_run_preparation(&game->enemy, &game->enemy_shop, &game->enemy_next_instance_id);
}

static void gui_record_battle_summary(GuiState *state,
                                      const GameContext *game,
                                      BattleResult result,
                                      int player_hp_before,
                                      int enemy_hp_before,
                                      int player_deployed,
                                      int enemy_deployed)
{
    if (state == 0 || game == 0)
    {
        return;
    }

    state->battle.has_battle = 1;
    state->battle.round = game->current_round;
    state->battle.result = result;
    state->battle.player_hp_before = player_hp_before;
    state->battle.player_hp_after = game->player.health;
    state->battle.enemy_hp_before = enemy_hp_before;
    state->battle.enemy_hp_after = game->enemy.health;
    state->battle.player_deployed = player_deployed;
    state->battle.enemy_deployed = enemy_deployed;
    state->battle.reward_equipment = EQUIPMENT_NONE;
    state->battle.reward_side = BATTLE_SIDE_PLAYER;

    if (result == BATTLE_RESULT_PLAYER_WIN)
    {
        state->battle.reward_equipment = game_select_round_equipment_reward(game->current_round);
        state->battle.reward_side = BATTLE_SIDE_PLAYER;
    }
    else if (result == BATTLE_RESULT_ENEMY_WIN)
    {
        state->battle.reward_equipment = game_select_round_equipment_reward(game->current_round);
        state->battle.reward_side = BATTLE_SIDE_ENEMY;
    }
}

static void gui_handle_button_click(GameContext *game, GuiState *state, int button_index)
{
    if (game == 0 || state == 0)
    {
        return;
    }

    if (button_index == 0)
    {
        ShopResult result = shop_refresh_for_player(&game->player_shop, &game->player);
        if (result == SHOP_OK)
        {
            gui_set_message(state, "Shop refreshed for 2 gold.");
        }
        else
        {
            snprintf(state->message, sizeof(state->message), "Refresh failed: %s.", gui_shop_result_label(result));
        }
    }
    else if (button_index == 1)
    {
        shop_set_locked(&game->player_shop, !game->player_shop.is_locked);
        gui_set_message(state, game->player_shop.is_locked ? "Shop locked." : "Shop unlocked.");
    }
    else if (button_index == 2)
    {
        ai_deploy_best_units(&game->player);
        ai_equip_best_units(&game->player);
        gui_clear_selection(state);
        gui_set_message(state, "Auto deployed and equipped best units.");
    }
    else if (button_index == 3)
    {
        BattleResult result = BATTLE_RESULT_DRAW;
        int player_hp_before = 0;
        int enemy_hp_before = 0;
        int player_deployed = 0;
        int enemy_deployed = 0;
        if (game->result != GAME_RESULT_ONGOING)
        {
            gui_set_message(state, "Game is already over.");
            return;
        }

        player_hp_before = game->player.health;
        enemy_hp_before = game->enemy.health;
        player_deployed = player_count_deployed_units(&game->player);
        enemy_deployed = player_count_deployed_units(&game->enemy);
        result = game_run_battle_phase(game, 0);
        gui_record_battle_summary(state, game, result, player_hp_before, enemy_hp_before, player_deployed, enemy_deployed);
        gui_clear_selection(state);
        if (game->result == GAME_RESULT_ONGOING)
        {
            gui_prepare_next_round(game);
        }
        snprintf(state->message, sizeof(state->message), "Round %d battle: %s.", state->battle.round, gui_battle_result_label(result));
    }
}

static Rectangle gui_button_rect(int button_index)
{
    return gui_make_rect(884 + button_index * 88, 536, 78, 34);
}

static int gui_hit_button(Vector2 mouse)
{
    for (int i = 0; i < 4; ++i)
    {
        if (CheckCollisionPointRec(mouse, gui_button_rect(i)))
        {
            return i;
        }
    }

    return -1;
}

static void gui_handle_click(GameContext *game, GuiState *state)
{
    Vector2 mouse = GetMousePosition();
    BoardPosition position = {-1, -1};
    int button_index = gui_hit_button(mouse);
    int shop_index = -1;
    int bench_index = -1;

    if (button_index >= 0)
    {
        gui_handle_button_click(game, state, button_index);
        return;
    }

    shop_index = gui_hit_shop(mouse);
    if (shop_index >= 0)
    {
        gui_handle_shop_click(game, state, shop_index);
        return;
    }

    bench_index = gui_hit_bench(mouse);
    if (bench_index >= 0)
    {
        gui_handle_bench_click(game, state, bench_index);
        return;
    }

    if (gui_hit_board_cell(mouse, &position))
    {
        gui_handle_board_click(game, state, position);
    }
}

static void gui_draw_status_bar(const GameContext *game)
{
    int deployed = game != 0 ? player_count_deployed_units(&game->player) : 0;

    DrawRectangle(0, 0, GUI_SCREEN_WIDTH, 56, (Color){31, 36, 48, 255});
    DrawText(TextFormat("AutoChess-C GUI | Core V%s", AUTOCHESS_VERSION), 24, 16, 20, RAYWHITE);

    if (game == 0)
    {
        return;
    }

    DrawText(TextFormat("Round %d", game->current_round + 1), 450, 17, 18, LIGHTGRAY);
    DrawText(TextFormat("HP %d", game->player.health), 560, 17, 18, LIGHTGRAY);
    DrawText(TextFormat("Enemy %d", game->enemy.health), 650, 17, 18, LIGHTGRAY);
    DrawText(TextFormat("Gold %d", game->player.gold), 780, 17, 18, GOLD);
    DrawText(TextFormat("Lv %d Exp %d", game->player.level, game->player.experience), 880, 17, 18, LIGHTGRAY);
    DrawText(TextFormat("Units %d/%d", deployed, game->player.level), 1020, 17, 18, LIGHTGRAY);
    DrawText(game->player_shop.is_locked ? "Locked" : "Unlocked", 1140, 17, 18, game->player_shop.is_locked ? GOLD : LIGHTGRAY);
}

static void gui_draw_panel(int x, int y, int width, int height, const char *title)
{
    DrawRectangle(x, y, width, height, (Color){245, 247, 250, 255});
    DrawRectangleLines(x, y, width, height, (Color){185, 193, 205, 255});
    DrawText(title, x + 14, y + 12, 18, (Color){35, 42, 55, 255});
}

static int gui_is_selected_board_cell(const GuiState *state, BoardPosition position)
{
    return state != 0 &&
           state->selection.type == GUI_SELECTION_BOARD &&
           board_positions_equal(state->selection.position, position);
}

static void gui_draw_unit_at_board_cell(const Unit *unit, BattleSide side, const GuiState *state)
{
    const HeroTemplate *hero = 0;
    Rectangle cell = {0};
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

    cell = gui_board_cell_rect(unit->position);
    x = (int)cell.x;
    y = (int)cell.y;

    if (side == BATTLE_SIDE_PLAYER && gui_is_selected_board_cell(state, unit->position))
    {
        DrawRectangleLinesEx(cell, 3.0f, GOLD);
    }

    DrawCircle(x + GUI_CELL_SIZE / 2, y + 22, 16, color);
    DrawCircleLines(x + GUI_CELL_SIZE / 2, y + 22, 16, (Color){30, 38, 50, 255});
    DrawText(TextFormat("H%d", hero->id), x + 13, y + 14, 14, RAYWHITE);
    DrawText(TextFormat("%d*", unit->star), x + 18, y + 39, 14, (Color){45, 54, 70, 255});
    if (unit->equipment_id != EQUIPMENT_NONE)
    {
        DrawText("E", x + 39, y + 39, 14, GOLD);
    }
}

static void gui_draw_player_units(const Player *player, const GuiState *state)
{
    if (player == 0)
    {
        return;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        gui_draw_unit_at_board_cell(&player->units[i], player->side, state);
    }
}

static void gui_draw_board(const GameContext *game, const GuiState *state)
{
    for (int row = 0; row < GUI_BOARD_ROWS; ++row)
    {
        for (int col = 0; col < GUI_BOARD_COLS; ++col)
        {
            BoardPosition position = {row, col};
            Rectangle cell = gui_board_cell_rect(position);
            Color fill = row < 4 ? (Color){230, 238, 247, 255} : (Color){232, 243, 235, 255};

            DrawRectangleRec(cell, fill);
            DrawRectangleLinesEx(cell, 1.0f, (Color){125, 137, 150, 255});
        }
    }

    DrawText("Enemy Side", GUI_BOARD_X, GUI_BOARD_Y - 26, 18, (Color){78, 90, 110, 255});
    DrawText("Player Side", GUI_BOARD_X, GUI_BOARD_Y + GUI_CELL_SIZE * 4 + 8, 18, (Color){78, 110, 88, 255});

    if (game != 0)
    {
        gui_draw_player_units(&game->enemy, state);
        gui_draw_player_units(&game->player, state);
    }
}

static void gui_draw_unit_card(Rectangle rect, const Unit *unit, int is_selected)
{
    const HeroTemplate *hero = 0;
    int x = (int)rect.x;
    int y = (int)rect.y;

    DrawRectangleRec(rect, (Color){255, 255, 255, 255});
    DrawRectangleLinesEx(rect, is_selected ? 3.0f : 1.0f, is_selected ? GOLD : (Color){180, 188, 198, 255});

    if (unit == 0 || !unit->is_active)
    {
        DrawText("Empty", x + 8, y + 27, 14, GRAY);
        return;
    }

    hero = hero_get_template(unit->template_id);
    if (hero == 0)
    {
        DrawText("Invalid", x + 8, y + 27, 14, RED);
        return;
    }

    DrawText(TextFormat("H%d", hero->id), x + 8, y + 10, 18, (Color){30, 38, 50, 255});
    DrawText(TextFormat("%d* C%d", unit->star, hero->cost), x + 8, y + 36, 14, (Color){90, 96, 108, 255});
    if (unit->equipment_id != EQUIPMENT_NONE)
    {
        DrawText("Equip", x + 8, y + 56, 12, (Color){170, 120, 20, 255});
    }
}

static void gui_draw_shop_preview(const GameContext *game)
{
    DrawText("Shop", GUI_SHOP_X, GUI_BOTTOM_Y - 26, 18, (Color){35, 42, 55, 255});
    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        Rectangle rect = gui_shop_rect(i);
        int x = (int)rect.x;
        int y = (int)rect.y;

        DrawRectangleRec(rect, (Color){255, 255, 255, 255});
        DrawRectangleLinesEx(rect, 1.0f, (Color){180, 188, 198, 255});

        if (game != 0 && game->player_shop.slots[i].is_available)
        {
            const HeroTemplate *hero = hero_get_template(game->player_shop.slots[i].template_id);
            if (hero != 0)
            {
                DrawText(TextFormat("H%d", hero->id), x + 8, y + 12, 18, (Color){30, 38, 50, 255});
                DrawText(TextFormat("Cost %d", hero->cost), x + 8, y + 40, 14, (Color){90, 96, 108, 255});
            }
        }
        else
        {
            DrawText("Empty", x + 8, y + 27, 14, GRAY);
        }
    }
}

static void gui_draw_bench_preview(const GameContext *game, const GuiState *state)
{
    DrawText("Bench", GUI_BENCH_X, GUI_BOTTOM_Y - 26, 18, (Color){35, 42, 55, 255});
    for (int i = 0; i < AUTOCHESS_BENCH_SIZE; ++i)
    {
        const Unit *unit = 0;
        int is_selected = state != 0 && state->selection.type == GUI_SELECTION_BENCH && state->selection.bench_index == i;

        if (game != 0 && game->player.bench_slots[i] >= 0)
        {
            unit = &game->player.units[game->player.bench_slots[i]];
        }

        gui_draw_unit_card(gui_bench_rect(i), unit, is_selected);
    }
}

static void gui_draw_info_panel(const GameContext *game, const GuiState *state)
{
    int deployed = game != 0 ? player_count_deployed_units(&game->player) : 0;
    int active = game != 0 ? player_count_active_units(&game->player) : 0;

    gui_draw_panel(24, 78, 280, 500, "Info");
    DrawText("V2.4 Battle Summary", 42, 122, 20, (Color){40, 48, 62, 255});
    DrawText("Click shop: buy", 42, 158, 16, (Color){85, 94, 108, 255});
    DrawText("Click bench: select", 42, 184, 16, (Color){85, 94, 108, 255});
    DrawText("Click board: deploy/move", 42, 210, 16, (Color){85, 94, 108, 255});
    DrawText("Battle: record summary", 42, 236, 16, (Color){85, 94, 108, 255});

    DrawText(TextFormat("Player units: %d", active), 42, 292, 16, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Deployed: %d/%d", deployed, game != 0 ? game->player.level : 0), 42, 318, 16, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Selection: %s",
                        state != 0 && state->selection.type == GUI_SELECTION_BENCH ? "bench" :
                        state != 0 && state->selection.type == GUI_SELECTION_BOARD ? "board" : "none"),
             42,
             344,
             16,
             (Color){55, 64, 78, 255});

    DrawText("Legend:", 42, 398, 16, (Color){55, 64, 78, 255});
    DrawText("H# hero id | * star | E equip", 42, 424, 16, (Color){85, 94, 108, 255});
}

static void gui_draw_battle_summary(const GuiState *state, int x, int y)
{
    const GuiBattleSummary *battle = state != 0 ? &state->battle : 0;
    int player_damage = 0;
    int enemy_damage = 0;

    DrawText("Last Battle", x, y, 18, (Color){40, 48, 62, 255});

    if (battle == 0 || !battle->has_battle)
    {
        DrawText("No battle has been played yet.", x, y + 30, 16, (Color){85, 94, 108, 255});
        DrawText("Click Battle after arranging units.", x, y + 56, 16, (Color){85, 94, 108, 255});
        return;
    }

    player_damage = battle->player_hp_before - battle->player_hp_after;
    enemy_damage = battle->enemy_hp_before - battle->enemy_hp_after;

    DrawText(TextFormat("Round %d | %s", battle->round, gui_battle_result_label(battle->result)), x, y + 30, 16, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Player HP %d -> %d  (-%d)", battle->player_hp_before, battle->player_hp_after, player_damage), x, y + 56, 16, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Enemy  HP %d -> %d  (-%d)", battle->enemy_hp_before, battle->enemy_hp_after, enemy_damage), x, y + 82, 16, (Color){55, 64, 78, 255});
    DrawText(TextFormat("Units: player %d / enemy %d", battle->player_deployed, battle->enemy_deployed), x, y + 108, 16, (Color){55, 64, 78, 255});

    if (battle->reward_equipment != EQUIPMENT_NONE)
    {
        DrawText(TextFormat("Reward: %s -> %s", gui_side_label(battle->reward_side), gui_equipment_label(battle->reward_equipment)), x, y + 134, 16, (Color){140, 100, 20, 255});
    }
    else
    {
        DrawText("Reward: none", x, y + 134, 16, (Color){85, 94, 108, 255});
    }
}

static void gui_draw_detail_panel(const GameContext *game, const GuiState *state)
{
    const Unit *unit = gui_get_selected_unit(game, state);
    const HeroTemplate *hero = unit != 0 ? hero_get_template(unit->template_id) : 0;

    gui_draw_panel(790, 78, 460, 500, "Unit / Battle");

    if (unit == 0 || hero == 0)
    {
        DrawText("No player unit selected.", 820, 124, 18, (Color){85, 94, 108, 255});
        DrawText("Select a bench or deployed unit.", 820, 154, 16, (Color){85, 94, 108, 255});
    }
    else
    {
        DrawText(TextFormat("Hero %d", hero->id), 820, 124, 24, (Color){40, 48, 62, 255});
        DrawText(TextFormat("Cost %d | Star %d | Instance %d", hero->cost, unit->star, unit->instance_id), 820, 164, 18, (Color){85, 94, 108, 255});
        DrawText(TextFormat("HP %d  ATK %d", hero->base_hp, hero->base_attack), 820, 204, 18, (Color){55, 64, 78, 255});
        DrawText(TextFormat("Armor %d  MR %d", hero->armor, hero->magic_resist), 820, 234, 18, (Color){55, 64, 78, 255});
        DrawText(TextFormat("Crit %d%% / %d%%", hero->crit_chance, hero->crit_damage), 820, 264, 18, (Color){55, 64, 78, 255});
        DrawText(TextFormat("Mana %d / %d", hero->initial_mana, hero->max_mana), 820, 294, 18, (Color){55, 64, 78, 255});
        DrawText(TextFormat("Equipment: %s", equipment_name(unit->equipment_id)), 820, 334, 18, (Color){140, 100, 20, 255});
        DrawText(TextFormat("Location: %s",
                            unit->location == UNIT_LOCATION_BENCH ? "bench" :
                            unit->location == UNIT_LOCATION_BOARD ? "board" : "none"),
                 820,
                 364,
                 18,
                 (Color){55, 64, 78, 255});
    }

    gui_draw_battle_summary(state, 820, 392);
}

static void gui_draw_buttons(const GameContext *game)
{
    const char *labels[4] = {"Refresh", "Lock", "Auto", "Battle"};

    for (int i = 0; i < 4; ++i)
    {
        Rectangle rect = gui_button_rect(i);
        Color fill = (Color){255, 255, 255, 255};
        const char *label = labels[i];

        if (i == 1 && game != 0 && game->player_shop.is_locked)
        {
            fill = (Color){255, 248, 220, 255};
            label = "Unlock";
        }

        DrawRectangleRec(rect, fill);
        DrawRectangleLinesEx(rect, 1.0f, (Color){160, 170, 184, 255});
        DrawText(label, (int)rect.x + 10, (int)rect.y + 9, 14, (Color){35, 42, 55, 255});
    }
}

static void gui_draw_message_bar(const GuiState *state)
{
    DrawRectangle(0, 690, GUI_SCREEN_WIDTH, 30, (Color){31, 36, 48, 255});
    DrawText(state != 0 ? state->message : "", 24, 697, 14, RAYWHITE);
    DrawText("Close window to exit.", 1100, 697, 14, LIGHTGRAY);
}

int main(void)
{
    GameContext game;
    GuiState state;

    game_init(&game, AUTOCHESS_DEFAULT_PLAYER_SEED, AUTOCHESS_DEFAULT_ENEMY_SEED);
    game_seed_player_demo_units(&game);
    ai_run_preparation(&game.enemy, &game.enemy_shop, &game.enemy_next_instance_id);
    shop_refresh(&game.player_shop, game.player.level);
    gui_init_state(&state);

    InitWindow(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, "AutoChess-C Battle Summary");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            gui_handle_click(&game, &state);
        }

        BeginDrawing();
        ClearBackground((Color){236, 240, 245, 255});

        gui_draw_status_bar(&game);
        gui_draw_info_panel(&game, &state);
        gui_draw_board(&game, &state);
        gui_draw_detail_panel(&game, &state);
        gui_draw_buttons(&game);
        gui_draw_bench_preview(&game, &state);
        gui_draw_shop_preview(&game);
        gui_draw_message_bar(&state);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
