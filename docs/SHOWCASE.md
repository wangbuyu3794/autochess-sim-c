# 自走棋模拟：作品展示说明

## 一句话介绍

`AutoChess-C` 是一个用 C 语言实现的单机自走棋模拟器，用来练习真实项目中的模块拆分、规则建模、状态管理、测试、文档维护和基础 GUI 展示。

## 当前版本

```text
V3.0 Showcase Release
```

终端版第一阶段已经完成，raylib 图形版已经进入可演示阶段。项目可以编译、运行、测试，并展示购买、部署、装备、战斗摘要、战斗日志、单位详情和操作提示。

## 推荐演示顺序

### 1. 先证明项目能构建和测试

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
ctest --test-dir build-mingw
```

预期结果：11 组测试全部通过。

### 2. 快速展示终端自动对局

```powershell
.\build-mingw\autochess.exe --rounds 5 --quiet
```

适合说明：这是同一个规则内核，不依赖 GUI 才能运行。

### 3. 展示终端交互流程

```powershell
.\build-mingw\autochess.exe --interactive
```

进入交互后可以依次尝试：

```text
guide
overview
shop
buy 1
bench
auto
traits
items
equipfit 0
ready
```

适合说明：终端版已经支持机制总览、运营、羁绊、装备和战斗日志。

### 4. 展示图形版

如果 raylib 安装在本项目推荐目录：

```powershell
cmake -S . -B build-gui -G "MinGW Makefiles" -DAUTOCHESS_BUILD_GUI=ON -DCMAKE_PREFIX_PATH=D:\Desktop\File\Project\code\_deps\raylib-install
cmake --build build-gui --target autochess_gui
.\build-gui\autochess_gui.exe
```

图形版推荐演示动作：

```text
点击商店卡牌购买英雄
点击备战席单位
点击己方棋盘空格部署
点击左侧装备查看详情
再次点击装备给选中单位穿戴
点击自动整理阵容
点击战斗查看摘要和日志
再选中单位查看技能、羁绊和装备
```

## 当前核心亮点

- 完整回合流程：准备、购买、布阵、战斗、结算、进入下一回合。
- 自动战斗：单位会索敌、移动、攻击、释放技能，并结算胜负。
- 运营系统：金币、收入、利息、刷新、锁店、买经验、升级。
- 阵容系统：备战席、上场人数限制、三合一升星、羁绊统计和羁绊效果。
- 战斗词条：护甲、魔抗、暴击率、暴击伤害、法力值、技能。
- 状态效果：护盾、灼烧、眩晕。
- 装备系统：装备库存、装备/卸装、装备奖励、自动装备、装备适配分。
- AI 决策：AI 会购买、部署、装备，并使用评分函数做基础选择。
- 图形版：棋盘、备战席、商店、装备详情、单位详情、战斗摘要、战斗日志和下一步提示。
- 测试覆盖：通过 11 组测试保护主要模块行为。

## 代码结构亮点

项目用清晰的模块边界组织代码：

```text
hero      英雄模板
unit      玩家拥有的单位实例
player    玩家、备战席、装备库存和部署规则
board     棋盘坐标、部署区域和距离
shop      商店刷新、购买和概率
economy   金币、收入、经验和升级
trait     羁绊统计和羁绊效果
skill     技能定义和释放
battle    战斗上下文、攻击、移动、状态和胜负
equipment 装备模板
ai        自动购买、部署和装备
command   交互式准备阶段命令
game      多回合对局流程
gui_main  raylib 图形版入口
```

## 可以讲给别人听的学习点

- 如何把游戏规则拆成多个 C 模块；
- 如何区分模板数据、玩家持有实例和战斗临时状态；
- 如何用固定数组管理对象生命周期；
- 如何让 AI 通过已有规则接口行动，而不是绕过规则直接改数据；
- 如何复用终端日志输出，让 GUI 展示战斗过程；
- 如何通过测试避免后续功能破坏旧行为；
- 如何用文档和版本路线防止项目无限膨胀。

## 当前边界

当前展示版暂不做：

- 联网或八人对局；
- 实时战斗动画；
- 复杂装备合成；
- 存档读取；
- 外部数据文件驱动；
- 商业级 UI 和平衡性。

这些内容适合作为后续维护阶段扩展，不作为当前 V3.0 展示版的阻塞项。

## 当前完成状态

`V3.0 Showcase Release` 是当前展示版终点。后续如果继续开发，建议作为维护阶段或新路线处理，而不是继续拉长当前收束版本。
