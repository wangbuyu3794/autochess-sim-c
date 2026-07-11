#include <stdio.h>

#include "battle.h"

int main(void)
{
    BattleContext context = battle_create_demo_context();
    BattleResult result = battle_run(&context, stdout);

    return result == BATTLE_RESULT_DRAW ? 1 : 0;
}
