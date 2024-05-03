# USAGE

## 需求分析

TCP 拥塞控制中，有以下几种事件：

1. new ack
2. duplicate ack
3. timeout

有以下三种状态：

1. slow start（慢启动）
2. congestion control（拥塞避免）
3. fast recovery（快速恢复）

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
    virtual self* handle(const fsm::event&) = 0;
    virtual self* handle(const new_ack&) = 0;
    virtual self* handle(const duplicate_ack&) = 0;
    virtual self* handle(const timeout&) = 0;
    void entry() override {}
    void exit() override {}
};
~~~

并定义各状态类，继承自 tcp_congestion_state 类型。

~~~cpp
struct slow_start : public tcp_congestion_state {
    using fsm::state::self;
    self* handle(const fsm::event& _e) override;
    self* handle(const new_ack& _e) override;
    self* handle(const duplicate_ack& _e) override;
    self* handle(const timeout& _e) override;
    void entry() override;
    void exit() override;
};
struct congestion_avoidance : public tcp_congestion_state {
    using fsm::state::self;
    self* handle(const fsm::event& _e) override;
    self* handle(const new_ack& _e) override;
    self* handle(const duplicate_ack& _e) override;
    self* handle(const timeout& _e) override;
    void entry() override;
    void exit() override;
};
struct fast_recovery : public tcp_congestion_state {
    using fsm::state::self;
    self* handle(const fsm::event& _e) override;
    self* handle(const new_ack& _e) override;
    self* handle(const duplicate_ack& _e) override;
    self* handle(const timeout& _e) override;
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

## 状态的事件处理

这里仅以 slow_start 为例。

~~~cpp
auto slow_start::handle(const fsm::event& _e) -> self* {
    printf("message from slow_start\n");
    this->dispatch<tcp_congestion_state>(timeout());
    return nullptr;
}
auto slow_start::handle(const new_ack& _e) -> self* {
    _dup_ack_count = 0; _cwnd += _MSS;
    printf("transmit new segment.\n");
    if (_cwnd >= _ssthresh) {
        return fsm::state::instance<congestion_avoidance>();
    }
    return this;
}
auto slow_start::handle(const duplicate_ack& _e) -> self* {
    ++_dup_ack_count;
    if (_dup_ack_count == 3) {
        return fsm::state::instance<fast_recovery>();
    }
    return this;
}
auto slow_start::handle(const timeout& _e) -> self* {
    _ssthresh = _cwnd / 2; _cwnd = _MSS; _dup_ack_count = 0;
    printf("retransmit new segment.\n");
    return this;
}
auto slow_start::entry() -> void {
    printf("entry slow_start.\n");
    this->show();
}
auto slow_start::exit() -> void {
    this->show();
    printf("exit slow_start.\n");
}
~~~

返回 nullptr 表示不改变状态，返回 this 表示返回当前状态（多数情况下与 nullptr 效果一致）。

dispatch 方法用于向指定的有限状态机发送事件，注意，该方法可能间接导致状态发生改变，因此建议在使用该方法后，返回 nullptr。

具体实现见 [tcp_state.cpp](../../example/tcp_congestion_control/src/tcp_state.cpp)。