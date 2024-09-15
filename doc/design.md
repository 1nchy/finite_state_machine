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
    virtual std::string_view handle(const event&) = 0;
    virtual void entry() = 0;
    virtual void exit() = 0;
    virtual void assign(const state& _s) {}
};
class derived_state : public state {
    derived_state& operator=(const derived_state&);
    virtual std::string_view handle(const derived_event1&) = 0;
    virtual std::string_view handle(const derived_event2&) = 0;
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
    std::string_view _state = ""; // 当前状态的键
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
template <typename _Tp> requires std::is_base_of<state, _Tp>::value class context {
    typedef context<_Tp> self;
public:
    // derived from fsm::state
    typedef _Tp state_type;
    context() = default;
    context(const self&) = delete;
    self& operator=(const self&) = delete;
    ~context() = default;
    template <typename _Et>
        requires std::is_base_of<event, _Et>::value
        void handle(const _Et&);
private:
    std::string_view _state = ""; // 当前状态的键
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
    virtual std::string_view handle(const event&) = 0;
    virtual std::string_view transit(state* const) const = 0;
};
~~~

我们这里先讨论一下 `state::handle` 的返回值，包括空字符串和非空字符串。
非空字符串较易理解，即 `handle` 显式的指定转移状态。
若返回空字符串，则表示将状态转移交给 `transit` 方法决定。

其次是 `state::transit` 的返回值，和上面一样，包括空字符串和非空字符串。
非空字符串是显式指定的转移状态。
若返回空字符串，则理解为重入当前状态。

对于 `transit` 方法而言，我们需要传入当前状态指针——否则我们在父类的 transit 函数中，无法返回当前状态。

对于错误的输入事件来说，上面两个函数可以用异常处理的方式实现。

同时，context 类中也得对事件处理函数的定义与实现做出修改：

~~~cpp
template <typename _Tp> template <typename _Et>
requires std::is_base_of<event, _Et>::value
auto context<_Tp>::handle(const _Et& _e) -> bool {
    try {
        std::string_view _ns = _M_state()->handle(_e);
        if (_ns.empty()) {
            _ns = _M_state()->transit(_M_state());
            // if (_ns.empty()) return true;
        }
        _M_transit(_ns);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}
~~~

## 状态机退出

[前面](#状态的条件转移)提到，当 `state::transit` 函数抛出异常时，表示状态机走到了终点。
从实践角度来说，原因可能是：转移到了无出度的（结束或错误）状态、接收了无法处理的事件。
具体情况依赖于 `state::handle` 及 `state::transit` 函数的实现。
但无论哪种情况，我们都需要对当前状态进行可收受检测，检测当前状态是否是可接受的结束状态。

因此我们给出以下接口：

~~~cpp
template <typename _Tp> requires std::is_base_of<state, _Tp>::value class context {
public:
    // derived from fsm::state
    typedef _Tp state_type;
    template <typename _St>
        requires std::is_base_of<state_type, _St>::value
        void accept();
    template <typename _St>
        requires std::is_base_of<state_type, _St>::value
        void reject();
    bool acceptable() const;
    const state_type* state() const;
private:
    std::unordered_set<std::string_view> _acceptable_states;
};
~~~

用于检测当前状态是否属于可接受状态列表。