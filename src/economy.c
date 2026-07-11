#include "economy.h"

#include "config.h"

int economy_calculate_interest(int gold)
{
    int interest = gold / 10;

    if (interest > AUTOCHESS_MAX_INTEREST)
    {
        interest = AUTOCHESS_MAX_INTEREST;
    }

    if (interest < 0)
    {
        interest = 0;
    }

    return interest;
}

int economy_calculate_round_income(int gold)
{
    return AUTOCHESS_BASE_ROUND_INCOME + economy_calculate_interest(gold);
}

void economy_apply_round_income(Player *player)
{
    if (player == 0)
    {
        return;
    }

    player->gold += economy_calculate_round_income(player->gold);
}

int economy_can_afford(const Player *player, int cost)
{
    return player != 0 && cost >= 0 && player->gold >= cost;
}

int economy_spend_gold(Player *player, int cost)
{
    if (!economy_can_afford(player, cost))
    {
        return 0;
    }

    player->gold -= cost;
    return 1;
}

void economy_add_experience(Player *player, int amount)
{
    if (player == 0 || amount <= 0)
    {
        return;
    }

    player->experience += amount;

    while (player->experience >= AUTOCHESS_EXP_PER_LEVEL && player->level < AUTOCHESS_MAX_LEVEL)
    {
        player->experience -= AUTOCHESS_EXP_PER_LEVEL;
        player->level += 1;
    }

    if (player->level >= AUTOCHESS_MAX_LEVEL)
    {
        player->level = AUTOCHESS_MAX_LEVEL;
        player->experience = 0;
    }
}

int economy_buy_experience(Player *player)
{
    if (!economy_spend_gold(player, AUTOCHESS_BUY_EXP_COST))
    {
        return 0;
    }

    economy_add_experience(player, AUTOCHESS_BUY_EXP_AMOUNT);
    return 1;
}
