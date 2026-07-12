# 自走棋模拟

`AutoChess-C` 是一个以学习 C 语言工程能力为核心目标的单机终端自走棋模拟器项目。

这个项目不会一开始追求复杂画面或完整复刻商业游戏，而是先把自动战棋的核心规则拆成清晰、可测试、可扩展的 C 语言模块：英雄、单位、玩家、棋盘、商店、经济、羁绊、战斗、AI、存档和命令系统。

## 项目目标

- 使用 C 语言实现一个结构成熟的小型游戏系统。
- 通过真实项目练习结构体、数组、指针、模块化、状态机、随机数、文件 IO、测试和调试。
- 先完成终端版游戏内核，再考虑 raylib 图形界面。
- 文档和终端展示中文友好，代码命名以英文为主。

## 当前阶段

当前处于 `V1.9 Trait Completion` 阶段，已经支持交互式准备、基础手动站位、运营经济命令、商店锁定、多费用商店概率、扩展英雄池、羁绊展示和完整的第一版羁绊战斗效果。

第一批文档用于确定项目边界、版本路线和协作规则：

- [docs/DESIGN.md](docs/DESIGN.md)：整体设计方案
- [docs/ROADMAP.md](docs/ROADMAP.md)：版本路线
- [docs/RULES.md](docs/RULES.md)：核心游戏规则
- [docs/CHANGELOG.md](docs/CHANGELOG.md)：变更记录

## 近期目标

`V1.0 Terminal Release` 已包含：

- 创建基础 CMake 工程；
- 定义英雄和战斗单位结构；
- 创建双方固定阵容；
- 加入 8x7 棋盘、坐标、攻击距离和简单移动；
- 加入玩家、备战席、部署和上场人数限制；
- 加入金币、回合收入、利息、经验购买和升级；
- 加入商店刷新和购买英雄；
- 加入三合一升星和星级属性倍率；
- 加入英雄职业/阵营，以及基础羁绊属性加成；
- 加入法力值、普攻回蓝和技能释放；
- 加入电脑玩家自动刷新、购买和部署；
- 加入多回合对局、胜负扣血和游戏结束判断；
- 支持命令行帮助、版本号、自定义随机种子、自定义最大回合和静默运行；
- 支持 `--interactive` 进入交互式准备阶段；
- 玩家可以查看商店、购买英雄、刷新商店、查看阵容、自动部署并确认进入战斗；
- 玩家可以手动部署、移动和撤回棋盘上的单位；
- 玩家可以出售单位、购买经验，并查看下回合收入和经验进度；
- 玩家可以锁定商店，保留下回合继续购买；
- 交互模式每回合会展示收入结算和商店刷新/保留状态；
- 玩家可以查看当前等级商店概率和英雄池；
- 英雄池已经扩展到 1-4 费；
- 玩家可以查看当前上场阵容羁绊、已触发档位和下一档差距；
- 守卫、剑士、游侠、法师、城邦、森林、元素、暗影羁绊已经拥有基础战斗效果；
- 将玩家拥有的单位复制为战斗临时单位；
- 实现距离优先目标选择、普通攻击、死亡判断和胜负判断；
- 输出中文战斗日志；
- 添加基础测试。

## 构建和运行

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
./build/autochess
```

在 Windows PowerShell + MinGW 中，可以使用：

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
ctest --test-dir build-mingw
.\build-mingw\autochess.exe
```

查看可用运行参数：

```powershell
.\build-mingw\autochess.exe --help
```

示例：使用固定随机种子，最多运行 10 回合，并只输出最终结果。

```powershell
.\build-mingw\autochess.exe --player-seed 123 --enemy-seed 456 --rounds 10 --quiet
```

进入交互式准备阶段：

```powershell
.\build-mingw\autochess.exe --interactive
```

准备阶段可用命令：

```text
help
status
shop
bench
buy 1
sell 0
buyxp
lock
odds
pool
traits
refresh
deploy 1 6 3
move 6 3 7 3
recall 7 3
auto
ready
quit
```

在 Visual Studio 生成器中，最后一步通常是：

```powershell
.\build\Debug\autochess.exe
```

## 项目边界

当前终端版暂不做：

- 联网对战；
- 八人真实匹配；
- 排位系统；
- 复杂装备系统；
- 复杂强化符文；
- 保存/读取；
- 外部数据文件驱动；
- 商业级平衡性；
- 图形界面。

这些内容可以作为未来扩展，而不是当前主线。
