# menu.h 中文对照参考

> 说明：这是给反作弊分析使用的菜单文本对照文档，不是可编译源码，也不应复制回 `menu.h` 构建运行。

## 顶部与页签

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| CS2 ESP | CS2 透视显示 | 表明主要功能是外部视觉辅助 |
| [ACTIVE] | 已启用 | 总开关状态 |
| [OFF] | 已关闭 | 总开关状态 |
| Main | 主页 | 配置、快捷键、性能、内存后端 |
| ESP | 透视显示 | 玩家信息绘制、骨骼/发光/方框 |
| Radar | 雷达 | 屏外敌人位置显示 |
| Aim | 瞄准 | 自动瞄准和自动开火 |
| Misc | 杂项 | 观战者列表、准星 |
| Nades | 投掷物 | 投掷物点位辅助 |
| Menu | 菜单 | 菜单样式和字体 |

## Main 主页

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| Status | 状态 | 显示总开关是否开启 |
| Active | 已启用 | 总开关开启 |
| Disabled | 已禁用 | 总开关关闭 |
| Key Binds | 快捷键绑定 | 用于检测热键轮询行为 |
| Menu Toggle | 菜单开关 | 默认 F1 |
| Master Toggle | 总开关 | 默认 F2 |
| Exit | 退出 | 默认 Insert |
| Performance | 性能 | 帧率/同步设置 |
| Vsync | 垂直同步 | overlay 渲染行为 |
| Target FPS | 目标帧率 | 可能导致高频渲染 |
| Higher = smoother ESP | 越高 ESP 越平滑 | 表明持续实时绘制 |
| Memory backend | 内存后端 | 关键检测点 |
| WinApi | WinAPI 读内存 | `OpenProcess`/`ReadProcessMemory` 路径 |
| Syscall | 系统调用读内存 | 间接 syscall 路径 |
| Kernel | 内核驱动读内存 | kdmapper/IOCTL 路径 |
| Restart required to apply | 需要重启程序生效 | 后端切换提示 |
| Reset All Settings | 重置所有设置 | 配置恢复 |
| Reset ALL settings to defaults? | 是否把所有设置恢复默认？ | 配置行为 |
| Yes | 是 | 确认 |
| Cancel | 取消 | 取消 |

