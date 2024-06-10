# USAGE

## 需求分析

为表示 float 浮点数，我们需要定义若干正则表达式规则：

|类型|正则表达式|
|:-:|:-:|
|$digit$|[0-9]|
|$unsigned$|$digit^+$|
|$opt\_fraction$|$('.'unsigned)?$|
|$opt\_exponent$|$('e'('+'+'-')?unsigned)?$|
|$float$|$('+'+'-')?unsigned\ opt\_fraction\ opt\_exponent$|


- float NFA (from A to G)
~~~mermaid
graph LR
A(("A")) --> B(("B")) --"0-9"--> C(("C")) --"."--> D(("D")) --"0-9"--> E(("E")) --> F(("F")) --"e"--> G(("G")) --> H(("H")) --"0-9"--> I(("I")) --> J((("J")))
A --"+"--> B
A --"-"--> B
C --> B
C --> F
E --> D
F --> J
I --> H
G --"+"--> H
G --"-"--> H
~~~

- float DFA (from AB)
~~~mermaid
graph LR
AB(("AB")) --"0-9"--> BCFJ((("BCFJ"))) --"e"--> GH(("GH")) --"0-9"--> HIJ((("HIJ")))
AB --"+-"--> B(("B")) --"0-9"--> BCFJ
BCFJ --"0-9"--> BCFJ
BCFJ --"."--> D(("D")) --"0-9"--> DEFJ((("DEFJ"))) --"e"--> GH
DEFJ --"0-9"--> DEFJ
GH --"+-"--> H(("H")) --"0-9"--> HIJ
HIJ --"0-9"--> HIJ
~~~

## 事件定义

~~~cpp
struct digit : public fsm::event {}; // 0-9
struct dot : public fsm::event {}; // .
struct alpha : public fsm::event { // a-z A-Z
    alpha(char _c) : _c(_c) {}
    const char _c;
};
struct sign : public fsm::event {}; // + -
~~~

## 状态定义

我们前面分析了浮点数解析的有限自动机模型，针对各节点定义一个状态即可。对于输入字符集之外的输入，我们统一在 `handle(const fsm::event&)` 里面处理。另外，多数状态仅有一到两种事件需要处理，而其他的意外事件对于当前状态来说均是非法输入，因此对所有事件都默认使用 `handle(const fsm::event&)` 方法处理。

~~~cpp
struct float_recognition_state : public fsm::state {
    typedef fsm::state::self self;
    float_recognition_state& operator=(const float_recognition_state&);
    virtual self* handle(const fsm::event&) override;
    virtual self* handle(const digit&);
    virtual self* handle(const dot&);
    virtual self* handle(const alpha&);
    virtual self* handle(const sign&);
    virtual self* transit(self* const) const override;
    void entry() override {}
    void exit() override {}
    self* clone(const float_recognition_state* const);
    size_t length() const { return _length; }
    size_t _length = 0;
    bool _end_of_float = false;
};
~~~

注意到 `BCFJ`、`DEFJ`、`HIJ` 三种状态是可接受的结束状态，因此需要显式的声明这三种状态为可接受的。

~~~cpp
auto* const _fsm = fsm::context<float_recognition_state>::instance();
_fsm->accept<BCFJ>();
_fsm->accept<DEFJ>();
_fsm->accept<HIJ>();
~~~