#include "command.h"

#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "economy.h"
#include "hero.h"

static void command_trim_newline(char *line)
{
    size_t length = 0;

    if (line == 0)
    {
        return;
    }

    length = strlen(line);
    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r'))
    {
        line[length - 1] = '\0';
        length -= 1;
    }

    if ((unsigned char)line[0] == 0xEF &&
        (unsigned char)line[1] == 0xBB &&
        (unsigned char)line[2] == 0xBF)
    {
        memmove(line, line + 3, strlen(line + 3) + 1);
    }
}

static int command_parse_slot(const char *text, int *slot_index)
{
    char *end = 0;
    long parsed = 0;

    if (text == 0 || slot_index == 0)
    {
        return 0;
    }

    parsed = strtol(text, &end, 10);
    if (*text == '\0' || *end != '\0' || parsed < 1 || parsed > AUTOCHESS_SHOP_SIZE)
    {
        return 0;
    }

    *slot_index = (int)parsed - 1;
    return 1;
}

static int command_parse_int(const char *text, int *value)
{
    char *end = 0;
    long parsed = 0;

    if (text == 0 || value == 0)
    {
        return 0;
    }

    parsed = strtol(text, &end, 10);
    if (*text == '\0' || *end != '\0' || parsed < -1000 || parsed > 1000)
    {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static int command_parse_position(const char *row_text, const char *col_text, BoardPosition *position)
{
    int row = 0;
    int col = 0;

    if (position == 0 ||
        !command_parse_int(row_text, &row) ||
        !command_parse_int(col_text, &col))
    {
        return 0;
    }

    position->row = row;
    position->col = col;
    return 1;
}

static void command_print_unit_line(FILE *output, int index, const Unit *unit)
{
    const HeroTemplate *hero = 0;

    if (output == 0 || unit == 0 || !unit->is_active)
    {
        return;
    }

    hero = hero_get_template(unit->template_id);
    if (hero == 0)
    {
        return;
    }

    if (unit_is_deployed(unit))
    {
        fprintf(output,
                "  #%d %s %d星：棋盘 (%d,%d)\n",
                index,
                hero->name,
                unit->star,
                unit->position.row,
                unit->position.col);
    }
    else if (unit_is_on_bench(unit))
    {
        fprintf(output,
                "  #%d %s %d星：备战席 %d\n",
                index,
                hero->name,
                unit->star,
                unit->bench_index + 1);
    }
}

void command_print_help(FILE *output)
{
    if (output == 0)
    {
        return;
    }

    fprintf(output, "准备阶段命令：\n");
    fprintf(output, "  help       查看命令\n");
    fprintf(output, "  status     查看玩家状态\n");
    fprintf(output, "  shop       查看商店\n");
    fprintf(output, "  bench      查看备战席和已上场单位\n");
    fprintf(output, "  buy <1-5>  购买商店中的英雄\n");
    fprintf(output, "  sell <编号>  出售单位，例如 sell 0\n");
    fprintf(output, "  buyxp      花费 %d 金币购买 %d 经验\n", AUTOCHESS_BUY_EXP_COST, AUTOCHESS_BUY_EXP_AMOUNT);
    fprintf(output, "  refresh    花费 %d 金币刷新商店\n", AUTOCHESS_REFRESH_COST);
    fprintf(output, "  deploy <备战席> <行> <列>  部署备战席单位，例如 deploy 1 6 3\n");
    fprintf(output, "  move <旧行> <旧列> <新行> <新列>  移动已上场单位\n");
    fprintf(output, "  recall <行> <列>  撤回指定坐标上的单位\n");
    fprintf(output, "  auto       自动部署当前最强单位\n");
    fprintf(output, "  ready      结束准备并进入战斗\n");
    fprintf(output, "  quit       退出本局\n");
}

void command_print_shop(const GameContext *game, FILE *output)
{
    if (game == 0 || output == 0)
    {
        return;
    }

    fprintf(output, "商店：\n");
    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        const ShopSlot *slot = &game->player_shop.slots[i];
        const HeroTemplate *hero = hero_get_template(slot->template_id);

        if (!slot->is_available || hero == 0)
        {
            fprintf(output, "  %d. 空\n", i + 1);
            continue;
        }

        fprintf(output,
                "  %d. %s，费用 %d，生命 %d，攻击 %d，射程 %d\n",
                i + 1,
                hero->name,
                hero->cost,
                hero->base_hp,
                hero->base_attack,
                hero->attack_range);
    }
}

void command_print_roster(const GameContext *game, FILE *output)
{
    int printed = 0;

    if (game == 0 || output == 0)
    {
        return;
    }

    fprintf(output,
            "玩家：生命 %d，金币 %d，等级 %d，经验 %d/%d，上场 %d/%d，下回合收入 %d\n",
            game->player.health,
            game->player.gold,
            game->player.level,
            game->player.experience,
            game->player.level >= AUTOCHESS_MAX_LEVEL ? 0 : AUTOCHESS_EXP_PER_LEVEL,
            player_count_deployed_units(&game->player),
            game->player.level,
            economy_calculate_round_income(game->player.gold));

    for (int i = 0; i < game->player.unit_count; ++i)
    {
        const Unit *unit = &game->player.units[i];

        if (!unit->is_active)
        {
            continue;
        }

        command_print_unit_line(output, i, unit);
        printed = 1;
    }

    if (!printed)
    {
        fprintf(output, "  暂无单位\n");
    }
}

CommandResult command_execute_preparation(GameContext *game, const char *line, FILE *output)
{
    char buffer[128];
    char *command = 0;
    char *argument = 0;
    FILE *out = output != 0 ? output : stdout;

    if (game == 0 || line == 0)
    {
        return COMMAND_RESULT_ERROR;
    }

    snprintf(buffer, sizeof(buffer), "%s", line);
    command_trim_newline(buffer);

    command = strtok(buffer, " \t");
    argument = strtok(0, " \t");

    if (command == 0)
    {
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0)
    {
        command_print_help(out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "status") == 0)
    {
        command_print_roster(game, out);
        fprintf(out,
                "敌方：生命 %d，金币 %d，等级 %d\n",
                game->enemy.health,
                game->enemy.gold,
                game->enemy.level);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "shop") == 0)
    {
        command_print_shop(game, out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "bench") == 0 || strcmp(command, "roster") == 0)
    {
        command_print_roster(game, out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "refresh") == 0)
    {
        ShopResult result = shop_refresh_for_player(&game->player_shop, &game->player);
        fprintf(out, "刷新商店：%s\n", shop_result_name(result));
        if (result == SHOP_OK)
        {
            command_print_shop(game, out);
        }
        return result == SHOP_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "buy") == 0)
    {
        int slot_index = -1;
        ShopResult result = SHOP_OK;

        if (!command_parse_slot(argument, &slot_index))
        {
            fprintf(out, "购买失败：请输入 buy 1 到 buy 5。\n");
            return COMMAND_RESULT_ERROR;
        }

        result = shop_buy_slot(&game->player_shop, &game->player, slot_index, game->player_next_instance_id);
        fprintf(out, "购买结果：%s\n", shop_result_name(result));
        if (result == SHOP_OK)
        {
            game->player_next_instance_id += 1;
        }
        return result == SHOP_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "sell") == 0)
    {
        int unit_index = -1;
        int refund = 0;
        PlayerResult result = PLAYER_OK;

        if (!command_parse_int(argument, &unit_index))
        {
            fprintf(out, "出售失败：请输入 sell <单位编号>，例如 sell 0。\n");
            return COMMAND_RESULT_ERROR;
        }

        result = player_sell_unit(&game->player, unit_index, &refund);
        if (result == PLAYER_OK)
        {
            fprintf(out, "出售结果：成功，获得 %d 金币。\n", refund);
            command_print_roster(game, out);
        }
        else
        {
            fprintf(out, "出售结果：%s\n", player_result_name(result));
        }
        return result == PLAYER_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "buyxp") == 0)
    {
        int level_before = game->player.level;
        int experience_before = game->player.experience;

        if (!economy_buy_experience(&game->player))
        {
            fprintf(out, "购买经验失败：金币不足。\n");
            return COMMAND_RESULT_ERROR;
        }

        fprintf(out,
                "购买经验成功：等级 %d -> %d，经验 %d -> %d，剩余金币 %d。\n",
                level_before,
                game->player.level,
                experience_before,
                game->player.experience,
                game->player.gold);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "deploy") == 0)
    {
        char *row_text = strtok(0, " \t");
        char *col_text = strtok(0, " \t");
        int bench_index = -1;
        BoardPosition position = {0, 0};
        PlayerResult result = PLAYER_OK;

        if (!command_parse_slot(argument, &bench_index) ||
            !command_parse_position(row_text, col_text, &position))
        {
            fprintf(out, "部署失败：请输入 deploy <备战席1-9> <行> <列>。\n");
            return COMMAND_RESULT_ERROR;
        }

        result = player_deploy_from_bench(&game->player, bench_index, position);
        fprintf(out, "部署结果：%s\n", player_result_name(result));
        if (result == PLAYER_OK)
        {
            command_print_roster(game, out);
        }
        return result == PLAYER_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "move") == 0)
    {
        char *from_col_text = strtok(0, " \t");
        char *to_row_text = strtok(0, " \t");
        char *to_col_text = strtok(0, " \t");
        BoardPosition from = {0, 0};
        BoardPosition to = {0, 0};
        PlayerResult result = PLAYER_OK;

        if (!command_parse_position(argument, from_col_text, &from) ||
            !command_parse_position(to_row_text, to_col_text, &to))
        {
            fprintf(out, "移动失败：请输入 move <旧行> <旧列> <新行> <新列>。\n");
            return COMMAND_RESULT_ERROR;
        }

        result = player_move_deployed_unit(&game->player, from, to);
        fprintf(out, "移动结果：%s\n", player_result_name(result));
        if (result == PLAYER_OK)
        {
            command_print_roster(game, out);
        }
        return result == PLAYER_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "recall") == 0)
    {
        char *col_text = strtok(0, " \t");
        BoardPosition position = {0, 0};
        PlayerResult result = PLAYER_OK;

        if (!command_parse_position(argument, col_text, &position))
        {
            fprintf(out, "撤回失败：请输入 recall <行> <列>。\n");
            return COMMAND_RESULT_ERROR;
        }

        result = player_return_unit_to_bench(&game->player, position);
        fprintf(out, "撤回结果：%s\n", player_result_name(result));
        if (result == PLAYER_OK)
        {
            command_print_roster(game, out);
        }
        return result == PLAYER_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "auto") == 0)
    {
        ai_deploy_best_units(&game->player);
        fprintf(out, "已自动部署当前可上场单位。\n");
        command_print_roster(game, out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "ready") == 0)
    {
        ai_deploy_best_units(&game->player);
        return COMMAND_RESULT_READY;
    }

    if (strcmp(command, "quit") == 0)
    {
        return COMMAND_RESULT_QUIT;
    }

    fprintf(out, "未知命令：%s。输入 help 查看命令。\n", command);
    return COMMAND_RESULT_ERROR;
}

CommandResult command_run_preparation_loop(GameContext *game, FILE *input, FILE *output)
{
    char line[128];

    if (game == 0 || input == 0 || output == 0)
    {
        return COMMAND_RESULT_ERROR;
    }

    command_print_help(output);

    while (1)
    {
        CommandResult result = COMMAND_RESULT_CONTINUE;

        fprintf(output, "\n准备> ");
        if (fgets(line, sizeof(line), input) == 0)
        {
            fprintf(output, "\n输入结束，自动进入战斗。\n");
            return COMMAND_RESULT_READY;
        }

        result = command_execute_preparation(game, line, output);
        if (result == COMMAND_RESULT_READY || result == COMMAND_RESULT_QUIT)
        {
            return result;
        }
    }
}

const char *command_result_name(CommandResult result)
{
    switch (result)
    {
    case COMMAND_RESULT_CONTINUE:
        return "继续准备";
    case COMMAND_RESULT_READY:
        return "准备完成";
    case COMMAND_RESULT_QUIT:
        return "退出";
    case COMMAND_RESULT_ERROR:
    default:
        return "命令错误";
    }
}
