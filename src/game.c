#include "game.h"

#include "ai.h"
#include "economy.h"
#include "hero.h"
#include "unit.h"

static void game_log_player_summary(FILE *log_stream, const char *label, const Player *player)
{
    if (log_stream == 0 || label == 0 || player == 0)
    {
        return;
    }

    fprintf(log_stream,
            "%s：生命 %d，金币 %d，等级 %d，经验 %d，已上场 %d/%d，拥有单位 %d\n",
            label,
            player->health,
            player->gold,
            player->level,
            player->experience,
            player_count_deployed_units(player),
            player->level,
            player_count_active_units(player));
}

static void game_log_deployed_units(FILE *log_stream, const char *label, const Player *player)
{
    int printed = 0;

    if (log_stream == 0 || label == 0 || player == 0)
    {
        return;
    }

    fprintf(log_stream, "%s阵容：", label);

    for (int i = 0; i < player->unit_count; ++i)
    {
        const Unit *unit = &player->units[i];
        const HeroTemplate *hero = 0;

        if (!unit->is_active || !unit_is_deployed(unit))
        {
            continue;
        }

        hero = hero_get_template(unit->template_id);
        if (hero == 0)
        {
            continue;
        }

        fprintf(log_stream,
                "%s%s %d星(%d,%d)",
                printed ? "，" : "",
                hero->name,
                unit->star,
                unit->position.row,
                unit->position.col);
        printed = 1;
    }

    if (!printed)
    {
        fprintf(log_stream, "无");
    }

    fprintf(log_stream, "\n");
}

static void game_prepare_context(GameContext *game, BattleContext *battle_context)
{
    if (game == 0 || battle_context == 0)
    {
        return;
    }

    *battle_context = (BattleContext){0};
    player_add_deployed_units_to_battle(&game->player, battle_context);
    player_add_deployed_units_to_battle(&game->enemy, battle_context);
    battle_apply_trait_summary(battle_context);
}

void game_init(GameContext *game, unsigned int player_seed, unsigned int enemy_seed)
{
    if (game == 0)
    {
        return;
    }

    player_init(&game->player, 1, BATTLE_SIDE_PLAYER);
    player_init(&game->enemy, 2, BATTLE_SIDE_ENEMY);
    shop_init(&game->player_shop, player_seed);
    shop_init(&game->enemy_shop, enemy_seed);
    game->player_next_instance_id = 1;
    game->enemy_next_instance_id = 101;
    game->current_round = 0;
    game->result = GAME_RESULT_ONGOING;
}

void game_seed_player_demo_units(GameContext *game)
{
    BoardPosition player_front = {6, 3};
    BoardPosition player_left = {7, 2};
    BoardPosition player_back = {7, 4};

    if (game == 0)
    {
        return;
    }

    economy_apply_round_income(&game->player);
    shop_refresh(&game->player_shop, game->player.level);
    shop_buy_slot(&game->player_shop, &game->player, 0, game->player_next_instance_id++);
    shop_buy_slot(&game->player_shop, &game->player, 1, game->player_next_instance_id++);
    shop_buy_slot(&game->player_shop, &game->player, 2, game->player_next_instance_id++);
    player_deploy_from_bench(&game->player, 0, player_front);
    player_deploy_from_bench(&game->player, 1, player_left);
    player_deploy_from_bench(&game->player, 2, player_back);
}

int game_calculate_damage(const BattleContext *battle_context, BattleSide winner)
{
    int alive_count = 0;

    if (battle_context == 0)
    {
        return 0;
    }

    for (int i = 0; i < battle_context->unit_count; ++i)
    {
        const BattleUnit *unit = &battle_context->units[i];

        if (unit->is_alive && unit->side == winner)
        {
            alive_count += 1;
        }
    }

    return AUTOCHESS_BASE_PLAYER_DAMAGE + alive_count;
}

