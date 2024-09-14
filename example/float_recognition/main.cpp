#include "finite_state_machine.hpp"

#include "float_event.hpp"
#include "float_state.hpp"

#include <string>
#include <cstring>

int main() {
    fsm::context<float_recognition_state> _fsm;

    _fsm.enroll<AB>();
    _fsm.enroll<B>();
    _fsm.enroll<BCFJ>();
    _fsm.enroll<D>();
    _fsm.enroll<DEFJ>();
    _fsm.enroll<GH>();
    _fsm.enroll<H>();
    _fsm.enroll<HIJ>();

    _fsm.accept<BCFJ>();
    _fsm.accept<DEFJ>();
    _fsm.accept<HIJ>();

    _fsm.start<AB>();

    const std::string _s = "-0.114e5.14\n"; // 务必在字符串末尾加上一个字符集以外的字符，以便让自动机在循环内终止
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
                printf("start with a float! [%s]\n", _s.substr(0, _len).c_str());
            }
            else {
                printf("not start with a float. error in [%ld]\n", _len);
            }
            break;
        }
    }

    return 0;
}