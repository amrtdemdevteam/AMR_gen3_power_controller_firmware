#pragma once

class IPCStatusChecker {
public:
    IPCStatusChecker() = default;

    void setIpcStatus(bool is_ok);
    bool pollStatusChange(bool& is_ok);

private:
    bool is_ipc_ok_ = false;
    bool last_reported_ipc_ok_ = false;
    bool has_reported_ipc_status_ = false;
};
