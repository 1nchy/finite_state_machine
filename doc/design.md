# DESIGN

有限状态机根据当前状态与事件，经过特定数据处理后，转移至可能的其他状态。

因此我们设计有限状态机时，需要考虑、状态以及有限状态机主体三种类型。

## 事件定义

~~~cpp
struct event {};
~~~

event 作为所有事件的基类，我们定义的所有事件都应该继承于此。

## 状态定义

~~~cpp
class state {
protected:
    state() = default;
public:
    state(const state&) = delete;
    state& operator=(const state&) = delete;
    ~state() = default;
};
~~~

`state` 作为状态基类，定义的所有状态都应该继承于此。

状态处理事件后，将转移至何种状态，应该由状态自己决定（而不是有限状态机或事件决定）。
但状态自身不应该主动调用有限状态机中的状态切换函数，
因此转移状态应该以返回值的形式传递给有限状态机。

倘若不使用单例设计模式的话，那么，各状态类应该有一个可以用于唯一标识的枚举/静态方法，因此我们给出了 `FSM_STATE_LABEL` 宏，为各状态类型提供了静态的 `label` 方法。
唯一标识的类型为 `std::string_view`

~~~cpp
#define FSM_STATE_LABEL \
static constexpr auto label() -> std::string_view { \
    std::string_view _name = std::source_location::current().function_name(); \
    size_t _first_colon = _name.rfind("::"); \
    size_t _space_after = _name.rfind(" ", _first_colon) + 1; \
    return _name.substr(_space_after, _first_colon - _space_after); \
}
~~~

我们把事件处理函数定义为

~~~cpp
virtual std::string_view handle(const event&) = 0;
~~~

但这种定义无法满足我们分离各事件处理函数的需要——所有处理过程都将在该函数中，各状态将根据 `event` 的某虚函数方法区分各事件。

