#include "finite_state_machine.hpp"

#include "tcp_event.hpp"
#include "tcp_state.hpp"

int main() {
    auto* const _fsm = fsm::context<tcp_congestion_state>::instance();
    _fsm->accept<slow_start>();
    _fsm->start<slow_start>();
    _fsm->handle(new_ack());
    auto _r = _fsm->handle(fsm::event());
    if (!_r) {
        auto _r1 = _fsm->acceptable();
        _fsm->stop();
        _fsm->start<slow_start>();
    }
    _fsm->handle(new_ack());
    _fsm->handle(new_ack());
    _fsm->handle(new_ack());
    _fsm->handle(new_ack());
    _fsm->handle(new_ack());
    _fsm->handle(duplicate_ack());
    _fsm->handle(duplicate_ack());
    _fsm->handle(duplicate_ack());
    _fsm->handle(duplicate_ack());
    _fsm->handle(timeout());
    // fsm::context<tcp_congestion_state>::instance()->start<slow_start>();
    _fsm->stop();
    return 0;
}