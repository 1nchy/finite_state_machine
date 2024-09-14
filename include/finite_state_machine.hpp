#ifndef _ICY_FINITE_STATE_MACHINE_HPP_
#define _ICY_FINITE_STATE_MACHINE_HPP_

#include <type_traits>
#include <typeinfo>

#include <cassert>
#include <cstdio>

#include <string>

#include <unordered_set>
#include <unordered_map>
#include <memory>

#include <source_location>

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
     * @return @c "" 状态不改变（将状态转移任务转交给 @c transit 函数）
     * @return string 希望变更到的状态的键（可能重入当前状态）
     * @throw @c std::logic_error 状态变更错误
     */
    virtual std::string_view handle(const event&) = 0;
    /**
     * @brief 状态转移
     * @param _s 实际状态指针
     * @return @c "" 不变更状态
     * @return string 希望变更到的状态的键（可能重入当前状态）
     * @throw @c std::logic_error 状态变更错误
     */
    virtual std::string_view transit(state* const _s) = 0;
    /**
     * @brief 状态复制（协变特性）
     * @param _s 被复制的状态指针
     * @return 复制后自身状态指针 @c this
     * @details 子类中实现时，应该使用 dynamic_cast 将参数转换为子类对象，再进行赋值操作
     */
    virtual state& assign(const state& _s) { return *this; }
    virtual void entry() = 0;
    virtual void exit() = 0;
};

#define FSM_STATE_LABEL \
static constexpr auto label() -> std::string_view { \
    std::string_view _name = std::source_location::current().function_name(); \
    size_t _first_colon = _name.rfind("::"); \
    size_t _space_after = _name.rfind(" ", _first_colon) + 1; \
    return _name.substr(_space_after, _first_colon - _space_after); \
}

/**
 * @brief 有限状态机
 * @tparam _Tp 有限状态类型
 */
template <typename _Tp> requires std::is_base_of<state, _Tp>::value class context {
    typedef context<_Tp> self;
public:
    // derived from fsm::state
    typedef _Tp state_type;
    context() = default;
    context(const self&) = delete;
    self& operator=(const self&) = delete;
    ~context() = default;
public:
    /**
     * @brief 状态注册
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void enroll() {
        _states.emplace(_St::label(), std::make_shared<_St>());
    }
    /**
     * @brief 事件处理
     * @tparam _Et 派生事件类型
     * @return false 状态机出错
     */
    template <typename _Et> requires std::is_base_of<event, _Et>::value
    bool handle(const _Et& _e) {
        try {
            std::string_view _ns = _M_state()->handle(_e);
            if (_ns.empty()) {
                _ns = _M_state()->transit(_M_state());
                // if (_ns.empty()) return true;
            }
            _M_transit(_ns);
            return true;
        }
        catch (const std::logic_error&) {
            return false;
        }
    }
    /**
     * @brief 状态初始化
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void start() {
        _M_transit(_St::label());
    }
    /**
     * @brief 可接受的结束状态
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void accept() {
        this->_acceptable_states.emplace(_St::label());
    }
    /**
     * @brief 不可接受的结束状态
     * @details 若可接受列表非空，则所有状态均默认为不可接受的结束状态
     * @tparam _St 状态类型
     */
    template <typename _St> requires std::is_base_of<state_type, _St>::value
    void reject() {
        this->_acceptable_states.erase(_St::label());
    }
    /**
     * @brief 关闭状态机
     */
    void stop() {
        if (_state.empty()) return;
        _M_state()->exit();
        _state = "";
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
    inline const state_type* state(std::string_view _s = "") const { return _M_state(_s); }
private:
    const state_type* _M_state(std::string_view _s = "") const {
        const std::string_view _k = (_s.empty() ? _state : _s);
        if (_states.contains(_k)) return _states.at(_k).get();
        else return nullptr;
    }
    state_type* _M_state(std::string_view _s = "") {
        const std::string_view _k = (_s.empty() ? _state : _s);
        if (_states.contains(_k)) return _states.at(_k).get();
        else return nullptr;
    }
    /**
     * @brief 状态切换
     * @param _s 状态键
     */
    void _M_transit(const std::string_view _s) {
        if (!_state.empty()) {
            _M_state()->exit();
        }
        if (!_state.empty() && !_s.empty()) {
            _M_state(_s)->assign(*_M_state());
        }
        if (!_s.empty()) {
            _state = _s;
        }
        _M_state()->entry();
    }
private:
    std::string_view _state = "";
    std::unordered_set<std::string_view> _acceptable_states;
    std::unordered_map<std::string_view, std::shared_ptr<state_type>> _states;
};

};

#endif // _ICY_FINITE_STATE_MACHINE_HPP_