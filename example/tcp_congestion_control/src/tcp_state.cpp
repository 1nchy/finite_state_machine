#include "tcp_state.hpp"

auto tcp_congestion_state::operator=(const tcp_congestion_state& _s)
-> tcp_congestion_state& {
    if (&_s == this) return *this;
    _dup_ack_count = _s._dup_ack_count;
    _cwnd = _s._cwnd;
    _ssthresh = _s._ssthresh;
    return *this;
}
auto tcp_congestion_state::handle(const timeout& _e) -> std::string_view {
    _ssthresh = _cwnd / 2;
    _cwnd = _MSS;
    _dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return slow_start::label();
}
auto tcp_congestion_state::transit(state* const _s) -> std::string_view {
    if (_fault) throw std::logic_error("internal error in fsm");
    if (_dup_ack_count >= 3) {
        const auto _old_ssthresh = _ssthresh;
        _ssthresh = _cwnd / 2;
        _cwnd = _old_ssthresh + 3 * _MSS;
        _dup_ack_count = 0;
        return fast_recovery::label();
    }
    return "";
}
auto tcp_congestion_state::assign(const state& _s) -> void {
    const auto& _p = dynamic_cast<const tcp_congestion_state&>(_s);
    *this = _p;
}


auto slow_start::handle(const fsm::event& _e) -> std::string_view {
    printf("message from slow_start\n");
    _fault = true;
    return "";
}
auto slow_start::handle(const new_ack& _e) -> std::string_view {
    _dup_ack_count = 0; _cwnd += _MSS;
    printf("transmit new segment.\n");
    return "";
}
auto slow_start::handle(const duplicate_ack& _e) -> std::string_view {
    ++_dup_ack_count;
    return "";
}
auto slow_start::transit(state* const _s) -> std::string_view {
    if (_cwnd >= _ssthresh) {
        return congestion_avoidance::label();
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

auto congestion_avoidance::handle(const fsm::event& _e) -> std::string_view {
    printf("message from congestion_avoidance\n");
    return "";
}
auto congestion_avoidance::handle(const new_ack& _e) -> std::string_view {
    _cwnd += _MSS / _cwnd;
    _dup_ack_count = 0;
    printf("transmit new segment.\n");
    return "";
}
auto congestion_avoidance::handle(const duplicate_ack& _e) -> std::string_view {
    ++_dup_ack_count;
    return "";
}
auto congestion_avoidance::entry() -> void {
    printf("entry congestion_avoidance.\n");
    this->show();
}
auto congestion_avoidance::exit() -> void {
    this->show();
    printf("exit congestion_avoidance.\n");
}

auto fast_recovery::handle(const fsm::event& _e) -> std::string_view {
    printf("message from fast_recovery\n");
    return "";
}
auto fast_recovery::handle(const new_ack& _e) -> std::string_view {
    _cwnd = _ssthresh;
    _dup_ack_count = 0;
    return congestion_avoidance::label();
}
auto fast_recovery::handle(const duplicate_ack& _e) -> std::string_view {
    _cwnd += _MSS;
    printf("retransmit new segment.\n");
    return "";
}
auto fast_recovery::entry() -> void {
    printf("entry fast_recovery.\n");
    this->show();
}
auto fast_recovery::exit() -> void {
    this->show();
    printf("exit fast_recovery.\n");
}