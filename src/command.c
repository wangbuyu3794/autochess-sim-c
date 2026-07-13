#include "command.h"

#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "economy.h"
#include "equipment.h"
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
    const char *equipment = 0;

    if (output == 0 || unit == 0 || !unit->is_active)
    {
        return;
    }

    hero = hero_get_template(unit->template_id);
    if (hero == 0)
    {
        return;
    }

    equipment = unit->equipment_id == EQUIPMENT_NONE ? "" : equipment_name(unit->equipment_id);

    if (unit_is_deployed(unit))
    {
        if (unit->equipment_id == EQUIPMENT_NONE)
        {
            fprintf(output,
                    "  #%d %s %d星：棋盘 (%d,%d)\n",
                    index,
                    hero->name,
                    unit->star,
                    unit->position.row,
                    unit->position.col);
        }
        else
        {
            fprintf(output,
                    "  #%d %s %d星：棋盘 (%d,%d)，装备 %s\n",
                    index,
                    hero->name,
                    unit->star,
                    unit->position.row,
                    unit->position.col,
                    equipment);
        }
    }
    else if (unit_is_on_bench(unit))
    {
        if (unit->equipment_id == EQUIPMENT_NONE)
        {
            fprintf(output,
                    "  #%d %s %d星：备战席 %d\n",
                    index,
                    hero->name,
                    unit->star,
                    unit->bench_index + 1);
        }
        else
        {
            fprintf(output,
                    "  #%d %s %d星：备战席 %d，装备 %s\n",
                    index,
                    hero->name,
                    unit->star,
                    unit->bench_index + 1,
                    equipment);
        }
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
    fprintf(output, "  overview   查看当前机制总览\n");
    fprintf(output, "  guide      查看推荐操作流程\n");
    fprintf(output, "  status     查看玩家状态\n");
    fprintf(output, "  shop       查看商店\n");
    fprintf(output, "  odds       查看当前等级商店概率\n");
    fprintf(output, "  pool       查看当前英雄池\n");
    fprintf(output, "  traits     查看当前上场阵容羁绊\n");
    fprintf(output, "  items      查看装备库存\n");
    fprintf(output, "  bench      查看备战席和已上场单位\n");
    fprintf(output, "  buy <1-5>  购买商店中的英雄\n");
    fprintf(output, "  sell <编号>  出售单位，例如 sell 0\n");
    fprintf(output, "  equip <单位编号> <装备编号>  给单位装备，例如 equip 0 1\n");
    fprintf(output, "  unequip <单位编号>  卸下单位装备，例如 unequip 0\n");
    fprintf(output, "  equipfit <单位编号>  查看单位装备适配分，例如 equipfit 0\n");
    fprintf(output, "  buyxp      花费 %d 金币购买 %d 经验\n", AUTOCHESS_BUY_EXP_COST, AUTOCHESS_BUY_EXP_AMOUNT);
    fprintf(output, "  lock       锁定或解锁商店，下回合保留当前商店\n");
    fprintf(output, "  refresh    花费 %d 金币刷新商店\n", AUTOCHESS_REFRESH_COST);
    fprintf(output, "  deploy <备战席> <行> <列>  部署备战席单位，例如 deploy 1 6 3\n");
    fprintf(output, "  move <旧行> <旧列> <新行> <新列>  移动已上场单位\n");
    fprintf(output, "  recall <行> <列>  撤回指定坐标上的单位\n");
    fprintf(output, "  auto       自动部署当前最强单位\n");
    fprintf(output, "  ready      结束准备并进入战斗\n");
    fprintf(output, "  quit       退出本局\n");
}

static void command_print_guide(FILE *output)
{
    if (output == 0)
    {
        return;
    }

    fprintf(output, "推荐流程：\n");
    fprintf(output, "  1. shop 查看商店，odds 查看当前等级概率，pool 查看英雄池。\n");
    fprintf(output, "  2. buy <1-5> 购买英雄，bench/status 查看阵容和经济。\n");
    fprintf(output, "  3. deploy/move/recall 调整站位，traits 查看羁绊。\n");
    fprintf(output, "  4. items 查看装备，equip/equipfit/unequip 管理装备。\n");
    fprintf(output, "  5. buyxp 升级，lock 保留商店，refresh 主动刷新。\n");
    fprintf(output, "  6. auto 自动补位和装备，ready 进入战斗。\n");
    fprintf(output, "终端版目标：当前版本已经完成核心战斗、运营、装备、AI 和交互命令闭环。\n");
}

static int command_count_equipment_total(const Player *player)
{
    size_t count = 0;
    const EquipmentTemplate *templates = equipment_get_templates(&count);
    int total = 0;

    if (player == 0)
    {
        return 0;
    }

    for (size_t i = 0; i < count; ++i)
    {
        total += player_count_equipment(player, templates[i].id);
    }

    return total;
}

