# 后台任务管理

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [短时任务](#section1312121216216)
    -   [接口说明](#section114564657874)
    -   [使用说明](#section129654513264)
        -   [短时任务使用约束](#section1551164914237)

-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

在资源调度子系统中后台任务管理负责管理后台任务，并提供后台任务的申请、取消和查询等接口。

## 目录<a name="section161941989596"></a>

```
/foundation/resourceschedule/background_task_mgr
├── frameworks       # 接口实现
├── interfaces
│   ├── innerkits    # 对内接口目录
│   └── kits         # 对外接口目录
├── sa_profile       # 组件服务配置
├── services         # 组件服务实现
└── utils            # 组件工具实现

```
## 短时任务<a name="section1312121216216"></a>

### 接口说明<a name="section114564657874"></a>

| 接口名                                                                                   | 接口描述     |
|------------------------------------------------------------------------------------------|-------------|
| function requestSuspendDelay(reason:string, callback:Callback\<void>): DelaySuspendInfo; | 申请延迟挂起 |
| function cancelSuspendDelay(requestId:number): void;                                     | 取消延迟挂起 |
| function getRemainingDelayTime(requestId:number, callback:AsyncCallback\<number>):void;  | 获取延迟挂起剩余时间（callback形式） |
| function getRemainingDelayTime(requestId:number): Promise\<number>;                      | 获取延迟挂起剩余时间（Promise形式） |

### 使用说明<a name="section129654513264"></a>

退到后台的应用有不可中断且短时间能完成的任务时，可以使用短时任务机制，该机制允许应用在后台短时间内完成任务，保障应用业务运行不受后台生命周期管理的影响。

- 注意：短时任务仅针对应用的临时任务提供资源使用生命周期保障，限制单次最大使用时长为3分钟，全天使用配额默认为10分钟（具体时长系统根据应用场景和系统状态智能调整）。

#### 短时任务使用约束<a name="section1551164914237"></a>

短时任务的使用需要遵从如下约束和规则：

- **申请时机**：允许应用在前台时，或退后台在被挂起之前（应用退到后台默认有6~12秒的运行时长，具体时长由系统根据具体场景决定）申请延迟挂起，否则可能被挂起（Suspend），导致申请失败。
- **超时**：延迟挂起超时（Timeout），系统通过回调知会应用，应用需要取消对应的延迟挂起，或再次申请延迟挂起。超期不取消或不处理，该应用会被强制取消延迟挂起。
- **取消时机**：任务完成后申请方应用主动取消延时申请，不要等到超时后被系统取消，否则会影响该应用的后台允许运行时长配额。
- **配额机制**：为了防止应用滥用保活，或者申请后不取消，每个应用每天都会有一定配额（会根据用户的使用习惯动态调整），配额消耗完就不再允许申请短时任务，所以应用完成短时任务后立刻取消延时申请，避免消耗配额。（注，这个配额指的是申请的时长，系统默认应用在后台运行的时间不计算在内）。

## 相关仓<a name="section1371113476307"></a>

资源调度子系统

**background\_task\_mgr**

notification_ces_standard

appexecfwk_standard
