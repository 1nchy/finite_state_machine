#include "tcp_state.hpp"

auto tcp_congestion_state::operator=(const tcp_congestion_state& _s)
-> tcp_congestion_state& {
    if (&_s == this) return *this;
    _dup_ack_count = _s._dup_ack_count;
    _cwnd = _s._cwnd;
    _ssthresh = _s._ssthresh;
    return *this;
}
auto tcp_congestion_state::handle(const timeout& _e) -> self* {
    auto* const _ret = fsm::state::instance<slow_start>();
    _ret->_ssthresh = _cwnd / 2;
    _ret->_cwnd = _MSS;
    _ret->_dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return _ret;
}
auto tcp_congestion_state::transit() const -> self* {
    if (_dup_ack_count >= 3) {
        auto* const _ret = fsm::state::instance<fast_recovery>();
        _ret->_ssthresh = _cwnd / 2;
        _ret->_cwnd = _ret->_ssthresh + 3 * _MSS;
        _ret->_dup_ack_count = 0;
        return _ret;
    }
    return nullptr;
}
auto tcp_congestion_state::clone(const tcp_congestion_state* const _s)
-> void {
    if (this == _s) return;
    *this = *_s;
}


auto slow_start::handle(const fsm::event& _e) -> self* {
    printf("message from slow_start\n");
    this->dispatch<tcp_congestion_state>(timeout());
    return nullptr;
}
auto slow_start::handle(const new_ack& _e) -> self* {
    _dup_ack_count = 0; _cwnd += _MSS;
    printf("transmit new segment.\n");
    return nullptr;
}
auto slow_start::handle(const duplicate_ack& _e) -> self* {
    ++_dup_ack_count;
    return nullptr;
}
auto slow_start::transit() const -> self* {
    if (_cwnd >= _ssthresh) {
        auto* const _ret = fsm::state::instance<congestion_avoidance>();
        _ret->clone(this);
        return _ret;
    }
    return tcp_congestion_state::transit();
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
    printf("message from congestion_avoidance\n");
    return nullptr;
}
auto congestion_avoidance::handle(const new_ack& _e) -> self* {
    _cwnd += _MSS / _cwnd;
    _dup_ack_count = 0;
    printf("transmit new segment.\n");
    return nullptr;
}
auto congestion_avoidance::handle(const duplicate_ack& _e) -> self* {
    ++_dup_ack_count;
    return nullptr;
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
    printf("message from fast_recovery\n");
    return nullptr;
}
auto fast_recovery::handle(const new_ack& _e) -> self* {
    auto* const _ret = fsm::state::instance<congestion_avoidance>();
    _ret->_cwnd = _ssthresh;
    _ret->_ssthresh = _ssthresh;
    _ret->_dup_ack_count = 0;
    return _ret;
}
auto fast_recovery::handle(const duplicate_ack& _e) -> self* {
    _cwnd += _MSS;
    printf("retransmit new segment.\n");
    return nullptr;
}
auto fast_recovery::entry() -> void {
    printf("entry fast_recovery.\n");
    this->show();
}
auto fast_recovery::exit() -> void {
    this->show();
    printf("exit fast_recovery.\n");
}