#include "finite_state_machine.hpp"

#include "tcp_event.hpp"
#include "tcp_state.hpp"

int main() {
    auto* const _fsm = fsm::context<tcp_congestion_state>::instance();
    _fsm->initialize<slow_start>();
    _fsm->handle(new_ack());
    // _fsm->handle(fsm::event());
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
    // fsm::context<tcp_congestion_state>::instance()->initialize<slow_start>();
    return 0;
}