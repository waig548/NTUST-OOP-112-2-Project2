# 112-2 Project 2

Walmart For the King

## Member
waig548

## Content
| Name | Description |
| --- | --- |
| [lib-ftk](./lib-ftx) | The base library of the game |
| [ftk-cli](./ftk-cli) | Abandoned, please ignore it |
| [ftk-gui](./ftk-gui) | A GUI implementation of the game using OpenGL and ImGui |

## Dependencies
- CMake

## Quickstart

```
git clone --recursive https://github.com/waig548/NTUST-OOP-112-2-Project2.git
cd NTUST-OOP-112-2-Project2
cmake -S . -B build
cmake --build build -j 8
```
Note: For MSVC toolchains, the minimum version is VS2022.\
You may also need to run these commands in the `Developer Command Prompt` to build.\
The executables will be in `build/out`.

Or simply clone this repo in Visual Studio, it should recognize the CMake scripts.

## Contribution

### waig548
- lib-ftk
    - library structures
    - base game logics
    - main controls
    - generalized game data handling
- ftk-cli
    - terminal-based user interface
    - abandoned
- ftk-gui
    - graphical user interface

## Score crieteras

### basics

||主分類|子分類|描述|Score|
|---|---|---|---|---|
||UI |||合計8%|
|v||140 * 50 的 Map |2|
|v||正確標記所有的道路、Role、Enemy、商店、不可通行地塊 |2|
|v||所有Role的Stat資訊 |2|
|v||所有Role的裝備資訊 |2|
||Map |||合計12%|
|v||使用者控制的Role可以在地圖上移動，不能移動到地圖外、不可通行地塊 |6|
|v||顯示地圖物件與互動 |6|
||Object |||合計10%|
|v|Shop |商店物品清單: 列出可以購買的物品與價格 |1|
|v|Shop |購買物品 |2|
|v|Enemy |行為清單: 列出可以進行的行為 |2|
|v|Enemy |可以進入戰鬥 |2|
|v|Enemy |撤退，回到上一個地塊 |2|
||Event |隨機事件(自行定義地圖符號與內容，放置於Rect上) |1|
||Combat |||合計20%|
|v||行動順序: 每一個Turn Entity的行動順序正確 |5|
|v||正確執行戰鬥機制: 傷害計算、正確使用專注力和結束戰鬥，Entity死亡後必須正確標記，且跳過死亡Entity's Turn |6|
|v||使用技能與正確選擇對象 (技能要進入冷卻，冷卻會在該Entity TurnStart時-1) |6|
|v||使用道具並且使用後消失 (戰鬥中：Role使用道具後直接進入 TurnEnd 階段) |3|
||Skill |||合計10%|
|v||Attack |5|
|v||Flee |5|
||Items |||合計10%|
|v||Godsbeard |1|
|v||GoldenRoot |1|
|v||TeleportScroll |3|
|||Tent |5

基本分合計 70 分

### advanced

僅限基本分合計 60 分以上，才計算該區域的分數

||主分類 |子分類 |描述 |Score
|---|---|---|---|---|
||Equipment |||共計5%
|v||Entity |Role能穿戴裝備，部位不能重複，並且正確套用裝備效果 |5
||Skill |||共計25%
|v||Active |Heal |3
|v||Active |SpeedUp |2
|v||Active |Provoke |4
|v||Active |Shock-Blast |6
|v||Passive |Hammer-Splash |6
|!||Passive |Run |2
|v||Passive |Destory |2
||Buff/Debuff |||共計15%
|v|||增益與能正確套用與消失 |3
|v||Buff |SpeedUp |2
|v||Debuff |Angry |2
|v||Debuff |Dizziness |4
|v||Debuff |Poisoned |4

進階分合計 45 分

### free-form

||主分類 |子分類 |描述 |Score
|---|---|---|---|---
||Map |||
|v|||戰爭迷霧 |5
||||自行擴充可互動物件 |上限5分
||Skill |||
|~||Active |自行定義主動技能的效果 |上限5分
|~||Passive |自行定義被動技能的效果 |上限5分
|~|Items ||自行定義道具的效果、價格、種類 |上限5分
||Buff/Debuff |||
|~||Buff |自行定義Buff的效果、持續時間 |上限5分
|~||Debuff |自行定義Debuff的效果、持續時間 |上限5分

由助教根據複雜度自行決定分數，但總分不會超過 40 分

### other

其他項目

僅限基本分合計 60 分以上，才計算該區域的分數

||主分類 |子分類 |描述 |Score
|---|---|---|---|---
||Extra |||
|v|||戰爭迷霧 |3
|v|||不能將整張地圖一次畫進整個視窗內(大地圖，需要完成捲動功能) |3
|v|||伏擊，執行3次擲骰判定，可以使用專注。 |2
|v|||戰鬥支援 |1
||||裝備隨機品質 |3
|v|||GUI，QT等UI框架 |4
|v|||讀檔，存檔: 必須能夠隨時保存當前遊戲狀態，必且能夠正確復原 |4
||||網路連線，多人遊戲，需要在不同的電腦或設備上進行遊戲 |4