void command_print_shop(const GameContext *game, FILE *output)
{
    if (game == 0 || output == 0)
    {
        return;
    }

    fprintf(output, "商店：%s\n", game->player_shop.is_locked ? "已锁定" : "未锁定");
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
                "  %d. %s，费用 %d，生命 %d，攻击 %d，护甲 %d，魔抗 %d，暴击 %d%%，射程 %d，技能 %s\n",
                i + 1,
                hero->name,
                hero->cost,
                hero->base_hp,
                hero->base_attack,
                hero->armor,
                hero->magic_resist,
                hero->crit_chance,
                hero->attack_range,
                skill_name(hero->skill_id));
    }
}

static void command_print_equipment_inventory(const Player *player, FILE *output)
{
    size_t count = 0;
    const EquipmentTemplate *templates = equipment_get_templates(&count);

    if (player == 0 || output == 0)
    {
        return;
    }

    fprintf(output, "装备库存：\n");
    for (size_t i = 0; i < count; ++i)
    {
        const EquipmentTemplate *equipment = &templates[i];
        fprintf(output,
                "  %d. %s x%d",
                equipment->id,
                equipment->name,
                player_count_equipment(player, equipment->id));

        if (equipment->bonus_hp > 0)
        {
            fprintf(output, "，生命 +%d", equipment->bonus_hp);
        }
        if (equipment->bonus_attack > 0)
        {
            fprintf(output, "，攻击 +%d", equipment->bonus_attack);
        }
        if (equipment->bonus_armor > 0)
        {
            fprintf(output, "，护甲 +%d", equipment->bonus_armor);
        }
        if (equipment->bonus_magic_resist > 0)
        {
            fprintf(output, "，魔抗 +%d", equipment->bonus_magic_resist);
        }
        if (equipment->bonus_initial_mana > 0)
        {
            fprintf(output, "，初始法力 +%d", equipment->bonus_initial_mana);
        }
        if (equipment->bonus_crit_chance > 0)
        {
            fprintf(output, "，暴击率 +%d%%", equipment->bonus_crit_chance);
        }
        if (equipment->bonus_crit_damage > 0)
        {
            fprintf(output, "，暴击伤害 +%d%%", equipment->bonus_crit_damage);
        }

        fprintf(output, "\n");
    }
}

static void command_print_equipment_fit(const Player *player, int unit_index, FILE *output)
{
    size_t count = 0;
    const EquipmentTemplate *templates = equipment_get_templates(&count);
    const Unit *unit = 0;
    const HeroTemplate *hero = 0;

    if (player == 0 || output == 0 ||
        unit_index < 0 || unit_index >= player->unit_count ||
        !player->units[unit_index].is_active)
    {
        fprintf(output, "装备适配失败：没有找到单位。\n");
        return;
    }

    unit = &player->units[unit_index];
    hero = hero_get_template(unit->template_id);
    if (hero == 0)
    {
        fprintf(output, "装备适配失败：英雄模板无效。\n");
        return;
    }

    fprintf(output, "#%d %s %d星 装备适配分：\n", unit_index, hero->name, unit->star);
    for (size_t i = 0; i < count; ++i)
    {
        const EquipmentTemplate *equipment = &templates[i];
        fprintf(output,
                "  %d. %s：%d 分，库存 %d\n",
                equipment->id,
                equipment->name,
                ai_score_equipment_for_unit(equipment->id, unit),
                player_count_equipment(player, equipment->id));
    }
}

static void command_print_shop_odds(const GameContext *game, FILE *output)
{
    if (game == 0 || output == 0)
    {
        return;
    }

    fprintf(output, "等级 %d 商店概率：\n", game->player.level);
    fprintf(output, "  1费：%d%%\n", shop_get_cost_probability(game->player.level, 1));
    fprintf(output, "  2费：%d%%\n", shop_get_cost_probability(game->player.level, 2));
    fprintf(output, "  3费：%d%%\n", shop_get_cost_probability(game->player.level, 3));
    fprintf(output, "  4费：%d%%\n", shop_get_cost_probability(game->player.level, 4));
}

static void command_print_hero_pool(FILE *output)
{
    const HeroTemplate *templates = 0;
    size_t count = 0;

    if (output == 0)
    {
        return;
    }

    templates = hero_get_templates(&count);
    fprintf(output, "英雄池：\n");

    for (size_t i = 0; i < count; ++i)
    {
        fprintf(output,
                "  %s：%d费，生命 %d，攻击 %d，护甲 %d，魔抗 %d，暴击 %d%%，爆伤 %d%%，射程 %d，技能 %s\n",
                templates[i].name,
                templates[i].cost,
                templates[i].base_hp,
                templates[i].base_attack,
                templates[i].armor,
                templates[i].magic_resist,
                templates[i].crit_chance,
                templates[i].crit_damage,
                templates[i].attack_range,
                skill_name(templates[i].skill_id));
    }
}

