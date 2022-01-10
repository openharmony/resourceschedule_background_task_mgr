# Background Task Manager

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Transient Tasks](#section1312121216216)
    -   [Available APIs](#section114564657874)
    -   [Usage Guidelines](#section129654513264)
        -   [Restrictions on Using Transient Tasks](#section1551164914237)

-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

In the resource scheduling subsystem, the background task management is responsible for managing background tasks, and provides interfaces for application, cancellation and query of background tasks.

## Directory Structure<a name="section161941989596"></a>

```

├── frameworks   # Frameworks
├── interfaces
│   ├── innerkits    # Internal APIs
│   └── kits         # External APIs
├── sa_profile   # SA profile
├── services     # Services
└── utils        # Utilities

```
## Transient Tasks<a name="section1312121216216"></a>

### Available APIs<a name="section114564657874"></a>

API                                                      |     Description                         
---------------------------------------------------------|-----------------------------------------
function requestSuspendDelay(reason:string, callback:Callback\<void>): DelaySuspendInfo; | Request suspend delay 
function cancelSuspendDelay(requestId:number): void;        | Cancel suspend delay 
function getRemainingDelayTime(requestId:number, callback:AsyncCallback\<number>):void; | Get remaining delay time(callback) 
function getRemainingDelayTime(requestId:number): Promise\<number>; | Get remaining delay time(Promise) 

### Usage Guidelines<a name="section129654513264"></a>

As mentioned above, applications and service modules with transient tasks have their suspension delayed so that their running is not affected by background lifecycle management within the specified time frame.

- Note: Applications and service modules can request transient tasks only for temporary tasks. The time quota is 3 minutes per time and 10 minutes per day. The system allocates the time frame based on the application scenario and system status.

#### Restrictions on Using Transient Tasks<a name="section1551164914237"></a>

Adhere to the following constraints and rules when using transient tasks:

- **When to request**：An application can request a transient task only when it is running in the foreground or before it is suspended in the background. Otherwise, the application may be suspended, resulting in request failure. By default, an application has 6–12 seconds of running time (subject to the application scenario) before it is suspended in the background.
- **Timeout**：The system notifies the application of the suspension delay timeout by using a callback. The application must then cancel the delayed suspension or apply for delayed suspension again. Otherwise, the application will be forcibly suspended.
- **When to cancel**：The requesting application shall cancel the request when the transient task is complete. If the request is forcibly canceled by the system, the time frame allowed for the application to run in the background will be affected.
- **Quota mechanism**：To prevent abuse of the keepalive, each application has a certain quota every day (dynamically adjusted based on user habits). After using up the quota, an application cannot request transient tasks. Therefore, applications should cancel their request immediately after the transient tasks are complete, to avoid quota consumption. (Note: The quota refers to the requested duration and does not include the time when the application runs in the background.)

## Repositories Involved<a name="section1371113476307"></a>

Resource Schedule subsystem

**background\_task\_mgr**

notification_ces_standard

appexecfwk_standard