GameResult game_settle_battle(GameContext *game, const BattleContext *battle_context, BattleResult battle_result)
{
    int damage = 0;

    if (game == 0)
    {
        return GAME_RESULT_DRAW;
    }

    if (battle_result == BATTLE_RESULT_PLAYER_WIN)
    {
        damage = game_calculate_damage(battle_context, BATTLE_SIDE_PLAYER);
        game->enemy.health -= damage;
    }
    else if (battle_result == BATTLE_RESULT_ENEMY_WIN)
    {
        damage = game_calculate_damage(battle_context, BATTLE_SIDE_ENEMY);
        game->player.health -= damage;
    }

    if (game->player.health <= 0 && game->enemy.health <= 0)
    {
        game->result = GAME_RESULT_DRAW;
    }
    else if (game->enemy.health <= 0)
    {
        game->result = GAME_RESULT_PLAYER_WIN;
    }
    else if (game->player.health <= 0)
    {
        game->result = GAME_RESULT_ENEMY_WIN;
    }
    else
    {
        game->result = GAME_RESULT_ONGOING;
    }

    return game->result;
}

BattleResult game_run_round(GameContext *game, FILE *log_stream)
{
    BattleContext battle_context = {0};
    BattleResult battle_result = BATTLE_RESULT_DRAW;

    if (game == 0 || game->result != GAME_RESULT_ONGOING)
    {
        return BATTLE_RESULT_DRAW;
    }

    game->current_round += 1;
    ai_run_preparation(&game->player, &game->player_shop, &game->player_next_instance_id);
    ai_run_preparation(&game->enemy, &game->enemy_shop, &game->enemy_next_instance_id);
    game_prepare_context(game, &battle_context);

    if (log_stream != 0)
    {
        fprintf(log_stream, "\n===== 第 %d 回合 =====\n", game->current_round);
        game_log_player_summary(log_stream, "玩家", &game->player);
        game_log_player_summary(log_stream, "敌方", &game->enemy);
        game_log_deployed_units(log_stream, "玩家", &game->player);
        game_log_deployed_units(log_stream, "敌方", &game->enemy);
    }

    battle_result = battle_run(&battle_context, log_stream);
    game_settle_battle(game, &battle_context, battle_result);

    if (log_stream != 0)
    {
        fprintf(log_stream, "回合结算：玩家生命 %d，敌方生命 %d\n", game->player.health, game->enemy.health);
    }

    return battle_result;
}

GameResult game_run_until_over(GameContext *game, FILE *log_stream)
{
    return game_run_until_over_with_limit(game, AUTOCHESS_MAX_GAME_ROUNDS, log_stream);
}

GameResult game_run_until_over_with_limit(GameContext *game, int max_rounds, FILE *log_stream)
{
    if (game == 0)
    {
        return GAME_RESULT_DRAW;
    }

    if (max_rounds <= 0)
    {
        max_rounds = AUTOCHESS_MAX_GAME_ROUNDS;
    }

    if (log_stream != 0)
    {
        fprintf(log_stream, "自走棋模拟 V%s\n", AUTOCHESS_VERSION);
        fprintf(log_stream, "模式：单机玩家 VS 电脑 AI，最大回合数 %d\n", max_rounds);
    }

    while (game->result == GAME_RESULT_ONGOING && game->current_round < max_rounds)
    {
        game_run_round(game, log_stream);
    }

    if (game->result == GAME_RESULT_ONGOING)
    {
        game->result = GAME_RESULT_DRAW;
    }

    if (log_stream != 0)
    {
        fprintf(log_stream, "游戏结束：%s\n", game_result_name(game->result));
    }

    return game->result;
}

const char *game_result_name(GameResult result)
{
    switch (result)
    {
    case GAME_RESULT_PLAYER_WIN:
        return "玩家获胜";
    case GAME_RESULT_ENEMY_WIN:
        return "敌方获胜";
    case GAME_RESULT_DRAW:
        return "平局";
    case GAME_RESULT_ONGOING:
    default:
        return "游戏进行中";
    }
}