static void command_build_deployed_trait_summary(const Player *player, TraitSummary *summary)
{
    if (player == 0 || summary == 0)
    {
        return;
    }

    trait_summary_init(summary);

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

        trait_summary_add(summary, hero->class_trait);
        trait_summary_add(summary, hero->origin_trait);
    }
}

static void command_print_trait_line(FILE *output, const TraitSummary *summary, TraitId trait)
{
    int count = trait_summary_get_count(summary, trait);
    int current = trait_current_threshold(count);
    int next = trait_next_threshold(count);

    if (output == 0 || summary == 0 || count <= 0)
    {
        return;
    }

    fprintf(output, "  %s：%d 个", trait_name(trait), count);
    if (current > 0)
    {
        fprintf(output, "，已触发 %d", current);
    }
    if (next > 0)
    {
        fprintf(output, "，距离 %d 还差 %d", next, next - count);
    }

    if (trait == TRAIT_GUARDIAN)
    {
        fprintf(output, "，守卫生命加成 +%d", trait_guardian_bonus_hp(summary));
    }
    else if (trait == TRAIT_BLADEMASTER)
    {
        fprintf(output, "，剑士攻击加成 +%d%%", trait_blademaster_attack_percent(summary));
    }
    else if (trait == TRAIT_RANGER)
    {
        fprintf(output, "，游侠射程加成 +%d", trait_ranger_bonus_range(summary));
    }
    else if (trait == TRAIT_MAGE)
    {
        fprintf(output, "，法师初始法力 +%d", trait_mage_bonus_initial_mana(summary));
    }
    else if (trait == TRAIT_CITY)
    {
        fprintf(output, "，城邦初始法力 +%d", trait_city_bonus_initial_mana(summary));
    }
    else if (trait == TRAIT_FOREST)
    {
        fprintf(output, "，森林生命加成 +%d", trait_forest_bonus_hp(summary));
    }
    else if (trait == TRAIT_ELEMENT)
    {
        fprintf(output, "，元素生命加成 +%d", trait_element_bonus_hp(summary));
    }
    else if (trait == TRAIT_SHADOW)
    {
        fprintf(output, "，暗影攻击加成 +%d%%", trait_shadow_attack_percent(summary));
    }
    else
    {
        fprintf(output, "，当前仅统计，暂未开放战斗加成");
    }

    fprintf(output, "\n");
}

