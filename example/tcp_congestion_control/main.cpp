#include "finite_state_machine.hpp"

#include "tcp_event.hpp"
#include "tcp_state.hpp"

using namespace icy;

int main() {
    fsm::context<tcp_congestion_state> _fsm;
    _fsm.enroll<slow_start, fast_recovery, congestion_avoidance>();
    _fsm.accept<slow_start>();
    _fsm.default_entry<slow_start>();
    _fsm.start();
    _fsm.handle(new_ack());
    auto _r = _fsm.handle(fsm::event());
    if (!_r) {
        auto _r1 = _fsm.acceptable();
        _fsm.restart();
    }
    _fsm.handle(new_ack());
    _fsm.handle(new_ack());
    _fsm.handle(new_ack());
    _fsm.handle(new_ack());
    _fsm.handle(new_ack());
    _fsm.handle(duplicate_ack());
    _fsm.handle(duplicate_ack());
    _fsm.handle(duplicate_ack());
    _fsm.handle(duplicate_ack());
    _fsm.handle(timeout());
    // fsm::context<tcp_congestion_state>::instance()->start<slow_start>();
    _fsm.stop();
    return 0;
}