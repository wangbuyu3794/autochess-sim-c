#include "logger.h"

void logger_battle_start(FILE *stream)
{
    if (stream != NULL)
    {
        fprintf(stream, "自走棋模拟 - V0.1 战斗模拟器\n\n");
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
