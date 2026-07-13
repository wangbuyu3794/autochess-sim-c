# 自走棋模拟

`AutoChess-C` 是一个以学习 C 语言工程能力为核心目标的单机终端自走棋模拟器项目。

这个项目不会一开始追求复杂画面或完整复刻商业游戏，而是先把自动战棋的核心规则拆成清晰、可测试、可扩展的 C 语言模块：英雄、单位、玩家、棋盘、商店、经济、羁绊、战斗、AI、存档和命令系统。

## 项目目标

- 使用 C 语言实现一个结构成熟的小型游戏系统。
- 通过真实项目练习结构体、数组、指针、模块化、状态机、随机数、文件 IO、测试和调试。
- 先完成终端版游戏内核，再考虑 raylib 图形界面。
- 文档和终端展示中文友好，代码命名以英文为主。

## 当前阶段

当前处于 `V1.30 Terminal Milestone` 阶段，终端版第一阶段已经完成。项目已经支持交互式准备、手动站位、运营经济、商店锁定、多费用商店概率、英雄池、羁绊、战斗属性、表驱动技能、状态效果、装备系统、装备奖励、自动装备、装备适配查看、AI 运营、完整回合循环和中文终端展示。

第一批文档用于确定项目边界、版本路线和协作规则：

- [docs/SHOWCASE.md](docs/SHOWCASE.md)：作品展示说明
- [docs/GUI_DESIGN.md](docs/GUI_DESIGN.md)：V2.0 图形界面设计草案
- [docs/DESIGN.md](docs/DESIGN.md)：整体设计方案
- [docs/ROADMAP.md](docs/ROADMAP.md)：版本路线
- [docs/RULES.md](docs/RULES.md)：核心游戏规则
- [docs/CHANGELOG.md](docs/CHANGELOG.md)：变更记录

## 快速体验

Windows PowerShell + MinGW：

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
ctest --test-dir build-mingw
.\build-mingw\autochess.exe --interactive
```

不进入交互，快速跑一局：

```powershell
.\build-mingw\autochess.exe --rounds 5 --quiet
```

进入交互后，可以先输入：

```text
guide
shop
buy 1
auto
ready
```

## 功能亮点

- 规则闭环：从准备阶段、商店运营、布阵到自动战斗和胜负结算。
- 模块清晰：英雄、单位、玩家、棋盘、商店、经济、羁绊、技能、装备、AI、命令系统分离。
- 数据驱动：技能、装备、英雄模板都尽量用表描述，便于扩展。
- 中文友好：终端命令、战斗日志、文档说明面向中文学习者。
- 可测试：项目包含 11 组基础测试，覆盖战斗、棋盘、玩家、商店、技能、AI、命令等模块。
- 可继续扩展：终端版完成后，可以进入 raylib 图形界面阶段。

## 学习重点

这个项目适合练习：

- C 语言结构体和枚举；
- 固定数组和索引管理；
- 模块化头文件和源文件拆分；
- 命令行参数和字符串解析；
- 游戏状态机；
- 简单 AI 评分函数；
- 单元测试和回归验证；
- 文档化和版本路线管理。

## 项目结构

```text
AutoChess-C/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── SHOWCASE.md
│   ├── DESIGN.md
│   ├── ROADMAP.md
│   ├── RULES.md
│   └── CHANGELOG.md
├── include/
├── src/
└── tests/
```

## 终端版完成范围

`V1.30 Terminal Milestone` 已包含：

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
- 准备阶段提供 `guide` 查看推荐操作流程；
- 玩家可以查看商店、购买英雄、刷新商店、查看阵容、自动部署并确认进入战斗；
- 玩家可以手动部署、移动和撤回棋盘上的单位；
- 玩家可以出售单位、购买经验，并查看下回合收入和经验进度；
- 玩家可以锁定商店，保留下回合继续购买；
- 交互模式每回合会展示收入结算和商店刷新/保留状态；
- 玩家可以查看当前等级商店概率和英雄池；
- 英雄池已经扩展到 1-4 费；
- 玩家可以查看当前上场阵容羁绊、已触发档位和下一档差距；
- 守卫、剑士、游侠、法师、城邦、森林、元素、暗影羁绊已经拥有基础战斗效果；
- 英雄拥有护甲、魔抗、暴击率和暴击伤害，普通攻击和技能会使用基础减伤公式；
- 技能拥有独立定义表，可以描述目标类型、伤害类型、基础数值、攻击系数和治疗量；
- 高费英雄已经拥有更有区分度的技能，例如秘法箭、圣光守护和终结斩；
- 战斗日志会显示暴击、技能伤害和技能治疗结果；
- 战斗单位支持护盾、灼烧和眩晕等基础状态；
- 技能支持最低血友军目标和主目标邻近溅射；
- 玩家拥有基础装备库存，可以查看装备、给单位装备，并在进入战斗时获得属性加成；
- 每回合胜方会获得一件基础装备奖励，用于后续准备阶段调整阵容；
- 自动部署和电脑准备阶段会给已上场单位自动装备库存装备；
- 玩家可以手动卸下单位装备，装备会回到库存；
- 自动装备会参考英雄定位和装备类型，优先做更合适的分配；
- 玩家可以查看指定单位对各装备的适配分，理解自动装备决策；
- 将玩家拥有的单位复制为战斗临时单位；
- 实现距离优先目标选择、普通攻击、死亡判断和胜负判断；
- 输出中文战斗日志；
- 添加基础测试。

## 构建和运行详情

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
guide
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
items
refresh
equip 0 1
unequip 0
equipfit 0
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

## 图形版实验入口

`V2.3` 已提供 raylib 图形版实验入口。默认不构建图形版，避免影响终端版。

当前图形版支持查看棋盘、备战席和商店，并能用鼠标完成基础准备阶段操作：点击商店购买，点击备战席选中，点击己方棋盘部署或移动，点击空备战席撤回，按钮支持刷新、锁店、自动部署/装备和进行一场战斗。

如果按本项目推荐方式把 raylib 安装到 `D:\Desktop\File\Project\code\_deps\raylib-install`，可以使用：

```powershell
cmake -S . -B build-gui -G "MinGW Makefiles" -DAUTOCHESS_BUILD_GUI=ON -DCMAKE_PREFIX_PATH=D:\Desktop\File\Project\code\_deps\raylib-install
cmake --build build-gui
.\build-gui\autochess_gui.exe
```

如果 raylib 安装在其他位置，把 `CMAKE_PREFIX_PATH` 改成你的 raylib 安装目录即可。

如果没有安装 raylib，继续使用普通构建即可：

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
```

## 项目边界

当前终端版暂不做：

- 联网对战；
- 八人真实匹配；
- 排位系统；
- 复杂装备掉落、合成和多装备槽；
- 复杂强化符文；
- 保存/读取；
- 外部数据文件驱动；
- 商业级平衡性；
- 图形界面。

这些内容可以作为未来扩展，而不是当前主线。
