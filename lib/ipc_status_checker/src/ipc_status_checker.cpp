#include "ipc_status_checker.hpp"

// --------- public methods begin ---------
void IPCStatusChecker::setIpcStatus(bool is_ok) {
    is_ipc_ok_ = is_ok;
}

bool IPCStatusChecker::pollStatusChange(bool& is_ok) {
    if (!has_reported_ipc_status_ || (last_reported_ipc_ok_ != is_ipc_ok_)) {
        last_reported_ipc_ok_ = is_ipc_ok_;
        has_reported_ipc_status_ = true;
        is_ok = is_ipc_ok_;
        return true;
    }

    return false;
}

// --------- public methods end ---------