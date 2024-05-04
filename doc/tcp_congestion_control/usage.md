# USAGE

## 需求分析

TCP 拥塞控制中，有以下几种事件：

1. new ack
2. duplicate ack
3. timeout

> 遇到 timeout 事件时，各状态的处理方式都是一致的。

有以下三种状态：

1. slow start（慢启动）
2. congestion control（拥塞避免）
3. fast recovery（快速恢复）

关于状态转移：

1. 当遇到 dupACKcount == 3 时，各状态的转移方式一致。
2. 当处于 slow start 状态时，需要对 cwnd $\ge$ ssthresh 做额外的状态转移。

## 事件定义

~~~cpp
struct new_ack : public fsm::event {};
struct duplicate_ack : public fsm::event {};
struct timeout : public fsm::event {};
~~~

具体定义详情见 [tcp_event.hpp](../../example/tcp_congestion_control/include/tcp_event.hpp)。

## 状态定义

在状态定义中，我们首先定义声明了各事件处理函数的 state 子类 tcp_congestion_state。

~~~cpp
struct tcp_congestion_state : public fsm::state {
    using fsm::state::self;
    virtual self* handle(const fsm::event&) override = 0;
    virtual self* handle(const new_ack&) override = 0;
    virtual self* handle(const duplicate_ack&) override = 0;
    virtual self* handle(const timeout&) override;
    virtual self* transit() const override;
    void entry() override {}
    void exit() override {}
};
~~~

由于各状态对 timeout 事件的处理方式一致，我们便把该事件的处理函数放在这里实现。

在此基础上，我们定义各状态类，继承自 tcp_congestion_state 类型。

~~~cpp
struct slow_start : public tcp_congestion_state {
    using fsm::state::self;
    self* handle(const fsm::event& _e) override;
    self* handle(const new_ack& _e) override;
    self* handle(const duplicate_ack& _e) override;
    self* transit() const override;
    void entry() override;
    void exit() override;
};
struct congestion_avoidance : public tcp_congestion_state {
    using fsm::state::self;
    self* handle(const fsm::event& _e) override;
    self* handle(const new_ack& _e) override;
    self* handle(const duplicate_ack& _e) override;
    void entry() override;
    void exit() override;
};
struct fast_recovery : public tcp_congestion_state {
    using fsm::state::self;
    self* handle(const fsm::event& _e) override;
    self* handle(const new_ack& _e) override;
    self* handle(const duplicate_ack& _e) override;
    void entry() override;
    void exit() override;
};
~~~

具体定义见 [tcp_state.hpp](../../example/tcp_congestion_control/include/tcp_state.hpp)。

具体实现见 [tcp_state.cpp](../../example/tcp_congestion_control/src/tcp_state.cpp)。

## 有限状态机定义与使用

获取以 tcp_congestion_state 为基类的状态的有限状态机。

~~~cpp
auto* const _fsm = fsm::context<tcp_congestion_state>::instance()
~~~

- 有限状态机状态初始化

将有限状态机状态初始化为 slow_start。

~~~cpp
_fsm->initialize<slow_start>();
~~~

- 有限状态机接收事件

~~~cpp
_fsm->handle(fsm::event());
_fsm->handle(new_ack());
_fsm->handle(duplicate_ack());
_fsm->handle(timeout());
~~~

## 状态转移的实现

前面提到各状态对于 dupACKcount == 3 的处理方式是一致的，因此把该状态条件转移放在 tcp_congestion_state 类中。
而 slow_start 状态还有需要额外考虑的条件转移，该部分的实现应该放在 slow_start 类中。
需要注意的是最后需要**返回父类的条件转移函数**。

~~~cpp
auto tcp_congestion_state::transit() const -> self* {
    if (_dup_ack_count >= 3) {
        auto* const _ret = fsm::state::instance<fast_recovery>();
        _ret->_ssthresh = _cwnd / 2;
        _ret->_cwnd = _ret->_ssthresh + 3 * _MSS;
        _ret->_dup_ack_count = 0;
        return _ret;
    }
    return nullptr;
}
auto slow_start::transit() const -> self* {
    if (_cwnd >= _ssthresh) {
        auto* const _ret = fsm::state::instance<congestion_avoidance>();
        _ret->clone(this);
        return _ret;
    }
    return tcp_congestion_state::transit();
}
~~~

## 状态的事件处理

这里仅以 slow_start 相关的事件处理为例。

~~~cpp
auto tcp_congestion_state::handle(const timeout& _e) -> self* {
    auto* const _ret = fsm::state::instance<slow_start>();
    _ret->_ssthresh = _cwnd / 2;
    _ret->_cwnd = _MSS;
    _ret->_dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return _ret;
}
auto slow_start::handle(const fsm::event& _e) -> self* {
    printf("message from slow_start\n");
    this->dispatch<tcp_congestion_state>(timeout());
    return nullptr;
}
auto slow_start::handle(const new_ack& _e) -> self* {
    _dup_ack_count = 0; _cwnd += _MSS;
    printf("transmit new segment.\n");
    return nullptr;
}
auto slow_start::handle(const duplicate_ack& _e) -> self* {
    ++_dup_ack_count;
    return nullptr;
}
~~~

返回 nullptr 表示不改变状态，返回 this 表示返回当前状态（多数情况下与 nullptr 效果一致）。

dispatch 方法用于向指定的有限状态机发送事件，注意，该方法可能间接导致状态发生改变，因此建议在使用该方法后，返回 nullptr。

具体实现见 [tcp_state.cpp](../../example/tcp_congestion_control/src/tcp_state.cpp)。