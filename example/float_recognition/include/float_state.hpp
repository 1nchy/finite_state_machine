#ifndef _FLOAT_STATE_HPP_
#define _FLOAT_STATE_HPP_

#include "finite_state_machine.hpp"
#include "float_event.hpp"

struct float_recognition_state : public fsm::state {
    using state = fsm::state;
    float_recognition_state& operator=(const float_recognition_state&);
    virtual state* handle(const fsm::event&) override;
    virtual state* handle(const digit&);
    virtual state* handle(const dot&);
    virtual state* handle(const alpha&);
    virtual state* handle(const sign&);
    state* transit(state* const) const override;
    state* clone(const state* const) override;
    void entry() override {}
    void exit() override {}
    size_t length() const { return _length; }
    size_t _length = 0;
    bool _end_of_float = false;
};

struct AB : public float_recognition_state {
    using state = fsm::state;
    state* handle(const digit& _e) override;
    state* handle(const sign& _e) override;
    void entry() override {
        printf("start parsing\n");
    }
};
struct B : public float_recognition_state {
    state* handle(const digit& _e) override;
};
struct BCFJ : public float_recognition_state {
    using state = fsm::state;
    state* handle(const digit& _e) override;
    state* handle(const dot& _e) override;
    state* handle(const alpha& _e) override;
};
struct D : public float_recognition_state {
    using state = fsm::state;
    state* handle(const digit& _e) override;
};
struct DEFJ : public float_recognition_state {
    using state = fsm::state;
    state* handle(const digit& _e) override;
    state* handle(const alpha& _e) override;
};
struct GH : public float_recognition_state {
    using state = fsm::state;
    state* handle(const digit& _e) override;
    state* handle(const sign& _e) override;
};
struct H : public float_recognition_state {
    using state = fsm::state;
    state* handle(const digit& _e) override;
};
struct HIJ : public float_recognition_state {
    using state = fsm::state;
    state* handle(const digit& _e) override;
};

#endif // _FLOAT_STATE_HPP_