#include "finite_state_machine.hpp"

#include "float_event.hpp"
#include "float_state.hpp"

int main() {
    auto* const _fsm = fsm::context<float_recognition_state>::instance();
    _fsm->accept<BCFJ>();
    _fsm->accept<DEFJ>();
    _fsm->accept<HIJ>();
    _fsm->start<AB>();

    const std::string _s = "-0.114e5.14\n";
    for (const auto& _c : _s) {
        bool _result;
        if (isdigit(_c)) {
            _result = _fsm->handle(digit());
        }
        else if (isalpha(_c)) {
            _result = _fsm->handle(alpha(_c));
        }
        else if (_c == '.') {
            _result = _fsm->handle(dot());
        }
        else if (_c == '+' || _c == '-') {
            _result = _fsm->handle(sign());
        }
        else {
            _result = _fsm->handle(fsm::event());
        }

        if (!_result) {
            size_t _len = _fsm->state()->length();
            if (_fsm->acceptable()) {
                printf("start with a float! [%s]\n", _s.substr(0, _len).c_str());
            }
            else {
                printf("not start with a float. error in [%ld]\n", _len);
            }
            break;
        }
    }
}