因此，我们的**各状态不能直接继承 state 类**，而应该继承定义了各类型事件处理函数的 `state` 子类。
这么做还有一个额外的好处，我们在 [有限状态机定义](#有限状态机定义) 章节中展开。

~~~cpp
class state {
    // omitted
public:
    FSM_STATE_LABEL
    using label_type = std::invoke_result<decltype(&state::label)>::type;
    virtual label_type handle(const event&) = 0;
    virtual void assign(const state& _s) {}
    virtual void entry() {}
    virtual void exit() {}
};
class derived_state : public state {
    derived_state& operator=(const derived_state&);
    virtual label_type handle(const derived_event1&) = 0;
    virtual label_type handle(const derived_event2&) = 0;
    virtual void assign(const state& _s) override { // optional
        this->operator=(std::dynamic_cast<const derived_state&>(_s));
    }
};
class state1 : public derived_state {
    FSM_STATE_LABEL
};
class state2 : public derived_state {
    FSM_STATE_LABEL
};
~~~

`handle` 方法主要负责事件处理，至于状态转移，可以将部分工作移交给 `transit` 方法。
关于这部分的讨论，见 [状态的条件转移](#状态的条件转移) 章节。

`assign` 方法用于状态间某些必要数据的赋值，该方法的重写不是必要的。

### Concept 约束

我们分别定义两个 `concept` 用于约束状态类型。

~~~cpp
template <typename _Bt> concept basic_state = std::derived_from<_Bt, state>;
template <typename _Bt, typename _St> concept label_state = 
basic_state<_Bt> && std::derived_from<_St, _Bt> &&
requires { {_St::label()} -> std::same_as<state::label_type>; };
~~~

前者 `basic_state` 用于约束基础状态类型，它被要求必须继承自 `state` 基类。

后者 `label_state` 用于约束状态类型，它被要求必须继承自基础状态类型，并且包含用于生成唯一标识的静态成员方法 `label()`。

## 有限状态机定义

一个容易想到的有限状态机的定义如下：

~~~cpp
class context {
public:
    context() = default;
    context(const context&) = delete;
    context& operator=(const context&) = delete;
    ~context() = default;
    void handle(const event& _e);
private:
    state::label_type _state = {}; // 当前状态的键
};
~~~

该定义存在两个问题，
一是 `context::handle` 向 `state::handle` 转递事件时，一定会匹配到基类事件处理函数，各状态的各事件处理函数无法匹配到；
二是状态机与状态几乎完全脱离，甚至其他状态机的状态也可以放在当前状态机中。

前面我们推导“各状态不能直接继承 `state` 类”结论时，是从事件处理函数的角度出发的。
这里我们从有限状态机的角度再次分析一下。

如果各状态直接继承自 `state` 类，那么状态机与状态几乎就完全分离了，
或者说，我们既没法约束状态机当前状态的类型，也无法从状态机获取除当前状态外其他状态的信息。

而让各状态继承定义了各类型事件处理函数的 `state` 子类 `derived_state` 的做法有个天然的好处——
假如状态机的状态指针类型为 `derived_state`，那么就可以约束状态类型了。
同时也避免了各个状态机混杂的问题。`derived_state` 作为一个独一无二的类，天然适合做有限状态机的形参。

因此，我们更改有限状态机的定义如下；

~~~cpp
template <basic_state _Bs> class context {
    typedef context<_Bs> self;
public:
    // derived from fsm::state
    typedef _Bs state_type;
    context() = default;
    context(const self&) = delete;
    self& operator=(const self&) = delete;
    ~context() = default;
    template <typename _Et> requires std::derived_from<_Et, event>
        void handle(const _Et&);
private:
    state::label_type _state = {}; // 当前状态的键
};
~~~

## 状态的条件转移

除开部分事件直接引发的状态转移，状态机中还有一种很常见的条件转移。
若所有状态的转移均在 `state::handle` 函数中实现，其实是有些耦合的。
`handle` 函数中既包含了事件处理，还包含了对条件的判断。
而且多种状态可能有相同的条件转移逻辑，都放在 `handle` 函数中会导致代码冗余。

> 例如状态中保存了某计数器，当计数器到达某个值的时候，出发条件转移。
>
> 但倘若每个事件都会引起计数器的修改，那在每个事件的处理函数下，都将加上同样的条件判断代码。
>
> 倘若各状态对相同的引起计数器修改的事件的处理方式不同，那代码的冗余还将继续增加。

因此我们将条件判断的代码抽离出来，在 `state` 中定义抽象方法 `transit` 来实现条件转移，并修改了 `handle` 的预期返回。

~~~cpp
class state {
public:
    virtual label_type handle(const event&) = 0;
    virtual label_type transit() const = 0;
};
~~~

我们这里先讨论一下 `state::handle` 的返回值，
面对不同返回值，我们总共有三种处理方式：直接进行状态转移；将状态转移交给 `transit` 方法决定；报错。
三种方式对应着 `_St::label()`、空 `label` 和非法 `label`。

我们把基类标识符作为非法标识符，即 `state::label()`。

其次是 `state::transit` 的返回值，和上面一样，我们总共有三种处理方式：直接进行状态转移；重入当前状态；报错。
三种方式对应着 `_St::label()`、空 `label` 和非法 `label`。

~~对于 `transit` 方法而言，我们需要传入当前状态指针——否则我们在父类的 transit 函数中，无法返回当前状态。~~

在以前的设计中（特指单例模式中），需要 `transit` 返回当前状态指针，才能实现状态的重入，因为空指针被用于做错误处理了。

~~对于错误的输入事件来说，上面两个函数可以用异常处理的方式实现。~~

后来取消单例设计模式时，没有考虑到使用 `state::label()` 作为非法 `label`，而使用了略微影响性能的异常来处理状态错误。随着我对异常机制了解的深入，以及结合实际使用场景，决定不再使用异常来进行错误处理。

> 这里的实际使用场景主要指：
>
> 1. 未来可能进行多线程安全化的改造，异常机制在多线程环境中开销较大。
> 2. 异常率较高，特别是以 有限自动机 形态进行工作时（例如进行按正则规则解析字符串，可能没读几个字符就该结束了，触发状态机报错），这会带来额外的开销。

同时，context 类中也得对事件处理函数的定义与实现做出修改：

~~~cpp
// in context<_Bs>:
template <typename _Et> requires std::derived_from<_Et, event>
auto handle(const _Et& _e) -> bool {
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
~~~

## 状态机退出

[前面](#状态的条件转移)提到，当 `state::transit` 函数返回非法标识符时，表示状态机走到了终点。
从实践角度来说，原因可能是：转移到了无出度的（结束或错误）状态、接收了无法处理的事件。
具体情况依赖于 `state::handle` 及 `state::transit` 函数的实现。
但无论哪种情况，我们都需要对当前状态进行可收受检测，检测当前状态是否是可接受的结束状态。

因此我们给出以下接口：

~~~cpp
template <basic_state _Tp> requires std::is_base_of<state, _Tp>::value class context {
public:
    // derived from fsm::state
    typedef _Tp state_type;
    template <typename _St> requires label_state<_Bs, _St> void accept();
    template <typename _St> requires label_state<_Bs, _St> void reject();
    bool acceptable() const;
    const state_type* state() const;
private:
    std::unordered_set<state::label_type> _acceptable_states;
};
~~~

用于检测当前状态是否属于可接受状态列表。