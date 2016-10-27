
#include "daemon_control.h"

namespace GLogiKd
{

std::atomic<bool> DaemonControl::daemonized_(false);

DaemonControl::DaemonControl() {
}

DaemonControl::~DaemonControl() {
}

void DaemonControl::disable_daemon( void ) {
	DaemonControl::daemonized_ = false;
}

bool DaemonControl::is_daemon_enabled() {
	return DaemonControl::daemonized_;
}

} // namespace GLogiKd

