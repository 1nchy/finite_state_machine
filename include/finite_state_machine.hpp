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

namespace icy {

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
    virtual void reset() {}
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
        _M_reset();
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
     * @brief reset state inner data
     */
    void _M_reset() {
        for (auto& [_l, _s] : _states) {
            _s->reset();
        }
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

namespace character {

struct ascii_code : public fsm::event {
    constexpr ascii_code(char _c): _val(_c) {}
    inline constexpr char value() const { return _val; }
private:
    const char _val;
};
struct printable_code : public ascii_code {
    printable_code(char _c) : ascii_code(_c) {
        if (!isprint(_c)) throw std::out_of_range("not printable code");
    }
};
struct control_code : public ascii_code {
    control_code(char _c) : ascii_code(_c) {
        if (isprint(_c)) throw std::out_of_range("not control code");
    }
};
struct alnum : public printable_code { // a-zA-Z0-9
    alnum(char _c) : printable_code(_c) {
        if (!isalnum(_c)) throw std::out_of_range("not in [a-zA-Z0-9]");
    }
};
struct alpha : public alnum { // a-zA-Z
    alpha(char _c) : alnum(_c) {
        if (!isalpha(_c)) throw std::out_of_range("not in [a-zA-Z]");
    }
};
struct lower_case : public alpha { // a-z
    lower_case(char _c) : alpha(_c) {
        if (!islower(_c)) throw std::out_of_range("not in [a-z]");
    }
};
struct upper_case : public alpha { // A-Z
    upper_case(char _c) : alpha(_c) {
        if (!isupper(_c)) throw std::out_of_range("not in [A-Z]");
    }
};
struct digit : public alnum { // 0-9
    digit(char _c) : alnum(_c) {
        if (!isdigit(_c)) throw std::out_of_range("not in [0-9]");
    }
};

struct plus : public printable_code { // +
    plus() : printable_code('+') {}
};
struct minus : public printable_code { // -
    minus() : printable_code('-') {}
};
struct asterisk : public printable_code { // *
    asterisk() : printable_code('*') {}
};
struct slash : public printable_code { // /
    slash() : printable_code('/') {}
};
struct assignment : public printable_code { // =
    assignment() : printable_code('=') {}
};
struct dot : public printable_code { // .
    dot() : printable_code('.') {}
};
struct comma : public printable_code { // ,
    comma() : printable_code(',') {}
};
struct vertical : public printable_code { // |
    vertical() : printable_code('|') {}
};
struct underline : public printable_code { // _
    underline() : printable_code('_') {}
};

struct left_angle : public printable_code { // <
    left_angle() : printable_code('<') {}
};
struct right_angle : public printable_code { // >
    right_angle() : printable_code('>') {}
};
struct left_parentheses : public printable_code { // (
    left_parentheses() : printable_code('(') {}
};
struct right_parentheses : public printable_code { // )
    right_parentheses() : printable_code(')') {}
};
struct left_square : public printable_code { // [
    left_square() : printable_code('[') {}
};
struct right_square : public printable_code { // ]
    right_square() : printable_code(']') {}
};
struct left_curly : public printable_code { // {
    left_curly() : printable_code('{') {}
};
struct right_curly : public printable_code { // }
    right_curly() : printable_code('}') {}
};

struct single_quote : public printable_code { // '
    single_quote() : printable_code('\'') {}
};
struct double_quote : public printable_code { // "
    double_quote() : printable_code('\"') {}
};
struct back_quote : public printable_code { // `
    back_quote() : printable_code('`') {}
};
struct tilde : public printable_code { // ~
    tilde() : printable_code('~') {}
};
struct backslash : public printable_code { // 
    backslash() : printable_code('\\') {}
};
struct question : public printable_code { // ?
    question() : printable_code('?') {}
};
struct colon : public printable_code { // :
    colon() : printable_code(':') {}
};
struct semicolon : public printable_code { // ;
    semicolon() : printable_code(';') {}
};

struct exclamation : public printable_code { // !
    exclamation() : printable_code('!') {}
};
struct at : public printable_code { // @
    at() : printable_code('@') {}
};
struct hashtag : public printable_code { // #
    hashtag() : printable_code('#') {}
};
struct dollar : public printable_code { // $
    dollar() : printable_code('$') {}
};
struct percent : public printable_code { // %
    percent() : printable_code('%') {}
};
struct caret : public printable_code { // ^
    caret() : printable_code('^') {}
};
struct ampersand : public printable_code { // &
    ampersand() : printable_code('&') {}
};

template <typename _Tp> auto handle(fsm::context<_Tp>& _f, char _c) {
    if (isprint(_c)) { // printable code
        if (isdigit(_c)) {
            return _f.handle(digit(_c));
        }
        if (islower(_c)) {
            return _f.handle(lower_case(_c));
        }
        if (isupper(_c)) {
            return _f.handle(upper_case(_c));
        }
        switch (_c) {
            case '+' : return _f.handle(plus());
            case '-' : return _f.handle(minus());
            case '*' : return _f.handle(asterisk());
            case '/' : return _f.handle(slash());
            case '=' : return _f.handle(assignment());
            case '.' : return _f.handle(dot());
            case ',' : return _f.handle(comma());
            case '|' : return _f.handle(vertical());
            case '_' : return _f.handle(underline());
            case '<' : return _f.handle(left_angle());
            case '>' : return _f.handle(right_angle());
            case '(' : return _f.handle(left_parentheses());
            case ')' : return _f.handle(right_parentheses());
            case '[' : return _f.handle(left_square());
            case ']' : return _f.handle(right_square());
            case '{' : return _f.handle(left_curly());
            case '}' : return _f.handle(right_curly());
            case '\'' : return _f.handle(single_quote());
            case '\"' : return _f.handle(double_quote());
            case '`' : return _f.handle(back_quote());
            case '~' : return _f.handle(tilde());
            case '\\' : return _f.handle(backslash());
            case '?' : return _f.handle(question());
            case ':' : return _f.handle(colon());
            case ';' : return _f.handle(semicolon());
            case '!' : return _f.handle(exclamation());
            case '@' : return _f.handle(at());
            case '#' : return _f.handle(hashtag());
            case '$' : return _f.handle(dollar());
            case '%' : return _f.handle(percent());
            case '^' : return _f.handle(caret());
            case '&' : return _f.handle(ampersand());
        }
        return _f.handle(printable_code(_c));
    }
    else { // control code
        return _f.handle(control_code(_c));
    }
    return _f.handle(ascii_code(_c));
}

}

}

}

#endif // _ICY_FINITE_STATE_MACHINE_HPP_