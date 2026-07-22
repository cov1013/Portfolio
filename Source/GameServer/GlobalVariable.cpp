#include "../NetLib/NetEnum.h"
#include "../NetLib/NetConfig.h"
#include "../NetLib/NetDevice.h"
#include "../Common/GameEnum.h"
#include "../Common/GameConfig.h"
#include "../Common/Actor.h"
#include "../Common/ActorManager.h"
#include "../Common/SectorManager.h"
#include "UpdateThread.h"
#include "MonitorThread.h"
#include "GlobalVariable.h"

std::unique_ptr<ActorManager>	g_pActorManager = std::make_unique<ActorManager>();
std::unique_ptr<SectorManager>	g_pSectorManager = std::make_unique<SectorManager>();
std::unique_ptr<UpdateThread>	g_pUpdateThread = std::make_unique<UpdateThread>();
std::unique_ptr<MonitorThread>	g_pMonitorThread = std::make_unique<MonitorThread>();
