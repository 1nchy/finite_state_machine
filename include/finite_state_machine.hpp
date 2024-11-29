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

namespace {

template <typename _Bt> concept basic_state = std::derived_from<_Bt, state>;

}

template <basic_state _Bs> class context;

/**
 * @brief 有限状态机的事件基类
 */
struct event {};

#define FSM_STATE_LABEL \
static constexpr auto label() -> std::string_view { \
    std::string_view _name = std::source_location::current().function_name(); \
    size_t _first_colon = _name.rfind("::"); \
    size_t _space_after = _name.rfind(" ", _first_colon) + 1; \
    return _name.substr(_space_after, _first_colon - _space_after); \
}

/**
 * @brief thrown by state::handle and state::transit, to report internal error in fsm
 */
class state_error : public std::logic_error {
    using base = std::logic_error;
    using self = state_error;
public:
    explicit state_error() : base("") {}
    explicit state_error(const std::string& _arg) : base(_arg) {}
    explicit state_error(const char* _arg) : base(_arg) {}
    state_error(const self&) = default;
    self& operator=(const self&) = default;
    state_error(self&&) = default;
    self& operator=(self&&) = default;
    virtual ~state_error() override = default;
};

/**
 * @brief 有限状态机的状态基类
 * 
 * @note 基于此类派生有限状态类型 @c state_type ，其中定义用于处理所有类型事件的纯虚函数
 */
class state {
protected:
    state() = default;
public:
    FSM_STATE_LABEL
    state(const state&) = delete;
    state& operator=(const state&) = default;
    ~state() = default;
    using label_type = std::invoke_result<decltype(&state::label)>::type;
    /**
     * @brief 事件处理
     * @return label_type 希望变更到的状态的键
     * @retval state::label() 状态出现错误
     * @retval label_type() 由 transit 函数负责状态转移任务
     */
    virtual label_type handle(const event&) = 0;
    /**
     * @brief 状态转移
     * @return label_type 希望变更到的状态的键；若为默认值，则重入当前状态
     * @retval state::label() 状态出现错误
     * @retval label_type() 重入当前状态
     */
    virtual label_type transit() = 0;
    /**
     * @brief 状态复制
     * @param _s 被复制的状态对象
     * @details 子类中实现时，应该使用 dynamic_cast 将参数转换为子类对象，再进行赋值操作
     */
    virtual void assign(const state& _s) {}
    virtual void entry() {}
    virtual void exit() {}
private:
    static bool null_label(label_type _l) { return _l.empty(); }
    static bool invalid_label(label_type _l) { return _l == state::label(); }
    template <basic_state _Bs> friend class context;
};

namespace {

template <typename _Bt, typename _St> concept label_state = 
basic_state<_Bt> &&
std::derived_from<_St, _Bt> && // std::is_base_of<_Bt, _St>::value
requires { {_St::label()} -> std::same_as<state::label_type>; };

}

/**
 * @brief 有限状态机
 * @tparam _Bs 有限状态类型
 */
