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
    typedef state self;
    state() = default;
public:
    state(const self&) = delete;
    self& operator=(const self&) = delete;
    ~state() = default;
};
~~~

state 作为状态基类，定义的所有状态都应该继承于此。

状态处理事件后，将转移至何种状态，应该由状态自己决定（而不是有限状态机或事件决定）。
但状态自身不应该主动调用有限状态机中的状态切换函数，
因此转移状态应该以返回值的形式传递给有限状态机。

由此可得出一个阶段性结论，各状态应该以单例模式出现，state 中应定义 instance 方法以访问各状态单例。

我们把事件处理函数定义为

~~~cpp
virtual self* handle(const event&) = 0;
~~~

但这种定义无法满足我们分离各事件处理函数的需要——所有处理过程都将在该函数中，各状态将根据 event 的某虚函数方法区分各事件。

因此，我们的**各状态不能直接继承 state 类**，而应该继承定义了各类型事件处理函数的 state 子类。
这么做还有一个额外的好处，我们在 有限状态机定义 章节中展开。

~~~cpp
class state {
    // omitted
public:
    virtual self* handle(const event&) = 0;
    virtual void entry() = 0;
    virtual void exit() = 0;
    template <typename _Tp>
        requires std::is_base_of<state, _Tp>::value
        static _Tp* instance();
};
class derived_state : public state {
    virtual self* handle(const derived_event1&) = 0;
    virtual self* handle(const derived_event2&) = 0;
};
class state1 : public derived_state {};
class state2 : public derived_state {};
~~~

## 有限状态机定义

一个容易想到的有限状态机的定义如下：

~~~cpp
class context {
    context() = default;
public:
    context(const context&) = delete;
    context& operator=(const context&) = delete;
    ~context() = default;
    static context* instance();
    void handle(const event& _e);
private:
    typename state* _state = nullptr; // 指向当前状态的单例
};
~~~

该定义存在两个问题，一是 context::handle 向 state::handle 转递事件时，一定会匹配到基类事件处理函数，各状态的各事件处理函数无法匹配到；二是状态机与状态几乎完全脱离，甚至其他状态机的状态也可以放在当前状态机中。

前面我们推导“各状态不能直接继承 state 类”结论时，是从事件处理函数的角度出发的。
这里我们从有限状态机的角度再次分析一下。

如果各状态直接继承自 state 类，那么状态机与状态几乎就完全分离了，或者说，我们既没法约束状态机当前状态的类型，也无法从状态机获取除当前状态外其他状态的信息。

而让各状态继承定义了各类型事件处理函数的 state 子类 derived_state 的做法有个天然的好处——假如状态机的状态指针类型为 derived_state，那么就可以约束状态类型了。同时也避免了各个状态机混杂的问题。derived_state 作为一个独一无二的类，天然适合做有限状态机的形参。

因此，我们更改有限状态机的定义如下；

~~~cpp
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
private:
    state_type* _state = nullptr;
};
~~~