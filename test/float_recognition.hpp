#ifndef _ICY_FINITE_STATE_MACHINE_TEST_FLOAT_RECOGNITION_HPP_
#define _ICY_FINITE_STATE_MACHINE_TEST_FLOAT_RECOGNITION_HPP_

#include "finite_state_machine.hpp"

#include <string>

/**
 * @details [+-]? [0-9]^+ [\.[0-9]^+]? [e[+-]?[0-9]^+]?
 */
struct float_recognition_state : public icy::fsm::state {
    using state = icy::fsm::state;
    float_recognition_state();
    float_recognition_state& operator=(const float_recognition_state&) = default;
    virtual label_type handle(const icy::fsm::event&) override;
    virtual label_type handle(const icy::fsm::character::digit&);
    virtual label_type handle(const icy::fsm::character::dot&);
    virtual label_type handle(const icy::fsm::character::alpha&);
    virtual label_type handle(const icy::fsm::character::plus&);
    virtual label_type handle(const icy::fsm::character::minus&);
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
    using state = icy::fsm::state;
    label_type handle(const icy::fsm::character::digit& _e) override;
    label_type handle(const icy::fsm::character::plus& _e) override;
    label_type handle(const icy::fsm::character::minus& _e) override;
    void entry() override {
        printf("start parsing\n");
    }
};
struct B : public float_recognition_state {
    FSM_STATE_LABEL
    label_type handle(const icy::fsm::character::digit& _e) override;
};
struct BCFJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = icy::fsm::state;
    label_type handle(const icy::fsm::character::digit& _e) override;
    label_type handle(const icy::fsm::character::dot& _e) override;
    label_type handle(const icy::fsm::character::alpha& _e) override;
};
struct D : public float_recognition_state {
    FSM_STATE_LABEL
    using state = icy::fsm::state;
    label_type handle(const icy::fsm::character::digit& _e) override;
};
struct DEFJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = icy::fsm::state;
    label_type handle(const icy::fsm::character::digit& _e) override;
    label_type handle(const icy::fsm::character::alpha& _e) override;
};
struct GH : public float_recognition_state {
    FSM_STATE_LABEL
    using state = icy::fsm::state;
    label_type handle(const icy::fsm::character::digit& _e) override;
    label_type handle(const icy::fsm::character::plus& _e) override;
    label_type handle(const icy::fsm::character::minus& _e) override;
};
struct H : public float_recognition_state {
    FSM_STATE_LABEL
    using state = icy::fsm::state;
    label_type handle(const icy::fsm::character::digit& _e) override;
};
struct HIJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = icy::fsm::state;
    label_type handle(const icy::fsm::character::digit& _e) override;
};

bool parse_float(icy::fsm::context<float_recognition_state>& _fsm, const std::string& _s, const std::string& _f);

#endif // _ICY_FINITE_STATE_MACHINE_TEST_FLOAT_RECOGNITION_HPP_