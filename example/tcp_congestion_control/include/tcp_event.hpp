#ifndef _TCP_EVENT_HPP_
#define _TCP_EVENT_HPP_

#include "finite_state_machine.hpp"

struct new_ack : public icy::fsm::event {};
struct duplicate_ack : public icy::fsm::event {};
struct timeout : public icy::fsm::event {};
struct initialization : public icy::fsm::event {};
struct message : public icy::fsm::event {};

#endif // _TCP_EVENT_HPP_