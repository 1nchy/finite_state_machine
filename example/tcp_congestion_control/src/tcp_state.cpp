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
    _ssthresh = _cwnd / 2;
    _cwnd = _MSS;
    _dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return fsm::state::instance<slow_start>()->clone(this);
}
auto tcp_congestion_state::transit(self* const _s) const -> self* {
    if (_fault) return nullptr;
    if (_dup_ack_count >= 3) {
        auto* const _ret = dynamic_cast<fast_recovery*>(fsm::state::instance<fast_recovery>()->clone(this));
        assert(_ret != nullptr);
        _ret->_ssthresh = _cwnd / 2;
        _ret->_cwnd = _ret->_ssthresh + 3 * _MSS;
        _ret->_dup_ack_count = 0;
        return _ret;
    }
    return _s;
}
auto tcp_congestion_state::clone(const self* const _s)
-> self* {
    const auto* const _p = dynamic_cast<const tcp_congestion_state* const>(_s);
    assert(_p != nullptr);
    if (this != _s && _p != nullptr) *this = *_p;
    return this;
}


auto slow_start::handle(const fsm::event& _e) -> self* {
    printf("message from slow_start\n");
    _fault = true;
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
auto slow_start::transit(self* const _s) const -> self* {
    if (_cwnd >= _ssthresh) {
        return fsm::state::instance<congestion_avoidance>()->clone(this);
    }
    return tcp_congestion_state::transit(_s);
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
    _cwnd = _ssthresh;
    _dup_ack_count = 0;
    return fsm::state::instance<congestion_avoidance>()->clone(this);
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