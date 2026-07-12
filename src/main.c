#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "game.h"

typedef struct
{
    unsigned int player_seed;
    unsigned int enemy_seed;
    int max_rounds;
    int quiet;
} CliOptions;

static void print_usage(const char *program_name)
{
    printf("自走棋模拟 V%s\n", AUTOCHESS_VERSION);
    printf("\n");
    printf("用法：%s [选项]\n", program_name);
    printf("\n");
    printf("选项：\n");
    printf("  --help              显示帮助\n");
    printf("  --version           显示版本号\n");
    printf("  --quiet             只运行模拟，不输出战斗日志\n");
    printf("  --rounds <数量>     设置最大回合数，默认 %d\n", AUTOCHESS_MAX_GAME_ROUNDS);
    printf("  --player-seed <值>  设置玩家商店随机种子，默认 %u\n", AUTOCHESS_DEFAULT_PLAYER_SEED);
    printf("  --enemy-seed <值>   设置敌方商店随机种子，默认 %u\n", AUTOCHESS_DEFAULT_ENEMY_SEED);
}

static int parse_positive_int(const char *text, int *value)
{
    char *end = 0;
    long parsed = 0;

    if (text == 0 || value == 0)
    {
        return 0;
    }

    parsed = strtol(text, &end, 10);
    if (*text == '\0' || *end != '\0' || parsed <= 0 || parsed > 1000)
    {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static int parse_seed(const char *text, unsigned int *value)
{
    char *end = 0;
    unsigned long parsed = 0;

    if (text == 0 || value == 0)
    {
        return 0;
    }

    parsed = strtoul(text, &end, 10);
    if (*text == '\0' || *end != '\0')
    {
        return 0;
    }

    *value = (unsigned int)parsed;
    return 1;
}

static int parse_options(int argc, char **argv, CliOptions *options)
{
    if (options == 0)
    {
        return 0;
    }

    options->player_seed = AUTOCHESS_DEFAULT_PLAYER_SEED;
    options->enemy_seed = AUTOCHESS_DEFAULT_ENEMY_SEED;
    options->max_rounds = AUTOCHESS_MAX_GAME_ROUNDS;
    options->quiet = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            print_usage(argv[0]);
            exit(0);
        }
        else if (strcmp(argv[i], "--version") == 0)
        {
            printf("自走棋模拟 V%s\n", AUTOCHESS_VERSION);
            exit(0);
        }
        else if (strcmp(argv[i], "--quiet") == 0)
        {
            options->quiet = 1;
        }
        else if (strcmp(argv[i], "--rounds") == 0)
        {
            if (i + 1 >= argc || !parse_positive_int(argv[++i], &options->max_rounds))
            {
                fprintf(stderr, "错误：--rounds 需要 1 到 1000 之间的整数。\n");
                return 0;
            }
        }
        else if (strcmp(argv[i], "--player-seed") == 0)
        {
            if (i + 1 >= argc || !parse_seed(argv[++i], &options->player_seed))
            {
                fprintf(stderr, "错误：--player-seed 需要非负整数。\n");
                return 0;
            }
        }
        else if (strcmp(argv[i], "--enemy-seed") == 0)
        {
            if (i + 1 >= argc || !parse_seed(argv[++i], &options->enemy_seed))
            {
                fprintf(stderr, "错误：--enemy-seed 需要非负整数。\n");
                return 0;
            }
        }
        else
        {
            fprintf(stderr, "错误：未知选项 %s\n", argv[i]);
            fprintf(stderr, "使用 --help 查看可用选项。\n");
            return 0;
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    GameContext game;
    GameResult result = GAME_RESULT_ONGOING;
    CliOptions options;
    FILE *log_stream = stdout;

    if (!parse_options(argc, argv, &options))
    {
        return 2;
    }

    if (options.quiet)
    {
        log_stream = NULL;
    }

    game_init(&game, options.player_seed, options.enemy_seed);
    game_seed_player_demo_units(&game);
    result = game_run_until_over_with_limit(&game, options.max_rounds, log_stream);

    if (options.quiet)
    {
        printf("游戏结束：%s，回合数 %d，玩家生命 %d，敌方生命 %d\n",
               game_result_name(result),
               game.current_round,
               game.player.health,
               game.enemy.health);
    }

    (void)result;
    return 0;
}
