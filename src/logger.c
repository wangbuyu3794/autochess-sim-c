#include "logger.h"

void logger_battle_start(FILE *stream)
{
    if (stream != NULL)
    {
        fprintf(stream, "战斗开始\n");
    }
}

void logger_skill(FILE *stream, const char *caster, const char *skill_name_text, const char *target)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 释放 %s，目标：%s\n", caster, skill_name_text, target);
    }
}

void logger_round_start(FILE *stream, int round)
{
    if (stream != NULL)
    {
        fprintf(stream, "第 %d 轮开始\n", round);
    }
}

void logger_attack(FILE *stream, const char *attacker, const char *target, int damage, int current_hp, int max_hp)
{
    logger_attack_detail(stream, attacker, target, damage, current_hp, max_hp, 0);
}

void logger_attack_detail(FILE *stream, const char *attacker, const char *target, int damage, int current_hp, int max_hp, int is_critical)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 攻击 %s%s，造成 %d 点伤害，%s 剩余生命值 %d/%d\n",
                attacker,
                target,
                is_critical ? "（暴击）" : "",
                damage,
                target,
                current_hp,
                max_hp);
    }
}

void logger_skill_damage(FILE *stream, const char *skill_name_text, const char *target, int damage, int current_hp, int max_hp)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 造成 %d 点伤害，%s 剩余生命值 %d/%d\n",
                skill_name_text,
                damage,
                target,
                current_hp,
                max_hp);
    }
}

void logger_skill_heal(FILE *stream, const char *skill_name_text, const char *target, int healing, int current_hp, int max_hp)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 治疗 %s %d 点生命，%s 当前生命值 %d/%d\n",
                skill_name_text,
                target,
                healing,
                target,
                current_hp,
                max_hp);
    }
}

void logger_shield_gain(FILE *stream, const char *unit_name, int amount, int shield)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 获得 %d 点护盾，当前护盾 %d\n", unit_name, amount, shield);
    }
}

void logger_shield_absorb(FILE *stream, const char *unit_name, int amount, int shield)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 的护盾吸收 %d 点伤害，剩余护盾 %d\n", unit_name, amount, shield);
    }
}

void logger_burn(FILE *stream, const char *unit_name, int damage, int turns_left, int current_hp, int max_hp)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 受到灼烧 %d 点伤害，剩余 %d 回合，生命值 %d/%d\n",
                unit_name,
                damage,
                turns_left,
                current_hp,
                max_hp);
    }
}

void logger_stunned(FILE *stream, const char *unit_name, int turns_left)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 被眩晕，跳过行动，剩余 %d 回合\n", unit_name, turns_left);
    }
}

void logger_move(FILE *stream, const char *unit_name, int from_row, int from_col, int to_row, int to_col)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 从 (%d, %d) 移动到 (%d, %d)\n",
                unit_name,
                from_row,
                from_col,
                to_row,
                to_col);
    }
}

void logger_wait(FILE *stream, const char *unit_name)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 被阻挡，原地等待\n", unit_name);
    }
}

void logger_defeated(FILE *stream, const char *target)
{
    if (stream != NULL)
    {
        fprintf(stream, "%s 被击败\n", target);
    }
}

void logger_battle_end(FILE *stream, const char *result_name)
{
    if (stream != NULL)
    {
        fprintf(stream, "\n战斗结束：%s\n", result_name);
    }
}
