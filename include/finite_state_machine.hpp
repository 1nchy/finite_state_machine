#ifndef _ASP_FINITE_STATE_MACHINE_HPP_
#define _ASP_FINITE_STATE_MACHINE_HPP_

#include <type_traits>
#include <typeinfo>

#include <cassert>
#include <cstdio>

namespace fsm {

struct event;
class state;
template <typename _Tp> requires std::is_base_of<state, _Tp>::value class context;

/**
 * @brief 有限状态机的事件基类
*/
struct event {
protected:
    event() = default;
};

/**
 * @brief 有限状态机的状态基类
 * 
 * 基于此类派生有限状态类型 @c state_type ，其中定义用于处理所有类型事件的纯虚函数
*/
class state {
protected:
    typedef state self;
    state() = default;
public:
    state(const self&) = delete;
    self& operator=(const self&) = delete;
    ~state() = default;
    virtual self* handle(const event&) = 0;
    virtual void entry() = 0;
    virtual void exit() = 0;
    template <typename _Tp, typename _Et>
        requires std::is_base_of<state, _Tp>::value && std::is_base_of<event, _Et>::value
        void dispatch(const _Et&);
    template <typename _Tp>
        requires std::is_base_of<state, _Tp>::value
        static _Tp* instance();
};

/**
 * @brief 有限状态机
 * @tparam _Tp 有限状态类型
*/
template <typename _Tp> requires std::is_base_of<state, _Tp>::value class context {
    typedef context<_Tp> self;
    context() = default;
public:
    // derived from fsm::state
    typedef _Tp state_type;
    context(const self&) = delete;
    self& operator=(const self&) = delete;
    ~context() = default;
    static self* instance();
    template <typename _Et>
        requires std::is_base_of<event, _Et>::value
        void handle(const _Et&);
    template <typename _St>
        requires std::is_base_of<state_type, _St>::value
        void initialize();
private:
    template <typename _St>
        requires std::is_base_of<state_type, _St>::value
        void transit();
private:
    state_type* _state = nullptr;
};


/**
 * @brief 向指定的有限状态机发送事件
 * @tparam _Tp 有限状态类型（@c state 派生类）
 * @tparam _Et 派生事件类型（@c event 派生类）
*/
template <typename _Tp, typename _Et>
requires std::is_base_of<state, _Tp>::value && std::is_base_of<event, _Et>::value
auto state::dispatch(const _Et& _e) -> void {
    context<_Tp>::instance()->handle(_e);
}
/**
 * @brief 状态单例
 * @tparam _Tp 状态类型（@c state_type 有限状态类型的派生类）
*/
template <typename _Tp> requires std::is_base_of<state, _Tp>::value
auto state::instance() -> _Tp* {
    static _Tp _s;
    return &_s;
}


template <typename _Tp> auto context<_Tp>::instance() -> self* {
    static self _s;
    return &_s;
}
/**
 * @brief 事件处理
 * @tparam _Et 派生事件类型
*/
template <typename _Tp> template <typename _Et>
requires std::is_base_of<event, _Et>::value
auto context<_Tp>::handle(const _Et& _e) -> void {
    auto* const _state_handled_result = _state->handle(_e);
    if (_state_handled_result == nullptr) return;
    auto* const _new_state = dynamic_cast<state_type*>(_state_handled_result);
    assert(_new_state != nullptr);
    if (_new_state != _state) {
        _state->exit();
        _new_state->entry();
        _state = _new_state;
    }
}
/**
 * @brief 状态初始化
 * @tparam _St 状态类型
*/
template <typename _Tp> template <typename _St>
requires std::is_base_of<typename context<_Tp>::state_type, _St>::value
auto context<_Tp>::initialize() -> void {
    assert(this->_state == nullptr);
    transit<_St>();
}
/**
 * @brief 状态切换
 * @tparam _St 状态类型
*/
template <typename _Tp> template <typename _St>
requires std::is_base_of<typename context<_Tp>::state_type, _St>::value
auto context<_Tp>::transit() -> void {
    if (this->_state != nullptr) {
        this->_state->exit();
    }
    this->_state = state::instance<_St>();
    this->_state->entry();
}

};

#endif // _ASP_FINITE_STATE_MACHINE_HPP_