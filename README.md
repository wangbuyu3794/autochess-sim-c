# 自走棋模拟

`AutoChess-C` 是一个以学习 C 语言工程能力为核心目标的单机终端自走棋模拟器项目。

这个项目不会一开始追求复杂画面或完整复刻商业游戏，而是先把自动战棋的核心规则拆成清晰、可测试、可扩展的 C 语言模块：英雄、单位、玩家、棋盘、商店、经济、羁绊、战斗、AI、存档和命令系统。

## 项目目标

- 使用 C 语言实现一个结构成熟的小型游戏系统。
- 通过真实项目练习结构体、数组、指针、模块化、状态机、随机数、文件 IO、测试和调试。
- 先完成终端版游戏内核，再考虑 raylib 图形界面。
- 文档和终端展示中文友好，代码命名以英文为主。

## 当前阶段

当前处于 `V0.4 Shop And Economy` 阶段。

第一批文档用于确定项目边界、版本路线和协作规则：

- [docs/DESIGN.md](docs/DESIGN.md)：整体设计方案
- [docs/ROADMAP.md](docs/ROADMAP.md)：版本路线
- [docs/RULES.md](docs/RULES.md)：核心游戏规则
- [docs/CHANGELOG.md](docs/CHANGELOG.md)：变更记录

## 近期目标

`V0.4 Shop And Economy` 的目标：

- 创建基础 CMake 工程；
- 定义英雄和战斗单位结构；
- 创建双方固定阵容；
- 加入 8x7 棋盘、坐标、攻击距离和简单移动；
- 加入玩家、备战席、部署和上场人数限制；
- 加入金币、回合收入、利息、经验购买和升级；
- 加入商店刷新和购买英雄；
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

在 Visual Studio 生成器中，最后一步通常是：

```powershell
.\build\Debug\autochess.exe
```

## 项目边界

终端版 `V1.0` 之前暂不做：

- 联网对战；
- 八人真实匹配；
- 排位系统；
- 复杂装备系统；
- 复杂强化符文；
- 商业级平衡性；
- 图形界面。

这些内容可以作为未来扩展，而不是当前主线。
