# 第一版结业验收清单

本文档用于确认 `AutoChess-C V3.0` 第一版已经达到“可展示、可运行、可测试、可继续维护”的结业标准。

## 结业定位

当前版本不是商业级完整自走棋，而是一个以 C 语言工程学习为目标的第一版展示项目。

它已经具备：

- 终端版完整规则闭环；
- 基础 GUI 展示；
- 自动测试；
- GitHub 仓库；
- 设计、路线、规则、展示和变更文档；
- 明确边界，避免无限扩张。

## 必跑验收命令

在项目根目录执行：

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
ctest --test-dir build-mingw
.\build-mingw\autochess.exe --version
.\build-mingw\autochess.exe --rounds 5 --quiet
```

通过标准：

- 构建成功；
- 11 组测试全部通过；
- 版本号显示 `自走棋模拟 V3.0.0`；
- 静默对局可以正常结束。

## 终端版体验验收

运行：

```powershell
.\build-mingw\autochess.exe --interactive
```

推荐输入：

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

通过标准：

- `guide` 能说明推荐流程；
- `overview` 能展示机制总览；
- 可以购买英雄；
- 可以自动部署和装备；
- 可以查看羁绊、装备和适配分；
- `ready` 后能进入战斗并输出日志。

## GUI 版体验验收

如果 raylib 安装在推荐目录，运行：

```powershell
cmake -S . -B build-gui -G "MinGW Makefiles" -DAUTOCHESS_BUILD_GUI=ON -DCMAKE_PREFIX_PATH=D:\Desktop\File\Project\code\_deps\raylib-install
cmake --build build-gui --target autochess_gui
.\build-gui\autochess_gui.exe
```

推荐检查：

- 左侧能看到系统总览；
- 棋盘、备战席和商店显示中文英雄名；
- 点击商店能购买；
- 点击备战席能选中单位；
- 点击己方棋盘能部署；
- 点击装备能查看详情；
- 选中单位后点击装备能穿戴；
- 点击自动能整理阵容；
- 点击战斗能看到摘要和日志；
- 选中单位能看到技能、羁绊和装备。

## 当前边界

第一版结业不要求：

- 实时战斗动画；
- 八人对局；
- 联网；
- 存档；
- 装备合成；
- 外部数据文件；
- 商业级 UI；
- 商业级平衡性。

这些内容如果后续继续做，应进入新的维护阶段。

## 结业结论

如果以上验收项通过，当前项目可以视为：

```text
AutoChess-C V3.0 第一版结业展示版
```

后续优化建议只围绕界面细节、文档清晰度和小范围稳定性修补展开，不再扩大第一版范围。
