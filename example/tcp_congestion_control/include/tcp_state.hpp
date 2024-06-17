#ifndef _TCP_STATE_HPP_
#define _TCP_STATE_HPP_

#include <cstddef>
#include <cstdio>

#include "finite_state_machine.hpp"
#include "tcp_event.hpp"

struct tcp_congestion_state : public fsm::state {
    using state = fsm::state;
    tcp_congestion_state& operator=(const tcp_congestion_state&);
    virtual state* handle(const fsm::event&) override = 0;
    virtual state* handle(const new_ack&) = 0;
    virtual state* handle(const duplicate_ack&) = 0;
    virtual state* handle(const timeout&);
    state* transit(state* const) const override;
    state* clone(const state* const) override;
    void entry() override {}
    void exit() override {}
    const size_t _MSS = 1460;
    size_t _dup_ack_count = 0;
    size_t _cwnd = _MSS;
    size_t _ssthresh = 64 * 1024;
    bool _fault = false;
    // void clone(const tcp_congestion_state* const _s); // assignment for sub state
    void show() {
        printf("  (dup ack count = %ld, cwnd = %ld, _ssthresh = %ld)\n", _dup_ack_count, _cwnd, _ssthresh);
    }
};

struct slow_start;
struct congestion_avoidance;
struct fast_recovery;

struct slow_start : public tcp_congestion_state {
    using state = fsm::state;
    state* handle(const fsm::event& _e) override;
    state* handle(const new_ack& _e) override;
    state* handle(const duplicate_ack& _e) override;
    state* transit(state* const) const override;
    void entry() override;
    void exit() override;
};
struct congestion_avoidance : public tcp_congestion_state {
    using state = fsm::state;
    state* handle(const fsm::event& _e) override;
    state* handle(const new_ack& _e) override;
    state* handle(const duplicate_ack& _e) override;
    void entry() override;
    void exit() override;
};
struct fast_recovery : public tcp_congestion_state {
    using state = fsm::state;
    state* handle(const fsm::event& _e) override;
    state* handle(const new_ack& _e) override;
    state* handle(const duplicate_ack& _e) override;
    void entry() override;
    void exit() override;
};


#endif // _TCP_STATE_HPP_