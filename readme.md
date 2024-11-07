Layout 布局
======

一个简单且快速的堆叠式盒布局库。它适用于计算 2D 用户界面等的布局。该库可以以 C99 或 C++ 编译，并已通过 gcc (mingw64)、VS2015 和 clang/LLVM 测试。您只需一个文件即可在自己的项目中使用：[layout.h](layout.h)。

![](https://raw.githubusercontent.com/wiki/randrew/layoutexample/ui_anim_small.gif)

Use Layout in Your Project 在项目中使用
--------------------------

要在您自己的项目中使用 *Layout*，请将 [layout.h](layout.h) 复制到项目的源代码目录中，并在包含 [layout.h](layout.h) 的一个 `.c` 或 `.cpp` 文件中定义 `LAY_IMPLEMENTATION`。

您需要在这个特定文件中按以下方式包含内容：

```C
#include ...
#include ...
#define LAY_IMPLEMENTATION
#include "layout.h"
```

项目中的其他文件不应定义 `LAY_IMPLEMENTATION`，并可以像普通头文件一样包含 [layout.h](layout.h)。

Requirements and Extras 依赖项
-----------------------

*Layout* 没有外部依赖项，但默认情况下会使用 `assert.h`、`stdlib.h` 和 `string.h` 中的 `assert`、`realloc` 和 `memset`。如果您的项目不使用或无法使用这些库，您可以通过预处理器定义轻松将它们替换为其他内容。详情请参阅下文的 [自定义选项](#customizations) 部分。

*Layout* 可以使用 C99 或 C++ 进行构建（如果使用的是 GCC 或 Clang），但如果使用 MSVC，则必须以 C++ 构建。之所以有此要求，是因为 *Layout* 实现代码在 GCC 和 Clang 中使用了 `vector_size` 扩展，而 MSVC 中没有该功能。在 MSVC 中，通过 C++ 的操作符重载来模拟此功能。三种编译器生成的代码差别不大——`vector_size` 扩展主要用于保持实现的语法易于阅读。

*Layout* 基于 [duangle](https://twitter.com/duangle) 开发的优秀库 [oui](https://bitbucket.org/duangle/oui-blendish)。与 *oui* 不同，*Layout* 不处理与用户输入、焦点或 UI 状态相关的任何内容。

可以通过 [tool.bash](tool.bash) 脚本或 [GENie](https://github.com/bkaradzic/GENie) 构建测试和基准。请参阅下文的 [构建测试和基准](#building-the-tests-and-benchmarks) 部分。此外，还提供了一个将 *Layout* 作为 Lua `.dll` 模块使用的示例。不过，在您自己的项目中使用 *Layout* 时无需依赖这些内容。

Options 构建选项
-------

您可以选择构建 *Layout* 以使用整数（int16）或浮点数（float）作为坐标。默认使用整数，因为 UI 和其他 2D 布局在对齐和定位元素时通常不会使用小于一个像素的单位。您可以通过定义 `LAY_FLOAT` 来选择使用浮点数而不是整数。

* 当定义了 `LAY_FLOAT` 时，将使用 `float` 而不是 `int16` 作为坐标类型。

除了 `LAY_FLOAT` 预处理选项，还可以通过设置其他预处理器定义来自定义 *Layout* 的行为。未定义的选项将使用默认行为。

* `LAY_ASSERT` 可替代 `assert.h` 中 `assert` 的使用
* `LAY_REALLOC` 可替代 `stdlib.h` 中 `realloc` 的使用
* `LAY_MEMSET` 可替代 `string.h` 中 `memset` 的使用

如果您定义了 `LAY_REALLOC`，还需要定义 `LAY_FREE`。

Example 示例
=======

```C
// LAY_IMPLEMENTATION 需要在一个包含 layout.h 的 .c 或 .cpp 文件中定义。
// 其他所有文件不应定义它。

#define LAY_IMPLEMENTATION
#include "layout.h"

// 假设我们正在创建某种 GUI，左侧是主列表，右侧是内容视图。

// 首先，我们需要一个这样的上下文
lay_context ctx;

// 然后，我们需要初始化它
lay_init_context(&ctx);

// 上下文会在使用过程中自动调整堆缓冲区的大小。
// 但我们可以通过预先保留足够的空间来避免多次重新分配。
// 不用担心，lay_init_context 不做任何分配，所以这是我们第一次也是唯一的分配。
lay_reserve_items_capacity(&ctx, 1024);

// 创建我们的根项。项就是 2D 盒子。
lay_id root = lay_item(&ctx);

// 假设我们有一个已知尺寸的游戏窗口或操作系统窗口。
// 我们希望显式地设置根项的尺寸为该大小。
lay_set_size_xy(&ctx, root, 1280, 720);

// 设置根项按插入顺序将子项排列为一行，从左到右。
lay_set_contain(&ctx, root, LAY_ROW);

// 创建主列表项。
lay_id master_list = lay_item(&ctx);
lay_insert(&ctx, root, master_list);
// 我们的主列表有一个固定的宽度，但希望它填满所有可用的垂直空间。
lay_set_size_xy(&ctx, master_list, 400, 0);
// 设置该项在其父项内的行为为填充所有可用的垂直空间。
lay_set_behave(&ctx, master_list, LAY_VFILL);
// 并且设置它按插入顺序将子项排列为一列，从上到下。
lay_set_contain(&ctx, master_list, LAY_COLUMN);

lay_id content_view = lay_item(&ctx);
lay_insert(&ctx, root, content_view);
// 内容视图只是想填充所有剩余的空间，所以我们不需要设置任何尺寸。

// 我们可以直接在这里设置 LAY_FILL，而不是通过按位或操作 LAY_HFILL 和 LAY_VFILL，
// 但我想演示如何组合标志。
lay_set_behave(&ctx, content_view, LAY_HFILL | LAY_VFILL);

// 通常在此时，我们可能希望创建主列表和内容视图的项并插入它们。
// 这是一个简单的示例，接下来让我们继续完成剩下的部分。

// 运行上下文 -- 这会执行所有实际的计算。
lay_run_context(&ctx);

// 现在我们可以获取项的计算结果，作为 2D 矩形。
// 向量的四个分量表示左上角的 x 和 y 坐标，然后是宽度和高度。
lay_vec4 master_list_rect = lay_get_rect(&ctx, master_list);
lay_vec4 content_view_rect = lay_get_rect(&ctx, content_view);

// master_list_rect  == {  0, 0, 400, 720}
// content_view_rect == {400, 0, 880, 720}

// 如果我们使用的是即时模式图形库，现在可以使用它绘制我们的盒子。
my_ui_library_draw_box_x_y_width_height(
    master_list_rect[0],
    master_list_rect[1],
    master_list_rect[2],
    master_list_rect[3]);

// 您也可以使用 lay_first_child 和 lay_next_sibling 等方法递归遍历整个项层次结构。

// 在调用 lay_run_context 之后，除非发生重新分配，否则结果应保持有效。
//
// 然而，虽然您可以手动更新上下文中的现有项，
// 通过使用 lay_set_size{_xy}，然后再次调用 lay_run_context，
// 但您可能想考虑每帧重新从头开始构建所有内容。
// 这比繁琐的细粒度失效处理更容易编程，
// 即使上下文中有成千上万的项，通常也只需要几微秒。

// 目前无法移除项 -- 一旦创建并插入，项就固定了。
// 如果我们想重置上下文，以便重新从头开始构建布局树，可以使用 lay_reset_context：

lay_reset_context(&ctx);

// 现在我们可以从头开始创建根项，插入更多项等。
// 我们不创建新的上下文是因为我们想重复使用已经分配的缓冲区。

// 假设我们正在关闭程序 -- 我们需要销毁上下文。
lay_destroy_context(&ctx);

// 堆分配的缓冲区现在已经释放。
// 在调用 lay_init_context 重新初始化之前，当前上下文不可再使用。
```

Building the Tests and Benchmarks 构建测试
=================================

ⓘ | 这些内容在您的项目中并不需要使用。这些说明仅适用于构建测试和基准程序，您可能不太关心这些内容。  
:---: | :---

如果您使用的是 POSIX 系统并且拥有 bash，您可以使用 `tool.bash` 脚本来构建 *Layout* 的独立测试和基准程序。运行 `tool.bash` 查看可用的选项。

<h3>使用 GENie</h3>

如果不想使用 `tool.bash` 脚本，您可以使用 GENie 来生成 Visual Studio 项目文件，或其支持的其他项目和构建系统输出类型。GENie 生成器还可以让您构建示例的 Lua 模块。

<details>
<summary>获取和使用 GENie 的说明</summary>

[genie.lua](genie.lua) 脚本主要在 Windows 上进行了测试，因此如果在其他平台上使用，可能需要进行一些调整。

您需要首先获取（或制作）一个 GENie 可执行文件，并将其放置在您的路径中，或放在本仓库的根目录下。

Download GENie
--------------

Linux:  
https://github.com/bkaradzic/bx/raw/master/tools/bin/linux/genie

OSX:  
https://github.com/bkaradzic/bx/raw/master/tools/bin/darwin/genie

Windows:  
https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe

Visual Studio 2015/2017
-----------------------

```
genie.exe vs2015
start build/vs2015/layout.sln
```

Replace vs2015 with vs2017 if you want to use Visual Studio 2017.

GCC/MinGW/Clang
---------------

```
./genie gmake
```

and then run your `make` in the directory `build/gmake`. You will need to
specify a target and config. Here is an example for building the `tests` target
in Windows with the 64-bit release configuration using mingw64 in a bash-like
shell (for example, git bash):


```
./genie.exe gmake && mingw32-make.exe -C build/gmake tests config=release64
```

If you want to use float coordinates instead of integer, you can use an option in the build script which will define `LAY_FLOAT` for you:

```
./genie gmake --coords=float
```

or if you want to specify integer (the default):

```
./genie gmake --coords=integer
```

</details>
