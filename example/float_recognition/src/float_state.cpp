#include "float_state.hpp"

auto float_recognition_state::operator=(const float_recognition_state& _s)
-> float_recognition_state& {
    if (&_s == this) return *this;
    _length = _s._length;
    return *this;
}

auto float_recognition_state::handle(const fsm::event& _e) -> std::string_view {
    _end_of_float = true;
    return "";
}
auto float_recognition_state::handle(const digit& _e) -> std::string_view {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const dot& _e) -> std::string_view {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const alpha& _e) -> std::string_view {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const sign& _e) -> std::string_view {
    return handle(fsm::event(_e));
}
auto float_recognition_state::transit(state* const _s) -> std::string_view {
    if (_end_of_float) throw std::logic_error("");
    return "";
}
auto float_recognition_state::assign(const state& _s) -> float_recognition_state& {
    const auto& _p = dynamic_cast<const float_recognition_state&>(_s);
    *this = _p;
    return *this;
}

auto AB::handle(const digit& _e) -> std::string_view {
    ++_length;
    return BCFJ::label();
}
auto AB::handle(const sign& _e) -> std::string_view {
    ++_length;
    return B::label();
}
auto B::handle(const digit& _e) -> std::string_view {
    ++_length;
    return BCFJ::label();
}
auto BCFJ::handle(const digit& _e) -> std::string_view {
    ++_length;
    return BCFJ::label();
}
auto BCFJ::handle(const dot& _e) -> std::string_view {
    ++_length;
    return D::label();
}
auto BCFJ::handle(const alpha& _e) -> std::string_view {
    if (_e._c != 'e') {
        _end_of_float = true;
        throw std::logic_error("");
    }
    ++_length;
    return GH::label();
}
auto D::handle(const digit& _e) -> std::string_view {
    ++_length;
    return DEFJ::label();
}
auto DEFJ::handle(const digit& _e) -> std::string_view {
    ++_length;
    return DEFJ::label();
}
auto DEFJ::handle(const alpha& _e) -> std::string_view {
    if (_e._c != 'e') {
        _end_of_float = true;
        throw std::logic_error("");
    }
    ++_length;
    return GH::label();
}
auto GH::handle(const digit& _e) -> std::string_view {
    ++_length;
    return HIJ::label();
}
auto GH::handle(const sign& _e) -> std::string_view {
    ++_length;
    return H::label();
}
auto H::handle(const digit& _e) -> std::string_view {
    ++_length;
    return HIJ::label();
}
auto HIJ::handle(const digit& _e) -> std::string_view {
    ++_length;
    return HIJ::label();
}