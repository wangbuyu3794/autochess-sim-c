#include "logger.h"

void logger_battle_start(FILE *stream)
{
    if (stream != NULL)
    {
        fprintf(stream, "自走棋模拟 - V0.4 商店经济战斗模拟器\n\n");
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
    if (stream != NULL)
    {
        fprintf(stream, "%s 攻击 %s，造成 %d 点伤害，%s 剩余生命值 %d/%d\n",
                attacker,
                target,
                damage,
                target,
                current_hp,
                max_hp);
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
