#ifndef _ICY_FINITE_STATE_MACHINE_HPP_
#define _ICY_FINITE_STATE_MACHINE_HPP_

#include <type_traits>
#include <typeinfo>

#include <cassert>
#include <cstdio>

#include <unordered_set>

namespace fsm {

struct event;
class state;
template <typename _Tp> requires std::is_base_of<state, _Tp>::value class context;

/**
 * @brief 有限状态机的事件基类
 */
struct event {};

/**
 * @brief 有限状态机的状态基类
 * 
 * 基于此类派生有限状态类型 @c state_type ，其中定义用于处理所有类型事件的纯虚函数
 */
class state {
protected:
    state() = default;
public:
    state(const state&) = delete;
    state& operator=(const state&) = default;
    ~state() = default;
    /**
     * @brief 事件处理（默认的事件处理函数是事实上的意外处理函数）
     * @return @c nullptr 状态不改变（将状态转移任务转交给 @c transit 函数）
     * @return @c this 重入当前状态
     * @return pointer 希望变更到的状态
     */
    virtual state* handle(const event&) = 0;
    /**
     * @brief 状态转移
     * @param _s 实际状态指针
     * @return @c nullptr 自检错误
     * @return @c  _s 重入当前状态
     * @return pointer 希望变更到的状态
     */
    virtual state* transit(state* const _s) const = 0;
    /**
     * @brief 状态复制
     * @param _s 被复制的状态指针
     * @return 复制后自身状态指针 @c this
     * @details 子类中实现时，应该使用 dynamic_cast 将参数转换为子类指针，再进行复制操作
     */
    virtual state* clone(const state* const _s) = 0;
    virtual void entry() = 0;
    virtual void exit() = 0;
    /**
     * @brief 向指定的有限状态机发送事件
     * @tparam _Tp 有限状态类型（@c state 派生类）
     * @tparam _Et 派生事件类型（@c event 派生类）
     */
    template <typename _Tp, typename _Et> requires std::is_base_of<state, _Tp>::value && std::is_base_of<event, _Et>::value
    void dispatch(const _Et& _e) {
        context<_Tp>::instance()->handle(_e);
    }
    /**
     * @brief 状态单例
     * @tparam _Tp 状态类型（@c state_type 有限状态类型的派生类）
     */
    template <typename _Tp> requires std::is_base_of<state, _Tp>::value
    static _Tp* instance() {
        static _Tp _s;
        return &_s;
    }
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
    static self* instance() {
        static self _s;
        return &_s;
    }
    /**
     * @brief 事件处理
     * @tparam _Et 派生事件类型
     */
    template <typename _Et> requires std::is_base_of<event, _Et>::value
    bool handle(const _Et& _e) {
        auto* _result = _state->handle(_e);
        if (_result == nullptr) {
            _result = _state->transit(_state);
            if (_result == nullptr) {
                return false;
            }
        }
        auto* const _new_state = dynamic_cast<state_type*>(_result);
        assert(_new_state != nullptr);
        _state->exit();
        _new_state->entry();
        _state = _new_state;
        return true;
    }
    /**
     * @brief 状态初始化
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void start() {
        assert(this->_state == nullptr);
        transit<_St>();
    }
    /**
     * @brief 可接受的结束状态
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void accept() {
        this->_acceptable_states.insert(state::instance<_St>());
    }
    /**
     * @brief 不可接受的结束状态
     * @details 若可接受列表非空，则所有状态均默认为不可接受的结束状态
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void reject() {
        this->_acceptable_states.erase(state::instance<_St>());
    }
    /**
     * @brief 关闭状态机
     */
    void stop() {
        assert(this->_state != nullptr);
        this->_state->exit();
        this->_state = nullptr;
    }
    /**
     * @brief 当前状态是否可接受
     */
    bool acceptable() const {
        return _acceptable_states.contains(_state);
    }
    /**
     * @brief 返回当前状态
     */
    const state_type* state() const {
        return _state;
    }
private:
    /**
     * @brief 状态切换
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void transit() {
        if (this->_state != nullptr) {
            this->_state->exit();
        }
        this->_state = state::instance<_St>();
        this->_state->entry();
    }
private:
    state_type* _state = nullptr;
    std::unordered_set<state_type*> _acceptable_states;
};

};

#endif // _ICY_FINITE_STATE_MACHINE_HPP_