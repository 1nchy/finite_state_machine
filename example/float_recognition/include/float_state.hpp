#ifndef _FLOAT_STATE_HPP_
#define _FLOAT_STATE_HPP_

#include "finite_state_machine.hpp"

struct float_recognition_state : public fsm::state {
    using state = fsm::state;
    float_recognition_state();
    float_recognition_state& operator=(const float_recognition_state&) = default;
    virtual label_type handle(const fsm::event&) override;
    virtual label_type handle(const fsm::character::digit&);
    virtual label_type handle(const fsm::character::dot&);
    virtual label_type handle(const fsm::character::alpha&);
    virtual label_type handle(const fsm::character::plus&);
    virtual label_type handle(const fsm::character::minus&);
    label_type transit() override;
    void assign(const state&) override;
    void reset() override;
    void entry() override {}
    void exit() override {}
    size_t length() const { return _length; }
    size_t _length = 0;
    bool _end_of_float = false;
};

struct AB : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    label_type handle(const fsm::character::digit& _e) override;
    label_type handle(const fsm::character::plus& _e) override;
    label_type handle(const fsm::character::minus& _e) override;
    void entry() override {
        printf("start parsing\n");
    }
};
struct B : public float_recognition_state {
    FSM_STATE_LABEL
    label_type handle(const fsm::character::digit& _e) override;
};
struct BCFJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    label_type handle(const fsm::character::digit& _e) override;
    label_type handle(const fsm::character::dot& _e) override;
    label_type handle(const fsm::character::alpha& _e) override;
};
struct D : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    label_type handle(const fsm::character::digit& _e) override;
};
struct DEFJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    label_type handle(const fsm::character::digit& _e) override;
    label_type handle(const fsm::character::alpha& _e) override;
};
struct GH : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    label_type handle(const fsm::character::digit& _e) override;
    label_type handle(const fsm::character::plus& _e) override;
    label_type handle(const fsm::character::minus& _e) override;
};
struct H : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    label_type handle(const fsm::character::digit& _e) override;
};
struct HIJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    label_type handle(const fsm::character::digit& _e) override;
};

#endif // _FLOAT_STATE_HPP_