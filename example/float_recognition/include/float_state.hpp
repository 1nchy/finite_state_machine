#ifndef _FLOAT_STATE_HPP_
#define _FLOAT_STATE_HPP_

#include "finite_state_machine.hpp"
#include "float_event.hpp"

struct float_recognition_state : public fsm::state {
    typedef fsm::state::self self;
    float_recognition_state& operator=(const float_recognition_state&);
    virtual self* handle(const fsm::event&) override;
    virtual self* handle(const digit&);
    virtual self* handle(const dot&);
    virtual self* handle(const alpha&);
    virtual self* handle(const sign&);
    self* transit(self* const) const override;
    self* clone(const self* const) override;
    void entry() override {}
    void exit() override {}
    size_t length() const { return _length; }
    size_t _length = 0;
    bool _end_of_float = false;
};

struct AB : public float_recognition_state {
    using fsm::state::self;
    self* handle(const digit& _e) override;
    self* handle(const sign& _e) override;
    void entry() override {
        printf("start parsing\n");
    }
};
struct B : public float_recognition_state {
    self* handle(const digit& _e) override;
};
struct BCFJ : public float_recognition_state {
    using fsm::state::self;
    self* handle(const digit& _e) override;
    self* handle(const dot& _e) override;
    self* handle(const alpha& _e) override;
};
struct D : public float_recognition_state {
    using fsm::state::self;
    self* handle(const digit& _e) override;
};
struct DEFJ : public float_recognition_state {
    using fsm::state::self;
    self* handle(const digit& _e) override;
    self* handle(const alpha& _e) override;
};
struct GH : public float_recognition_state {
    using fsm::state::self;
    self* handle(const digit& _e) override;
    self* handle(const sign& _e) override;
};
struct H : public float_recognition_state {
    using fsm::state::self;
    self* handle(const digit& _e) override;
};
struct HIJ : public float_recognition_state {
    using fsm::state::self;
    self* handle(const digit& _e) override;
};

#endif // _FLOAT_STATE_HPP_