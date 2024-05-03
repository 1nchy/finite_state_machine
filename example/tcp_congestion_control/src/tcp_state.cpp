#include "tcp_state.hpp"

auto slow_start::handle(const fsm::event& _e) -> self* {
    return this;
}
auto slow_start::handle(const new_ack& _e) -> self* {
    _dup_ack_count = 0; _cwnd += _MSS;
    printf("transmit new segment.\n");
    if (_cwnd >= _ssthresh) {
        return fsm::state::instance<congestion_avoidance>();
    }
    return this;
}
auto slow_start::handle(const duplicate_ack& _e) -> self* {
    ++_dup_ack_count;
    if (_dup_ack_count == 3) {
        return fsm::state::instance<fast_recovery>();
    }
    return this;
}
auto slow_start::handle(const timeout& _e) -> self* {
    _ssthresh = _cwnd / 2;
    _cwnd = _MSS;
    _dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return this;
}
auto slow_start::handle(const initialization& _e) -> self* {
    this->dispatch<tcp_congestion_state>(message());
    _dup_ack_count = 0; _cwnd = _MSS; _ssthresh = 64 * 1024;
    return this;
}
auto slow_start::handle(const message& _e) -> self* {
    printf("message from slow_start\n");
    return this;
}
auto slow_start::entry() -> void {
    printf("entry slow_start.\n");
    this->show();
}
auto slow_start::exit() -> void {
    this->show();
    printf("exit slow_start.\n");
}

auto congestion_avoidance::handle(const fsm::event& _e) -> self* {
    return this;
}
auto congestion_avoidance::handle(const new_ack& _e) -> self* {
    _cwnd += _MSS / _cwnd;
    _dup_ack_count = 0;
    printf("transmit new segment.\n");
    return this;
}
auto congestion_avoidance::handle(const duplicate_ack& _e) -> self* {
    ++_dup_ack_count;
    if (_dup_ack_count == 3) {
        return fsm::state::instance<fast_recovery>();
    }
    return this;
}
auto congestion_avoidance::handle(const timeout& _e) -> self* {
    auto* const _ret = fsm::state::instance<slow_start>();
    _ret->_ssthresh = _cwnd / 2;
    _ret->_cwnd = _MSS;
    _ret->_dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return _ret;
}
auto congestion_avoidance::handle(const initialization& _e) -> self* {
    this->dispatch<tcp_congestion_state>(message());
    _dup_ack_count = 0; _cwnd = _MSS; _ssthresh = 64 * 1024;
    return this;
}
auto congestion_avoidance::handle(const message& _e) -> self* {
    printf("message from congestion_avoidance\n");
    return this;
}
auto congestion_avoidance::entry() -> void {
    printf("entry congestion_avoidance.\n");
    this->show();
}
auto congestion_avoidance::exit() -> void {
    this->show();
    printf("exit congestion_avoidance.\n");
}

auto fast_recovery::handle(const fsm::event& _e) -> self* {
    return this;
}
auto fast_recovery::handle(const new_ack& _e) -> self* {
    return fsm::state::instance<congestion_avoidance>();
}
auto fast_recovery::handle(const duplicate_ack& _e) -> self* {
    _cwnd += _MSS;
    printf("retransmit new segment.\n");
    return this;
}
auto fast_recovery::handle(const timeout& _e) -> self* {
    _ssthresh = _cwnd / 2;
    _cwnd = _MSS;
    _dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return fsm::state::instance<slow_start>();
}
auto fast_recovery::handle(const initialization& _e) -> self* {
    this->dispatch<tcp_congestion_state>(message());
    _dup_ack_count = 0; _cwnd = _MSS; _ssthresh = 64 * 1024;
    return this;
}
auto fast_recovery::handle(const message& _e) -> self* {
    printf("message from fast_recovery\n");
    return this;
}
auto fast_recovery::entry() -> void {
    printf("entry fast_recovery.\n");
    this->show();
}
auto fast_recovery::exit() -> void {
    this->show();
    printf("exit fast_recovery.\n");
}