## ESP 透视显示

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| ESP Enabled | 启用透视显示 | 主视觉辅助开关 |
| ESP Theme Color | ESP 主题颜色 | 视觉配置 |
| Use Theme Color | 使用主题颜色 | 统一视觉样式 |
| Theme Color | 主题颜色 | 颜色选择 |
| Presets | 预设 | 颜色预设 |
| Cyan | 青色 | 颜色预设 |
| Lime | 亮绿 | 颜色预设 |
| Blue | 蓝色 | 颜色预设 |
| Magenta | 洋红 | 颜色预设 |
| White | 白色 | 颜色预设 |
| Red | 红色 | 颜色预设 |
| Chams Style | 人物高亮样式 | 角色模型/骨骼视觉辅助 |
| Filled | 填充 | 填充高亮 |
| Wire | 线框 | 线框高亮 |
| Glow | 发光 | 发光描边 |
| Skeleton | 骨骼 | 骨骼线显示 |
| Draw Head | 绘制头部 | 头部定位 |
| Teammates | 显示队友 | 是否过滤队友 |
| Distance Opacity Drop | 距离透明度衰减 | 远距离视觉处理 |
| Fade Start | 开始淡出距离 | 距离参数 |
| Fade End | 完全淡出距离 | 距离参数 |
| Min Opacity | 最小透明度 | 透明度参数 |
| Box | 方框 | 玩家方框 ESP |
| Corners | 角框 | 方框样式 |
| Full | 完整方框 | 方框样式 |
| Dashed | 虚线方框 | 方框样式 |
| Thickness | 线条粗细 | 绘制参数 |
| Padding X | 横向边距 | 绘制参数 |
| Padding Y | 纵向边距 | 绘制参数 |
| Corner % | 角框比例 | 绘制参数 |
| Health Bar | 血条 | 生命值读取和显示 |
| Solid Color | 固定颜色 | 血条样式 |
| Solid Color (theme override) | 固定颜色（由主题覆盖） | 主题控制 |
| Bar Color | 血条颜色 | 颜色配置 |
| Health Text | 血量文字 | 生命值数值显示 |
| HP Font | 血量字体 | 字体大小 |
| HP Color | 血量颜色 | 颜色配置 |
| HP Shadow | 血量阴影 | 文字样式 |
| Shadow | 阴影 | 文字阴影 |
| Name | 名字 | 玩家名称读取/显示 |
| Top | 顶部 | 名字位置 |
| Bot | 底部 | 名字位置 |
| Name Font | 名字字体 | 字体大小 |
| Offset X | 横向偏移 | 绘制参数 |
| Offset Y | 纵向偏移 | 绘制参数 |
| Weapon | 武器 | 当前武器读取/显示 |
| Show Icon | 显示图标 | 武器图标资源 |
| Show Text | 显示文字 | 武器名称显示 |
| Weapon Font | 武器字体 | 字体大小 |
| Dist. Dropoff | 距离缩放 | 远距离显示缩放 |
| Text Color | 文字颜色 | 颜色配置 |
| Icon Tint | 图标染色 | 颜色配置 |
| Weapon Shadow | 武器文字阴影 | 文字样式 |
| ESP Font | ESP 字体 | 字体枚举/加载 |
| Body Tuning | 身体绘制调节 | 角色形状显示参数 |
| Body Width | 身体宽度 | 绘制参数 |
| Head Size | 头部大小 | 头部定位/显示参数 |
| Depth Scale | 深度缩放 | 透视绘制参数 |
| Glow Outer | 外发光 | 发光参数 |
| Glow Inner | 内发光 | 发光参数 |
| Reset Body | 重置身体参数 | 恢复绘制参数 |
| ESP Colors | ESP 颜色 | 颜色配置 |
| Enemy | 敌人 | 敌我区分 |
| Team | 队友 | 敌我区分 |
| Fill | 填充 | 颜色配置 |
| Outline | 外框 | 颜色配置 |
| Glow | 发光 | 颜色配置 |

## Radar 雷达

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| Show Radar | 显示雷达 | 雷达功能开关 |
| Circle Shape | 圆形雷达 | 雷达形状 |
| Rotate with View | 随视角旋转 | 使用本地视角 |
| Range Rings | 距离环 | 雷达辅助显示 |
| Player Names | 玩家名字 | 雷达显示名字 |
| Names Font | 名字字体 | 字体大小 |
| Size | 大小 | 雷达尺寸 |
| Zoom | 缩放 | 雷达范围 |
| Opacity | 透明度 | 背景透明 |
| Position X | X 位置 | 雷达位置 |
| Position Y | Y 位置 | 雷达位置 |
| Radar Colors | 雷达颜色 | 敌我颜色 |
| Enemy | 敌人 | 敌人颜色 |
| Team | 队友 | 队友颜色 |

## Aim 瞄准

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| Aim assist can get you VACLIVE banned. | 瞄准辅助可能导致 VACLIVE 封禁 | 作者明确知道该功能风险 |
| Enable aimbot | 启用自动瞄准 | 高风险功能 |
| Key | 按键 | 默认瞄准键 |
| Fov | 视野范围 | 自动选目标范围 |
| Smoothing | 平滑 | 鼠标移动平滑度 |
| Head only | 仅瞄头 | 骨骼目标选择 |
| Enable triggerbot | 启用自动开火 | 高风险功能 |
| Trigger key | 自动开火按键 | 触发键 |
| Delay ms | 延迟毫秒 | 自动开火延迟 |
| Head only | 仅打头 | 自动开火目标限制 |

