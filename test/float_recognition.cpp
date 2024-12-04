#include "float_recognition.hpp"

float_recognition_state::float_recognition_state() {
    float_recognition_state::reset();
}
auto float_recognition_state::handle(const fsm::event& _e) -> label_type {
    _end_of_float = true;
    return {};
}
auto float_recognition_state::handle(const digit& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const dot& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const alpha& _e) -> label_type {
    return handle(fsm::event(_e));
}
auto float_recognition_state::handle(const sign& _e) -> label_type {
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
    _length = 0;
    _end_of_float = false;
}

auto AB::handle(const digit& _e) -> label_type {
    ++_length;
    return BCFJ::label();
}
auto AB::handle(const sign& _e) -> label_type {
    ++_length;
    return B::label();
}
auto B::handle(const digit& _e) -> label_type {
    ++_length;
    return BCFJ::label();
}
auto BCFJ::handle(const digit& _e) -> label_type {
    ++_length;
    return BCFJ::label();
}
auto BCFJ::handle(const dot& _e) -> label_type {
    ++_length;
    return D::label();
}
auto BCFJ::handle(const alpha& _e) -> label_type {
    if (_e._c != 'e') {
        _end_of_float = true;
        return state::label();
    }
    ++_length;
    return GH::label();
}
auto D::handle(const digit& _e) -> label_type {
    ++_length;
    return DEFJ::label();
}
auto DEFJ::handle(const digit& _e) -> label_type {
    ++_length;
    return DEFJ::label();
}
auto DEFJ::handle(const alpha& _e) -> label_type {
    if (_e._c != 'e') {
        _end_of_float = true;
        return state::label();
    }
    ++_length;
    return GH::label();
}
auto GH::handle(const digit& _e) -> label_type {
    ++_length;
    return HIJ::label();
}
auto GH::handle(const sign& _e) -> label_type {
    ++_length;
    return H::label();
}
auto H::handle(const digit& _e) -> label_type {
    ++_length;
    return HIJ::label();
}
auto HIJ::handle(const digit& _e) -> label_type {
    ++_length;
    return HIJ::label();
}


int main() {
    fsm::context<float_recognition_state> _fsm;

    _fsm.enroll<AB, B, BCFJ, D, DEFJ, GH, H, HIJ>();
    _fsm.accept<BCFJ, DEFJ, HIJ>();
    _fsm.default_entry<AB>();

    auto parse_float = [&](const std::string& _s, const std::string& _expect) -> bool {
        _fsm.restart();
        for (const auto& _c : _s) {
            bool _result;
            if (isdigit(_c)) {
                _result = _fsm.handle(digit());
            }
            else if (isalpha(_c)) {
                _result = _fsm.handle(alpha(_c));
            }
            else if (_c == '.') {
                _result = _fsm.handle(dot());
            }
            else if (_c == '+' || _c == '-') {
                _result = _fsm.handle(sign());
            }
            else {
                _result = _fsm.handle(fsm::event());
            }
            if (!_result) {
                size_t _len = _fsm.state()->length();
                if (_fsm.acceptable()) {
                    return _expect == _s.substr(0, _len);
                }
                else {
                    return _expect.empty();
                }
            }
        }
        if (_fsm.acceptable()) {
            size_t _len = _fsm.state()->length();
            return _expect == _s.substr(0, _len);
        }
        return _expect.empty();
    };

    assert(parse_float("1", "1"));
    assert(parse_float("-0.23", "-0.23"));
    assert(parse_float("1e9", "1e9"));
    assert(parse_float("-0.123e2.13", "-0.123e2"));
    assert(parse_float("+0.1.123e2.13", "+0.1"));
    assert(parse_float("+10.1e.123e2.13", ""));
    assert(parse_float("+10e.123e2.13", ""));
    assert(parse_float("-2.3e-3", "-2.3e-3"));
    assert(parse_float("-2e+33e-3", "-2e+33"));

    return 0;
}