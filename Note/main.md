作为一个**整合者（Integrator）**，你的工作不仅仅是写代码，更是**制定规则**和**搭建框架**，让队友（美术、UI、动画）的内容能填进去。

针对复现《黑神话：悟空》这种第三人称动作游戏（ARPG），在 UE5 C++ 项目中，你需要优先创建以下 **5 类核心 C++ 基类**。

建议将项目前缀设为 `BMW` (Black Myth Wukong) 或类似的缩写，方便区分。

---

### 1. 核心角色基类 (Base Character)
**类名建议：** `ABMWBaseCharacter` (继承自 `ACharacter`)

这是你和“角色状态机”负责人、以及“地编”（如果是放置怪物的）的交接点。
你需要创建一个**父类**，让玩家（悟空）和敌人（小怪、BOSS）都继承它。

*   **你需要写什么：**
    *   **属性组件（Attribute Component）**：血量、体力（Stamina）、霸体值。
    *   **受击接口（Interface）**：`TakeDamage` 函数的重写。
    *   **死亡逻辑**：当血量 <= 0 时触发的逻辑（播放死亡动画、布娃娃系统）。
*   **为什么需要：** 这样你的战斗逻辑只需要写一次：“如果 `BaseCharacter` 被打中，扣血”。你不需要分别为悟空和老虎写两套扣血逻辑。

---

### 2. 玩家角色类 (Player Character)
**类名建议：** `ABMWPlayerCharacter` (继承自 `ABMWBaseCharacter`)

这是重中之重，是给“角色状态机”负责人的核心容器。

*   **你需要写什么：**
    *   **摄像机组件**：`USpringArmComponent` 和 `UCameraComponent`（第三人称视角）。
    *   **棍势系统（Focus Points）**：这是黑神话的核心机制（积攒豆子），需要一个变量 `int32 CurrentFocusPoint` 和 `float CurrentFocusEnergy`。
    *   **状态枚举（Enums）**：定义 `ECharacterStance`（劈棍、立棍、戳棍），暴露给蓝图，让负责动画的队友在 AnimBP 里读取。
    *   **定身术/变化术接口**：预留好函数槽位，比如 `CastSpell(ESpellType Spell)`。
*   **给队友的接口：** 记得把变量标记为 `UPROPERTY(BlueprintReadWrite)`，这样负责状态机的队友才能在蓝图里根据你的变量切换动画。

---

### 3. 玩家控制器 (Player Controller)
**类名建议：** `ABMWPlayerController` (继承自 `APlayerController`)

这是你处理“输入”的地方，不要把输入写在角色里，写在控制器里更规范（特别是涉及 UI 交互时）。

*   **你需要写什么：**
    *   **Enhanced Input Setup**：配置 UE5 的增强输入系统（移动、攻击、跳跃、闪避、喝葫芦）。
    *   **UI 交互逻辑**：按下 `Esc` 呼出菜单，或者打开背包。
*   **整合作用：** 负责 UI 的队友需要你提供“打开/关闭菜单”的函数。

---

### 4. 属性与战斗组件 (Attribute Component)
**类名建议：** `UBMWAttributeComponent` (继承自 `UActorComponent`)

**强烈建议把属性独立成一个组件**。这是“UI 负责人”最需要的东西。

*   **你需要写什么：**
    *   **数值管理**：`Health`, `MaxHealth`, `Stamina`, `Mana`, `FocusPoints`。
    *   **委托（Delegates/Event Dispatchers）**：这是关键！
        *   定义一个广播：`OnHealthChanged`。
        *   当血量变化时，广播这个事件。
*   **整合作用：**
    *   **UI 队友**：他的血条 UI 不需要每帧去检查（Tick）玩家血量，而是**绑定（Bind）**你的 `OnHealthChanged`。你一扣血，UI 自动更新。这才是高效的协作方式。

---

### 5. 游戏模式 (Game Mode)
**类名建议：** `ABMWGameMode` (继承自 `AGameModeBase`)

这是整个游戏的裁判。

*   **你需要写什么：**
    *   指定 `DefaultPawnClass` 为你创建的 `BMWPlayerCharacter`。
    *   指定 `PlayerControllerClass` 为你创建的 `BMWPlayerController`。
    *   **重生/存读档逻辑**：如果在这个简易Demo里玩家死了，是否在最近的“土地庙”重生？逻辑写在这里。

---

### 给整合者的行动清单 (Action List)

你现在的首要任务是搭建**“数据流”**：

1.  **创建上述 C++ 类**。
2.  **创建蓝图子类**：
    *   基于 `ABMWPlayerCharacter` 创建 `BP_Wukong`。把之前搞好的模型、材质、骨骼选进去。
    *   基于 `ABMWGameMode` 创建 `BP_GameMode`。
3.  **编写 C++ 接口（头文件）**：
    *   先别急着写复杂的实现（.cpp），先把头文件（.h）里的变量定义好。
    *   例如：`GetHealthPercent()`, `GetStaminaPercent()`, `GetFocusPoints()`。
4.  **把蓝图交给队友**：
    *   把 `BP_Wukong` 给负责状态机的队友，让他去写动画蓝图（AnimBP）。
    *   告诉 UI 队友去读 `BMWAttributeComponent` 里的数据。
    *   告诉地编队友把 `BP_GameMode` 设置为关卡的默认模式。

**一句话总结：**
你现在的核心任务是**“暴露变量”**。暴露血量给 UI，暴露状态（攻击中/闪避中）给动画，暴露输入给控制器。做好这些，你们小组的协作就会非常顺畅。