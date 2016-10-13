
#include "devices_manager.h"
#include "include/log.h"

#include "daemon.h"

namespace GLogiKd
{

DevicesManager::DevicesManager() {
	LOG(INFO) << GLogiKDaemon::is_daemon_enabled();
}

DevicesManager::~DevicesManager() {
}

} // namespace GLogiKd

