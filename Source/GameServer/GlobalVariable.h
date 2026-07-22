#pragma once
#include <memory>

class ActorManager;
class SectorManager;
class UpdateThread;
class MonitorThread;

extern std::unique_ptr<ActorManager>	g_pActorManager;
extern std::unique_ptr<SectorManager>	g_pSectorManager;
extern std::unique_ptr<UpdateThread>	g_pUpdateThread;
extern std::unique_ptr<MonitorThread>	g_pMonitorThread;