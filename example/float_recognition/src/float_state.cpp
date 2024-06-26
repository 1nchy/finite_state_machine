#include "float_state.hpp"

auto float_recognition_state::operator=(const float_recognition_state& _s)
-> float_recognition_state& {
    if (&_s == this) return *this;
    _length = _s._length;
    return *this;
}

auto float_recognition_state::handle(const fsm::event& _e) -> state* {
    _end_of_float = true;
    return nullptr;
}
auto float_recognition_state::handle(const digit& _e) -> state* {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const dot& _e) -> state* {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const alpha& _e) -> state* {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const sign& _e) -> state* {
    return handle(fsm::event(_e));
}
auto float_recognition_state::transit(state* const _s) const -> state* {
    if (_end_of_float) return nullptr;
    return _s;
}
auto float_recognition_state::clone(const state* const _s) -> state* {
    const auto* const _p = dynamic_cast<const float_recognition_state* const>(_s);
    assert(_p != nullptr);
    if (this != _s && _p != nullptr) *this = *_p;
    return this;
}

auto AB::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<BCFJ>()->clone(this);
}
auto AB::handle(const sign& _e) -> state* {
    ++_length;
    return fsm::state::instance<B>()->clone(this);
}
auto B::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<BCFJ>()->clone(this);
}
auto BCFJ::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<BCFJ>()->clone(this);
}
auto BCFJ::handle(const dot& _e) -> state* {
    ++_length;
    return fsm::state::instance<D>()->clone(this);
}
auto BCFJ::handle(const alpha& _e) -> state* {
    if (_e._c != 'e') {
        _end_of_float = true;
        return nullptr;
    }
    ++_length;
    return fsm::state::instance<GH>()->clone(this);
}
auto D::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<DEFJ>()->clone(this);
}
auto DEFJ::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<DEFJ>()->clone(this);
}
auto DEFJ::handle(const alpha& _e) -> state* {
    if (_e._c != 'e') {
        _end_of_float = true;
        return nullptr;
    }
    ++_length;
    return fsm::state::instance<GH>()->clone(this);
}
auto GH::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<HIJ>()->clone(this);
}
auto GH::handle(const sign& _e) -> state* {
    ++_length;
    return fsm::state::instance<H>()->clone(this);
}
auto H::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<HIJ>()->clone(this);
}
auto HIJ::handle(const digit& _e) -> state* {
    ++_length;
    return fsm::state::instance<HIJ>()->clone(this);
}