static void command_print_traits(const GameContext *game, FILE *output)
{
    TraitSummary summary;
    int printed = 0;

    if (game == 0 || output == 0)
    {
        return;
    }

    command_build_deployed_trait_summary(&game->player, &summary);
    fprintf(output, "当前上场羁绊：\n");

    for (int trait = TRAIT_GUARDIAN; trait <= TRAIT_SHADOW; ++trait)
    {
        int count = trait_summary_get_count(&summary, (TraitId)trait);
        if (count > 0)
        {
            command_print_trait_line(output, &summary, (TraitId)trait);
            printed = 1;
        }
    }

    if (!printed)
    {
        fprintf(output, "  暂无已上场单位。\n");
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

static void command_print_overview(const GameContext *game, FILE *output)
{
    TraitSummary summary;
    int active_traits = 0;
    int next_income = 0;

    if (game == 0 || output == 0)
    {
        return;
    }

    command_build_deployed_trait_summary(&game->player, &summary);
    next_income = economy_calculate_round_income(game->player.gold);

    for (int trait = TRAIT_GUARDIAN; trait <= TRAIT_SHADOW; ++trait)
    {
        if (trait_summary_get_count(&summary, (TraitId)trait) > 0)
        {
            active_traits += 1;
        }
    }

    fprintf(output, "机制总览：\n");
    fprintf(output,
            "  回合：第 %d 回合准备阶段，目标是在战斗前完成购买、部署、装备和经济选择。\n",
            game->current_round + 1);
    fprintf(output,
            "  生命：玩家 %d，敌方 %d；生命归零则失败。\n",
            game->player.health,
            game->enemy.health);
    fprintf(output,
            "  经济：金币 %d，下回合预计收入 %d；可 refresh 刷新、lock 锁店、buyxp 升级。\n",
            game->player.gold,
            next_income);
    fprintf(output,
            "  等级：%d 级，经验 %d/%d；当前最多上场 %d 个单位。\n",
            game->player.level,
            game->player.experience,
            game->player.level >= AUTOCHESS_MAX_LEVEL ? 0 : AUTOCHESS_EXP_PER_LEVEL,
            game->player.level);
    fprintf(output,
            "  商店：%s，5 个槽位；odds 看概率，pool 看英雄池，buy <1-5> 购买。\n",
            game->player_shop.is_locked ? "已锁定" : "未锁定");
    fprintf(output,
            "  阵容：拥有 %d 个单位，已上场 %d/%d；deploy/move/recall 调整站位。\n",
            player_count_active_units(&game->player),
            player_count_deployed_units(&game->player),
            game->player.level);
    fprintf(output,
            "  羁绊：当前有 %d 类上场羁绊；traits 查看触发档位和下一档差距。\n",
            active_traits);
    fprintf(output,
            "  装备：库存 %d 件；items 查看属性，equip/equipfit/unequip 管理装备。\n",
            command_count_equipment_total(&game->player));
    fprintf(output,
            "  战斗：ready 后自动索敌、移动、普攻、释放技能并结算胜负和装备奖励。\n");
    fprintf(output,
            "建议流程：overview -> shop -> buy -> bench -> deploy/auto -> traits/items -> ready。\n");
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

    if (strcmp(command, "guide") == 0)
    {
        command_print_guide(out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "overview") == 0)
    {
        command_print_overview(game, out);
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

    if (strcmp(command, "odds") == 0)
    {
        command_print_shop_odds(game, out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "pool") == 0)
    {
        command_print_hero_pool(out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "traits") == 0)
    {
        command_print_traits(game, out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "bench") == 0 || strcmp(command, "roster") == 0)
    {
        command_print_roster(game, out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "items") == 0)
    {
        command_print_equipment_inventory(&game->player, out);
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

    if (strcmp(command, "lock") == 0)
    {
        shop_set_locked(&game->player_shop, !game->player_shop.is_locked);
        fprintf(out, "商店%s。\n", game->player_shop.is_locked ? "已锁定" : "已解锁");
        command_print_shop(game, out);
        return COMMAND_RESULT_CONTINUE;
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

    if (strcmp(command, "equip") == 0)
    {
        char *equipment_text = strtok(0, " \t");
        int unit_index = -1;
        int equipment_id = 0;
        PlayerResult result = PLAYER_OK;

        if (!command_parse_int(argument, &unit_index) ||
            !command_parse_int(equipment_text, &equipment_id))
        {
            fprintf(out, "装备失败：请输入 equip <单位编号> <装备编号>，例如 equip 0 1。\n");
            return COMMAND_RESULT_ERROR;
        }

        result = player_equip_unit(&game->player, unit_index, (EquipmentId)equipment_id);
        fprintf(out, "装备结果：%s\n", player_result_name(result));
        if (result == PLAYER_OK)
        {
            command_print_roster(game, out);
            command_print_equipment_inventory(&game->player, out);
        }
        return result == PLAYER_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "unequip") == 0)
    {
        int unit_index = -1;
        PlayerResult result = PLAYER_OK;

        if (!command_parse_int(argument, &unit_index))
        {
            fprintf(out, "卸装失败：请输入 unequip <单位编号>，例如 unequip 0。\n");
            return COMMAND_RESULT_ERROR;
        }

        result = player_unequip_unit(&game->player, unit_index);
        fprintf(out, "卸装结果：%s\n", player_result_name(result));
        if (result == PLAYER_OK)
        {
            command_print_roster(game, out);
            command_print_equipment_inventory(&game->player, out);
        }
        return result == PLAYER_OK ? COMMAND_RESULT_CONTINUE : COMMAND_RESULT_ERROR;
    }

    if (strcmp(command, "equipfit") == 0)
    {
        int unit_index = -1;

        if (!command_parse_int(argument, &unit_index))
        {
            fprintf(out, "装备适配失败：请输入 equipfit <单位编号>，例如 equipfit 0。\n");
            return COMMAND_RESULT_ERROR;
        }

        if (unit_index < 0 || unit_index >= game->player.unit_count || !game->player.units[unit_index].is_active)
        {
            fprintf(out, "装备适配失败：没有找到单位。\n");
            return COMMAND_RESULT_ERROR;
        }

        command_print_equipment_fit(&game->player, unit_index, out);
        return COMMAND_RESULT_CONTINUE;
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
        ai_equip_best_units(&game->player);
        fprintf(out, "已自动部署当前可上场单位。\n");
        command_print_roster(game, out);
        return COMMAND_RESULT_CONTINUE;
    }

    if (strcmp(command, "ready") == 0)
    {
        ai_deploy_best_units(&game->player);
        ai_equip_best_units(&game->player);
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