## Misc 杂项

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| Spectator List | 观战者列表 | 读取观察目标 |
| Show Spectators | 显示观战者 | 观战者检测 |
| Spec X | 观战者列表 X 位置 | UI 位置 |
| Spec Y | 观战者列表 Y 位置 | UI 位置 |
| Auto | 自动 | 自动放到默认位置 |
| Crosshair | 准星 | 自定义准星 |
| Enabled | 启用 | 准星开关 |
| Shape | 形状 | 准星样式 |
| Dot | 点 | 准星样式 |
| Color | 颜色 | 准星颜色 |
| Size | 大小 | 准星尺寸 |
| Thick | 粗细 | 准星线宽 |
| Gap | 间隙 | 准星间距 |
| Outline | 描边 | 准星描边 |
| OL Color | 描边颜色 | 准星颜色 |
| OL Width | 描边宽度 | 准星线宽 |
| Center Dot | 中心点 | 准星中心点 |
| Dot Size | 点大小 | 准星中心点大小 |

## Nades 投掷物

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| Grenade Helper | 投掷物辅助 | 投掷点位/瞄点辅助 |
| Enable to configure and use grenade lineups. | 启用后配置并使用投掷物点位 | 功能说明 |
| [VISIBLE] | 可见 | 投掷辅助显示中 |
| [HIDDEN] | 隐藏 | 投掷辅助隐藏 |
| Key Binds | 快捷键绑定 | 热键轮询 |
| Toggle Visible | 切换显示 | 投掷辅助显示热键 |
| Add Spot | 添加点位 | 点位记录 |
| Delete Spot | 删除点位 | 点位管理 |
| Filters | 筛选 | 投掷物类型 |
| Smoke | 烟雾弹 | 类型筛选 |
| Molotov | 燃烧弹 | 类型筛选 |
| Frag | 手雷 | 类型筛选 |
| Flash | 闪光弹 | 类型筛选 |
| Appearance | 外观 | 显示样式 |
| Circle Radius | 圆圈半径 | 点位标记尺寸 |
| Circle Thickness | 圆圈粗细 | 点位标记线宽 |
| Text Size | 文字大小 | 字体大小 |
| Circle | 圆圈 | 普通点位颜色 |
| Active Circle | 激活圆圈 | 当前点位颜色 |
| Aim Line | 瞄准线 | 瞄点辅助线 |
| Text | 文字 | 标签文字 |
| Spots | 点位 | 点位列表 |

## Menu 菜单样式

| 英文 | 中文含义 | 反作弊分析价值 |
| --- | --- | --- |
| Menu Appearance | 菜单外观 | UI 配置 |
| Accent Color | 强调色 | 菜单颜色 |
| Border Color | 边框颜色 | 菜单颜色 |
| BG Opacity | 背景透明度 | overlay 透明度 |
| Presets | 预设 | 主题预设 |
| Cyber Green | 赛博绿 | 主题预设 |
| Blood Red | 血红 | 主题预设 |
| Electric Blue | 电光蓝 | 主题预设 |
| Purple Haze | 紫雾 | 主题预设 |
| Gold | 金色 | 主题预设 |
| Minimal | 极简 | 主题预设 |
| Menu Font | 菜单字体 | 字体枚举/加载 |
| Menu Font Size | 菜单字体大小 | 字体大小 |
| No fonts | 没有字体 | 字体扫描结果 |

## 重点理解

- `ESP`、`Radar`、`Spectator List` 主要依赖对 `cs2.exe` 内存的持续读取。
- `Aimbot`、`Triggerbot` 除了读内存，还会调用鼠标输入注入。
- `Memory backend` 是这套源码最关键的反作弊入口：普通 WinAPI、间接 syscall、内核驱动三条路径分别对应不同检测策略。
- `Discord Overlay` 复用、透明顶层窗口、D3D11/ImGui 渲染、热键轮询可以作为组合行为画像。
