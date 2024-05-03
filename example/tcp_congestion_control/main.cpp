#include "finite_state_machine.hpp"

#include "tcp_event.hpp"
#include "tcp_state.hpp"

struct tcp_congestion {};

int main() {
    auto* const _fsm = fsm::context<tcp_congestion_state>::instance();
    _fsm->initialize<slow_start>();
    _fsm->handle(initialization());
    _fsm->handle(new_ack());
    _fsm->handle(new_ack());
    _fsm->handle(duplicate_ack());
    _fsm->handle(duplicate_ack());
    _fsm->handle(duplicate_ack());
    _fsm->handle(duplicate_ack());
    // fsm::context<tcp_congestion_state>::instance()->initialize<slow_start>();
    return 0;
}