template <basic_state _Bs> class context {
    typedef context<_Bs> self;
    // using label_type = state::label_type;
public:
    // derived from fsm::state
    typedef _Bs state_type;
    context() = default;
    context(const self&) = delete;
    self& operator=(const self&) = delete;
    ~context() = default;
public:
    /**
     * @brief 状态注册
     * @tparam _Sts 状态类型
     */
    template <typename... _Sts> void enroll() {
        _M_enroll<_Sts...>();
    };
    /**
     * @brief 事件处理
     * @tparam _Et 派生事件类型
     * @return 状态处理结果
     * @retval true 状态处理正常
     * @retval false 状态处理出错
     * @implements state::handle -> state::transit -> context::_M_transit
     */
    template <typename _Et> requires std::derived_from<_Et, event>
    bool handle(const _Et& _e) {
        state::label_type _ns = _M_state()->handle(_e);
        if (state::null_label(_ns)) {
            _ns = _M_state()->transit();
            if (state::null_label(_ns)) { // reentry the current state
                _ns = _state;
            }
        }
        if (state::invalid_label(_ns)) {
            return false;
        }
        _M_transit(_ns);
        return true;
    }
    /**
     * @brief 状态初始化
     * @tparam _St 状态类型
     */
    template <typename _St> requires label_state<_Bs, _St>
    void start() {
        _M_transit(_St::label());
    }
    /**
     * @brief 状态初始化
     */
    void start() {
        _M_transit(_default_entry_state);
    }
    /**
     * @brief 状态重置
     * @tparam _St 状态类型
     */
    template <typename _St> requires label_state<_Bs, _St>
    void restart() {
        stop();
        start<_St>();
    }
    /**
     * @brief 状态重置
     */
    void restart() {
        stop();
        start();
    }
    /**
     * @brief 可接受的结束状态
     * @tparam _Sts 状态类型
     */
    template <typename... _Sts> void accept() {
        _M_accept<_Sts...>();
    }
    /**
     * @brief 不可接受的结束状态
     * @details 若可接受列表非空，则所有状态均默认为不可接受的结束状态
     * @tparam _Sts 状态类型
     */
    template <typename... _Sts> void reject() {
        _M_reject<_Sts...>();
    }
    /**
     * @brief 默认初始状态
     * @tparam _St 状态类型
     */
    template <typename _St> requires label_state<_Bs, _St>
    void default_entry() {
        this->_default_entry_state = _St::label();
    }
    /**
     * @brief 关闭状态机
     */
    void stop() {
        if (state::null_label(_state)) return;
        _M_state()->exit();
        _state = {};
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
    inline const state_type* state() const { return _M_state(_state); }
private:
    /**
     * @brief 状态注册
     * @tparam _St 状态类型
     */
    template <typename _St, typename... _Sts> requires label_state<_Bs, _St>
    void _M_enroll() {
        _states.emplace(_St::label(), std::make_shared<_St>());
        if constexpr (sizeof...(_Sts) != 0) {
            _M_enroll<_Sts...>();
        }
    }
    /**
     * @brief 可接受的结束状态
     * @tparam _St 状态类型
     */
    template <typename _St, typename... _Sts> requires label_state<_Bs, _St>
    void _M_accept() {
        this->_acceptable_states.emplace(_St::label());
        if constexpr (sizeof...(_Sts) != 0) {
            _M_accept<_Sts...>();
        }
    }
    /**
     * @brief 不可接受的结束状态
     * @details 若可接受列表非空，则所有状态均默认为不可接受的结束状态
     * @tparam _St 状态类型
     */
    template <typename _St, typename... _Sts> requires label_state<_Bs, _St>
    void _M_reject() {
        this->_acceptable_states.erase(_St::label());
        if constexpr (sizeof...(_Sts) != 0) {
            _M_reject<_Sts...>();
        }
    }
    const state_type* _M_state(state::label_type _s) const {
        return (_states.contains(_s) ? _states.at(_s).get() : nullptr);
    }
    state_type* _M_state(state::label_type _s) {
        return (_states.contains(_s) ? _states.at(_s).get() : nullptr);
    }
    const state_type* _M_state() const {
        return _M_state(_state);
    }
    state_type* _M_state() {
        return _M_state(_state);
    }
    /**
     * @brief 状态切换
     * @param _s 状态键（必须是合法的状态键）
     * @implements state::assign -> state::exit -> state::entry
     */
    void _M_transit(const state::label_type _s) {
        assert(!state::null_label(_s) && !state::invalid_label(_s));
        if (!state::null_label(_state)) {
            _M_state(_s)->assign(*_M_state());
            _M_state()->exit();
        }
        _state = _s;
        _M_state()->entry();
    }
private:
    state::label_type _state = {};
    state::label_type _default_entry_state = {};
    std::unordered_set<state::label_type> _acceptable_states;
    std::unordered_map<state::label_type, std::shared_ptr<state_type>> _states;
};

};

#endif // _ICY_FINITE_STATE_MACHINE_HPP_