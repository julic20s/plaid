# plaid

[简体中文](README.md)

🚧🚧🚧 UNFINISHED 🚧🚧🚧

plaid is a software renderer in C++。It is consist of three parts:
* `core` The rendering pipeline implementation.
* `json` Parse json to dom
* `viewer` Loading and displaying the model.

![Hello triangle!](screenshot/screenshot_triangle.png)

### Progress
* Programmable rendering pipeline
* Triangle rasterization
* Programmable vertex shader and fragment shader
* Common color format transition
* Render passes (untested)
* Parse a json to a dom
* Load simple `.obj`
* Move the surround camera
* WIN32 window

### TODO
* The code is so mess，I will try to improve the readability.
* Load from glTF
* Custom viewport
* MSAA
* A half-baked idea: using coroutine to implement ddx, ddy, and mipmap

### Environments
* CMake
* Windows SDK 10.0.17134.0 or higher
> `viewer` run on Windows only now，because my window system only written for win32 now.

### Build
```
cmake -B.build
cmake --build .build
```

> If there are any Environment issue/Compiling error/Bug, add it to issue，or send to: julic20s@outlook.com, please.
