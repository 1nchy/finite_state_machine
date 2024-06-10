#ifndef _FLOAT_EVENT_HPP_
#define _FLOAT_EVENT_HPP_

#include "finite_state_machine.hpp"

struct digit : public fsm::event {}; // 0-9
struct dot : public fsm::event {}; // .
struct alpha : public fsm::event { // a-z A-Z
    alpha(char _c) : _c(_c) {}
    const char _c;
};
struct sign : public fsm::event {}; // + -

#endif // _FLOAT_EVENT_HPP_