#include "float_state.hpp"

using namespace icy;

float_recognition_state::float_recognition_state() {
    float_recognition_state::reset();
}
auto float_recognition_state::handle(const fsm::event& _e) -> label_type {
    _end_of_float = true;
    return {};
}
auto float_recognition_state::handle(const fsm::character::digit& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const fsm::character::dot& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const fsm::character::alpha& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const fsm::character::plus& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const fsm::character::minus& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::transit() -> label_type {
    if (_end_of_float) return state::label();
    return {};
}
auto float_recognition_state::assign(const state& _s) -> void {
    this->operator=(dynamic_cast<const float_recognition_state&>(_s));
}
auto float_recognition_state::reset() -> void {
    _length = 0; _end_of_float = false;
}

auto AB::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return BCFJ::label();
}
auto AB::handle(const fsm::character::plus& _e) -> label_type {
    ++_length;
    return B::label();
}
auto AB::handle(const fsm::character::minus& _e) -> label_type {
    ++_length;
    return B::label();
}
auto B::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return BCFJ::label();
}
auto BCFJ::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return BCFJ::label();
}
auto BCFJ::handle(const fsm::character::dot& _e) -> label_type {
    ++_length;
    return D::label();
}
auto BCFJ::handle(const fsm::character::alpha& _e) -> label_type {
    if (_e.value() != 'e') {
        _end_of_float = true;
        return state::label();
    }
    ++_length;
    return GH::label();
}
auto D::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return DEFJ::label();
}
auto DEFJ::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return DEFJ::label();
}
auto DEFJ::handle(const fsm::character::alpha& _e) -> label_type {
    if (_e.value() != 'e') {
        _end_of_float = true;
        return state::label();
    }
    ++_length;
    return GH::label();
}
auto GH::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return HIJ::label();
}
auto GH::handle(const fsm::character::plus& _e) -> label_type {
    ++_length;
    return H::label();
}
auto GH::handle(const fsm::character::minus& _e) -> label_type {
    ++_length;
    return H::label();
}
auto H::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return HIJ::label();
}
auto HIJ::handle(const fsm::character::digit& _e) -> label_type {
    ++_length;
    return HIJ::label();
}