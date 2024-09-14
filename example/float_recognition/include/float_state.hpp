#ifndef _FLOAT_STATE_HPP_
#define _FLOAT_STATE_HPP_

#include "finite_state_machine.hpp"
#include "float_event.hpp"

struct float_recognition_state : public fsm::state {
    using state = fsm::state;
    float_recognition_state& operator=(const float_recognition_state&);
    virtual std::string_view handle(const fsm::event&) override;
    virtual std::string_view handle(const digit&);
    virtual std::string_view handle(const dot&);
    virtual std::string_view handle(const alpha&);
    virtual std::string_view handle(const sign&);
    std::string_view transit(state* const) override;
    void assign(const state&) override;
    void entry() override {}
    void exit() override {}
    size_t length() const { return _length; }
    size_t _length = 0;
    bool _end_of_float = false;
};

struct AB : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    std::string_view handle(const digit& _e) override;
    std::string_view handle(const sign& _e) override;
    void entry() override {
        printf("start parsing\n");
    }
};
struct B : public float_recognition_state {
    FSM_STATE_LABEL
    std::string_view handle(const digit& _e) override;
};
struct BCFJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    std::string_view handle(const digit& _e) override;
    std::string_view handle(const dot& _e) override;
    std::string_view handle(const alpha& _e) override;
};
struct D : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    std::string_view handle(const digit& _e) override;
};
struct DEFJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    std::string_view handle(const digit& _e) override;
    std::string_view handle(const alpha& _e) override;
};
struct GH : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    std::string_view handle(const digit& _e) override;
    std::string_view handle(const sign& _e) override;
};
struct H : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    std::string_view handle(const digit& _e) override;
};
struct HIJ : public float_recognition_state {
    FSM_STATE_LABEL
    using state = fsm::state;
    std::string_view handle(const digit& _e) override;
};

#endif // _FLOAT_STATE_HPP_