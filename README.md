# plaid

[ENGLISH](README-EN.md)

🚧🚧🚧 未完成 🚧🚧🚧

plaid 是一个 C++ 软光栅渲染器。它由三个部分组成:
* `core` 渲染管线框架实现。
* `json` 解析 json 到 dom
* `viewer` 加载并渲染模型。

![Hello triangle!](screenshot/screenshot_triangle.png)

### 已完成
* 可定制渲染管线
* 三角形光栅化
* 可编程顶点着色器和片元着色器
* 常见颜色内存布局转换
* 多通道渲染（未测试）
* 解析 json 到 dom
* 加载简单的 `.obj`
* 移动环绕相机
* WIN32 窗口

### TODO
* 代码很乱可读性很差，慢慢优化
* 加载 glTF
* 自定义 viewport
* MSAA
* 一个不成熟的想法：用 coroutine 来实现 ddx, ddy 和 mipmap

### 环境需求
* CMake
* Windows SDK 10.0.17134.0 or higher
> `viewer` 现在只能在 Windows 上运行，原因是目前创建窗口只写了 win32 版本的

### 构建
```
cmake -B.build
cmake --build .build
```

> 环境配置/编译问题/Bug 请直接提 issue，或者发我邮箱: julic20s@outlook.com
