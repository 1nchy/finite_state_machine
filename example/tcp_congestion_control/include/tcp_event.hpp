#ifndef _TCP_EVENT_HPP_
#define _TCP_EVENT_HPP_

#include "finite_state_machine.hpp"

struct new_ack : public fsm::event {};
struct duplicate_ack : public fsm::event {};
struct timeout : public fsm::event {};
struct initialization : public fsm::event {};
struct message : public fsm::event {};

#endif // _TCP_EVENT_HPP_