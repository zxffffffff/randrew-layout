#ifndef LAY_INCLUDE_HEADER
#define LAY_INCLUDE_HEADER

// 在项目中的一个 C 或 C++ 文件中定义 LAY_IMPLEMENTATION，然后在该文件中包含 layout.h。
// 你的包含语句应该如下所示：
//
//   #include ...
//   #include ...
//   #define LAY_IMPLEMENTATION
//   #include "layout.h"
//
// 项目中的其他文件不应该定义 LAY_IMPLEMENTATION。

#include <stdint.h>

#ifndef LAY_EXPORT
#define LAY_EXPORT extern
#endif

// 如果用户希望使用自定义的断言，可以定义 LAY_ASSERT。
// 否则，默认使用 assert.h 中的断言。
#ifndef LAY_ASSERT
#include <assert.h>
#define LAY_ASSERT assert
#endif

// 'static inline' 用于我们希望始终内联的代码 -- 编译器不应考虑不进行内联。
#if defined(__GNUC__) || defined(__clang__)
#define LAY_STATIC_INLINE __attribute__((always_inline)) static inline
#elif defined(_MSC_VER)
#define LAY_STATIC_INLINE __forceinline static
#else
#define LAY_STATIC_INLINE inline static
#endif

typedef uint32_t lay_id;
#if LAY_FLOAT == 1
typedef float lay_scalar;
#else
typedef int16_t lay_scalar;
#endif

#define LAY_INVALID_ID UINT32_MAX

// GCC 和 Clang 允许我们使用 vector_size 扩展创建基于某个类型的向量。
// 这使我们可以通过索引操作访问向量的各个组件。
#if defined(__GNUC__) || defined(__clang__)

#ifdef LAY_FLOAT
// 使用浮动类型坐标比使用 int16 占用更多空间。一个四元素向量占用 128 位。
typedef float lay_vec4 __attribute__ ((__vector_size__ (16), aligned(4)));
typedef float lay_vec2 __attribute__ ((__vector_size__ (8), aligned(4)));
#else
// 整数版本使用 64 位存储四元素向量。
typedef int16_t lay_vec4 __attribute__ ((__vector_size__ (8), aligned(2)));
typedef int16_t lay_vec2 __attribute__ ((__vector_size__ (4), aligned(2)));
#endif // LAY_FLOAT

// 请注意，我们并不打算显式使用任何平台的 SIMD 指令 -- 我们仅仅使用向量扩展提供更方便的语法。
// 因此，我们可以指定更宽松的对齐要求。有关此的说明请参见文件结尾。

// MSVC 没有 vector_size 属性，但我们希望为布局逻辑代码提供便捷的索引操作符。
// 因此，在 MSVC 中强制使用 C++ 编译，并使用 C++ 操作符重载。
#elif defined(_MSC_VER)
struct lay_vec4 {
    lay_scalar xyzw[4];
    const lay_scalar& operator[](int index) const
    { return xyzw[index]; }
    lay_scalar& operator[](int index)
    { return xyzw[index]; }
};
struct lay_vec2 {
    lay_scalar xy[2];
    const lay_scalar& operator[](int index) const
    { return xy[index]; }
    lay_scalar& operator[](int index)
    { return xy[index]; }
};
#endif // __GNUC__/__clang__ or _MSC_VER

typedef struct lay_item_t {
    uint32_t flags;
    lay_id first_child;
    lay_id next_sibling;
    lay_vec4 margins;
    lay_vec2 size;
} lay_item_t;

typedef struct lay_context {
    lay_item_t *items;
    lay_vec4 *rects;
    lay_id capacity;
    lay_id count;
} lay_context;

// 传递给 lay_set_container() 的容器标志
typedef enum lay_box_flags {
    // flex-direction (bit 0+1)

    // left to right
    LAY_ROW = 0x002,
    // top to bottom
    LAY_COLUMN = 0x003,

    // model (bit 1)

    // free layout
    LAY_LAYOUT = 0x000,
    // flex model
    LAY_FLEX = 0x002,

    // flex-wrap (bit 2)

    // single-line
    LAY_NOWRAP = 0x000,
    // multi-line, wrap left to right
    LAY_WRAP = 0x004,


    // justify-content (start, end, center, space-between)
    // at start of row/column
    LAY_START = 0x008,
    // at center of row/column
    LAY_MIDDLE = 0x000,
    // at end of row/column
    LAY_END = 0x010,
    // insert spacing to stretch across whole row/column
    LAY_JUSTIFY = 0x018

    // align-items
    // 可以通过将 flex 容器放入布局容器中来实现，
    // 然后使用 LAY_TOP、LAY_BOTTOM、LAY_VFILL、LAY_VCENTER 等。
    // FILL 相当于拉伸

    // align-content (start, end, center, stretch)
    // 可以通过将 flex 容器放入布局容器中来实现，
    // 然后使用 LAY_TOP、LAY_BOTTOM、LAY_VFILL、LAY_VCENTER 等。
    // FILL 相当于拉伸；不支持 space-between。
} lay_box_flags;

// 传递给 lay_set_behave() 的子布局标志
typedef enum lay_layout_flags {
    // attachments (bit 5-8)
    // 当父项使用 LAY_LAYOUT 模式时完全有效
    // 在 LAY_FLEX 模式下部分有效

    // anchor to left item or left side of parent
    LAY_LEFT = 0x020,
    // anchor to top item or top side of parent
    LAY_TOP = 0x040,
    // anchor to right item or right side of parent
    LAY_RIGHT = 0x080,
    // anchor to bottom item or bottom side of parent
    LAY_BOTTOM = 0x100,
    // anchor to both left and right item or parent borders
    LAY_HFILL = 0x0a0,
    // anchor to both top and bottom item or parent borders
    LAY_VFILL = 0x140,
    // center horizontally, with left margin as offset
    LAY_HCENTER = 0x000,
    // center vertically, with top margin as offset
    LAY_VCENTER = 0x000,
    // center in both directions, with left/top margin as offset
    LAY_CENTER = 0x000,
    // anchor to all four directions
    LAY_FILL = 0x1e0,
    // When in a wrapping container, put this element on a new line. Wrapping
    // layout code auto-inserts LAY_BREAK flags as needed. See GitHub issues for
    // TODO related to this.
    //
    // Drawing routines can read this via item pointers as needed after
    // performing layout calculations.
    LAY_BREAK = 0x200
} lay_layout_flags;

enum {
    // these bits, starting at bit 16, can be safely assigned by the
    // application, e.g. as item types, other event types, drop targets, etc.
    // this is not yet exposed via API functions, you'll need to get/set these
    // by directly accessing item pointers.
    //
    // (In reality we have more free bits than this, TODO)
    //
    // TODO fix int/unsigned size mismatch (clang issues warning for this),
    // should be all bits as 1 instead of INT_MAX
    LAY_USERMASK = 0x7fff0000,

    // a special mask passed to lay_find_item() (currently does not exist, was
    // not ported from oui)
    LAY_ANY = 0x7fffffff
};

enum {
    // extra item flags

    // bit 0-2
    LAY_ITEM_BOX_MODEL_MASK = 0x000007,
    // bit 0-4
    LAY_ITEM_BOX_MASK       = 0x00001F,
    // bit 5-9
    LAY_ITEM_LAYOUT_MASK = 0x0003E0,
    // item has been inserted (bit 10)
    LAY_ITEM_INSERTED   = 0x400,
    // horizontal size has been explicitly set (bit 11)
    LAY_ITEM_HFIXED      = 0x800,
    // vertical size has been explicitly set (bit 12)
    LAY_ITEM_VFIXED      = 0x1000,
    // bit 11-12
    LAY_ITEM_FIXED_MASK  = LAY_ITEM_HFIXED | LAY_ITEM_VFIXED,

    // which flag bits will be compared
    LAY_ITEM_COMPARE_MASK = LAY_ITEM_BOX_MODEL_MASK
        | (LAY_ITEM_LAYOUT_MASK & ~LAY_BREAK)
        | LAY_USERMASK
};

LAY_STATIC_INLINE lay_vec4 lay_vec4_xyzw(lay_scalar x, lay_scalar y, lay_scalar z, lay_scalar w)
{
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__cplusplus)
    return (lay_vec4){x, y, z, w};
#else
    lay_vec4 result;
    result[0] = x;
    result[1] = y;
    result[2] = z;
    result[3] = w;
    return result;
#endif
}

// 在使用上下文之前调用此函数。
// 如果您想在调用 lay_destroy_context() 后再次使用此上下文，也必须调用此函数。
LAY_EXPORT void lay_init_context(lay_context *ctx);

// 为了容纳 `count` 个项而预留足够的堆内存，而不需要重新分配。
// 初始的 lay_init_context() 调用不会分配任何堆内存，
// 因此，如果您初始化了一个上下文并且之后调用此函数，并为将要创建的项指定足够大的数量，则不会发生进一步的重新分配。
LAY_EXPORT void lay_reserve_items_capacity(lay_context *ctx, lay_id count);

// 释放上下文使用的所有堆内存。
// 如果上下文没有调用 lay_init_context()，则不应调用此函数。
// 要在销毁上下文后重用它，您需要再次调用 lay_init_context()。
LAY_EXPORT void lay_destroy_context(lay_context *ctx);

// 清除上下文中的所有项，将其计数设置为 0。
// 当您想要从根项重新声明布局时使用此函数。此操作不会释放任何内存或执行分配。
// 调用此函数后可以安全地再次使用上下文。
// 如果您在循环中重新计算布局，可能应该使用此函数，而不是调用 init/destroy。
LAY_EXPORT void lay_reset_context(lay_context *ctx);

// 执行布局计算，从根项（id 为 0）开始。
// 在调用此函数后，您可以使用 lay_get_rect() 查询项的计算矩形。
// 如果在调用此函数后使用 lay_append() 或 lay_insert() 等过程，若发生重新分配，您的计算数据可能会变得无效。
//
// 您应该更倾向于从根项重新创建项目，而不是对现有上下文进行细粒度更新。
//
// 但是，如果您只是对布局中的项进行缩放动画而没有更改其内容，
// 那么使用 lay_set_size 更新项，然后重新运行 lay_run_context 是安全的。
LAY_EXPORT void lay_run_context(lay_context *ctx);

// 与 lay_run_context() 类似，此过程将执行布局计算——但它允许您指定要从哪个项开始。
// lay_run_context() 总是从项 0，即第一个项，作为根项开始。
// 从特定项运行布局计算在您需要迭代地重新运行布局层次结构的部分时很有用，或者如果您只对更新它的某些子集感兴趣。
// 使用时要小心——如果父项尚未计算其输出矩形，或者它们已经失效（例如由于重新分配），很容易生成错误的输出。
LAY_EXPORT void lay_run_item(lay_context *ctx, lay_id item);

// 在启用换行的父容器中进行布局时，可能会在计算过程中修改标志。
// 如果您计划多次调用 lay_run_context 或 lay_run_item，而不调用 lay_reset，
// 并且如果容器使用了换行，且容器的宽度或高度可能已更改，您应在再次调用 lay_run_context 
// 或 lay_run_item 之前调用 lay_clear_item_break 来清除所有子项的换行标志。
// 如果不清除，布局计算可能会进行不必要的换行。
//
// 此要求未来可能会更改。
//
// 调用此函数还会重置所有手动指定的换行。您需要再次设置手动换行，或者干脆不在任何希望手动换行的项上调用此函数。
//
// 如果您每次计算布局时都清除上下文，或者如果不使用换行，则不需要调用此函数。
LAY_EXPORT void lay_clear_item_break(lay_context *ctx, lay_id item);

// 返回在上下文中已创建项的数量。
LAY_EXPORT lay_id lay_items_count(lay_context *ctx);

// 返回上下文在不进行重新分配的情况下可以容纳的项数。
LAY_EXPORT lay_id lay_items_capacity(lay_context *ctx);

// 创建一个新项，可以简单地认为它是一个矩形。返回用于标识该项的 id（句柄）。
LAY_EXPORT lay_id lay_item(lay_context *ctx);

// 将项插入到另一个项中，形成父子关系。
// 一个项可以包含任意数量的子项。插入到父项中的项会被放置在排序的末尾，在所有现有兄弟项之后。
LAY_EXPORT void lay_insert(lay_context *ctx, lay_id parent, lay_id child);

// lay_append 将一个项作为兄弟项插入到另一个项之后。
// 这允许您将项插入到父项中现有项列表的中间。
// 与在循环中反复使用 lay_insert(ctx, parent, new_child) 创建父项的项列表相比，它更高效，因为它不需要每次遍历父项的子项。
// 因此，如果您在父项中创建一个长的子项列表，可能会更倾向于在使用 lay_insert 插入第一个子项后使用此方法。
LAY_EXPORT void lay_append(lay_context *ctx, lay_id earlier, lay_id later);

// 与 lay_insert 相似，但将新项作为父项的第一个子项，而不是最后一个。
LAY_EXPORT void lay_push(lay_context *ctx, lay_id parent, lay_id child);

// 获取通过 lay_set_size 或 lay_set_size_xy 设置的大小。_xy 版本将输出值写入指定的地址，而不是返回 lay_vec2 中的值。
LAY_EXPORT lay_vec2 lay_get_size(lay_context *ctx, lay_id item);
LAY_EXPORT void lay_get_size_xy(lay_context *ctx, lay_id item, lay_scalar *x, lay_scalar *y);

// 设置项的大小。_xy 版本将宽度和高度作为单独的参数传递，但功能相同。
LAY_EXPORT void lay_set_size(lay_context *ctx, lay_id item, lay_vec2 size);
LAY_EXPORT void lay_set_size_xy(lay_context *ctx, lay_id item, lay_scalar width, lay_scalar height);

// 设置项的标志，决定它作为父项的行为。例如，设置 LAY_COLUMN 会使项表现得像一个列——它会垂直排列其子项。
LAY_EXPORT void lay_set_contain(lay_context *ctx, lay_id item, uint32_t flags);

// 设置项的标志，决定它作为父项内部子项的行为。例如，设置 LAY_VFILL 会使项尽量填满父项内所有可用的垂直空间。
LAY_EXPORT void lay_set_behave(lay_context *ctx, lay_id item, uint32_t flags);

// 获取通过 lay_set_margins 设置的边距。_ltrb 版本将输出值写入指定的地址，而不是返回 lay_vec4 中的值。
LAY_EXPORT lay_vec4 lay_get_margins(lay_context *ctx, lay_id item);
LAY_EXPORT void lay_get_margins_ltrb(lay_context *ctx, lay_id item, lay_scalar *l, lay_scalar *t, lay_scalar *r, lay_scalar *b);

// 设置项的边距。向量的组件是：
// 0: left, 1: top, 2: right, 3: bottom.
LAY_EXPORT void lay_set_margins(lay_context *ctx, lay_id item, lay_vec4 ltrb);

// 与 lay_set_margins 相同，但组件作为单独的参数传递。
// (left, top, right, bottom).
LAY_EXPORT void lay_set_margins_ltrb(lay_context *ctx, lay_id item, lay_scalar l, lay_scalar t, lay_scalar r, lay_scalar b);

// 通过项的 id 获取缓冲区中的项指针。
// 不要保留此指针——一旦发生任何重新分配，它将变得无效。只需存储 id（它更小，而且查找成本为零）。
LAY_STATIC_INLINE lay_item_t *lay_get_item(const lay_context *ctx, lay_id id)
{
    LAY_ASSERT(id != LAY_INVALID_ID && id < ctx->count);
    return ctx->items + id;
}

// 获取项的第一个子项的 id（如果有的话）。如果没有子项，则返回 LAY_INVALID_ID。
LAY_STATIC_INLINE lay_id lay_first_child(const lay_context *ctx, lay_id id)
{
    const lay_item_t *pitem = lay_get_item(ctx, id);
    return pitem->first_child;
}

// 获取项的下一个兄弟项的 id（如果有的话）。如果没有下一个兄弟项，则返回 LAY_INVALID_ID。
LAY_STATIC_INLINE lay_id lay_next_sibling(const lay_context *ctx, lay_id id)
{
    const lay_item_t *pitem = lay_get_item(ctx, id);
    return pitem->next_sibling;
}

// 返回项的计算矩形。
// 只有在调用 lay_run_context 后且在发生任何重新分配之前，这个值才有效。否则，结果将是未定义的。
// 向量的组件是：
// 0: x 起始位置, 1: y 起始位置
// 2: 宽度, 3: 高度
LAY_STATIC_INLINE lay_vec4 lay_get_rect(const lay_context *ctx, lay_id id)
{
    LAY_ASSERT(id != LAY_INVALID_ID && id < ctx->count);
    return ctx->rects[id];
}

// 与 lay_get_rect 相同，但将 x, y 位置和宽度、高度的值写入指定的地址，而不是返回 lay_vec4 中的值。
LAY_STATIC_INLINE void lay_get_rect_xywh(
        const lay_context *ctx, lay_id id,
        lay_scalar *x, lay_scalar *y, lay_scalar *width, lay_scalar *height)
{
    LAY_ASSERT(id != LAY_INVALID_ID && id < ctx->count);
    lay_vec4 rect = ctx->rects[id];
    *x = rect[0];
    *y = rect[1];
    *width = rect[2];
    *height = rect[3];
}

#undef LAY_EXPORT
#undef LAY_STATIC_INLINE

#endif // LAY_INCLUDE_HEADER

// 关于仅出于语法方便而使用vector_size的注意事项：
//
// 当前的布局计算过程并不是以能够从SIMD指令使用中获益为目标编写的。
//
// (通过使用__vectorcall传递128位float4向量，在某些特定情况下可能会带来一些小的好处，但这很可能不值得麻烦。
// 我相信只有在以防止编译器在复制矩形/大小数据时进行内联优化的方式下，才需要这样做。)
//
// 将来我可能会回头改用常规结构封装数组。
// 我不确定在GCC/clang中依赖向量处理，并在MSVC中使用C++运算符重载是否值得为了在实现代码中每个数组访问上节省几个额外字符而感到困扰。

#ifdef LAY_IMPLEMENTATION

#include <stddef.h>
#include <stdbool.h>

// Users of this library can define LAY_REALLOC to use a custom (re)allocator
// instead of stdlib's realloc. It should have the same behavior as realloc --
// first parameter type is a void pointer, and its value is either a null
// pointer or an existing pointer. The second parameter is a size_t of the new
// desired size. The buffer contents should be preserved across reallocations.
//
// And, if you define LAY_REALLOC, you will also need to define LAY_FREE, which
// should have the same behavior as free.
#ifndef LAY_REALLOC
#include <stdlib.h>
#define LAY_REALLOC(_block, _size) realloc(_block, _size)
#define LAY_FREE(_block) free(_block)
#endif

// Like the LAY_REALLOC define, LAY_MEMSET can be used for a custom memset.
// Otherwise, the memset from string.h will be used.
#ifndef LAY_MEMSET
#include <string.h>
#define LAY_MEMSET(_dst, _val, _size) memset(_dst, _val, _size)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LAY_FORCE_INLINE __attribute__((always_inline)) inline
#ifdef __cplusplus
#define LAY_RESTRICT __restrict
#else
#define LAY_RESTRICT restrict
#endif // __cplusplus
#elif defined(_MSC_VER)
#define LAY_FORCE_INLINE __forceinline
#define LAY_RESTRICT __restrict
#else
#define LAY_FORCE_INLINE inline
#ifdef __cplusplus
#define LAY_RESTRICT
#else
#define LAY_RESTRICT restrict
#endif // __cplusplus
#endif

// Useful math utilities
static LAY_FORCE_INLINE lay_scalar lay_scalar_max(lay_scalar a, lay_scalar b)
{ return a > b ? a : b; }
static LAY_FORCE_INLINE lay_scalar lay_scalar_min(lay_scalar a, lay_scalar b)
{ return a < b ? a : b; }
static LAY_FORCE_INLINE float lay_float_max(float a, float b)
{ return a > b ? a : b; }
static LAY_FORCE_INLINE float lay_float_min(float a, float b)
{ return a < b ? a : b; }

void lay_init_context(lay_context *ctx)
{
    ctx->capacity = 0;
    ctx->count = 0;
    ctx->items = NULL;
    ctx->rects = NULL;
}

void lay_reserve_items_capacity(lay_context *ctx, lay_id count)
{
    if (count >= ctx->capacity) {
        ctx->capacity = count;
        const size_t item_size = sizeof(lay_item_t) + sizeof(lay_vec4);
        ctx->items = (lay_item_t*)LAY_REALLOC(ctx->items, ctx->capacity * item_size);
        const lay_item_t *past_last = ctx->items + ctx->capacity;
        ctx->rects = (lay_vec4*)past_last;
    }
}

void lay_destroy_context(lay_context *ctx)
{
    if (ctx->items != NULL) {
        LAY_FREE(ctx->items);
        ctx->items = NULL;
        ctx->rects = NULL;
    }
}

void lay_reset_context(lay_context *ctx)
{ ctx->count = 0; }

static void lay_calc_size(lay_context *ctx, lay_id item, int dim);
static void lay_arrange(lay_context *ctx, lay_id item, int dim);

void lay_run_context(lay_context *ctx)
{
    LAY_ASSERT(ctx != NULL);

    if (ctx->count > 0) {
        lay_run_item(ctx, 0);
    }
}

void lay_run_item(lay_context *ctx, lay_id item)
{
    LAY_ASSERT(ctx != NULL);

    lay_calc_size(ctx, item, 0);
    lay_arrange(ctx, item, 0);
    lay_calc_size(ctx, item, 1);
    lay_arrange(ctx, item, 1);
}

// Alternatively, we could use a flag bit to indicate whether an item's children
// have already been wrapped and may need re-wrapping. If we do that, in the
// future, this would become deprecated and we could make it a no-op.

void lay_clear_item_break(lay_context *ctx, lay_id item)
{
    LAY_ASSERT(ctx != NULL);
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->flags = pitem->flags & ~(uint32_t)LAY_BREAK;
}

lay_id lay_items_count(lay_context *ctx)
{
    LAY_ASSERT(ctx != NULL);
    return ctx->count;
}

lay_id lay_items_capacity(lay_context *ctx)
{
    LAY_ASSERT(ctx != NULL);
    return ctx->capacity;
}

lay_id lay_item(lay_context *ctx)
{
    lay_id idx = ctx->count++;

    if (idx >= ctx->capacity) {
        ctx->capacity = ctx->capacity < 1 ? 32 : (ctx->capacity * 4);
        const size_t item_size = sizeof(lay_item_t) + sizeof(lay_vec4);
        ctx->items = (lay_item_t*)LAY_REALLOC(ctx->items, ctx->capacity * item_size);
        const lay_item_t *past_last = ctx->items + ctx->capacity;
        ctx->rects = (lay_vec4*)past_last;
    }

    lay_item_t *item = lay_get_item(ctx, idx);
    // We can either do this here, or when creating/resetting buffer
    LAY_MEMSET(item, 0, sizeof(lay_item_t));
    item->first_child = LAY_INVALID_ID;
    item->next_sibling = LAY_INVALID_ID;
    // hmm
    LAY_MEMSET(&ctx->rects[idx], 0, sizeof(lay_vec4));
    return idx;
}

static LAY_FORCE_INLINE
void lay_append_by_ptr(
        lay_item_t *LAY_RESTRICT pearlier,
        lay_id later, lay_item_t *LAY_RESTRICT plater)
{
    plater->next_sibling = pearlier->next_sibling;
    plater->flags |= LAY_ITEM_INSERTED;
    pearlier->next_sibling = later;
}

lay_id lay_last_child(const lay_context *ctx, lay_id parent)
{
    lay_item_t *pparent = lay_get_item(ctx, parent);
    lay_id child = pparent->first_child;
    if (child == LAY_INVALID_ID) return LAY_INVALID_ID;
    lay_item_t *pchild = lay_get_item(ctx, child);
    lay_id result = child;
    for (;;) {
        lay_id next = pchild->next_sibling;
        if (next == LAY_INVALID_ID) break;
        result = next;
        pchild = lay_get_item(ctx, next);
    }
    return result;
}

void lay_append(lay_context *ctx, lay_id earlier, lay_id later)
{
    LAY_ASSERT(later != 0); // Must not be root item
    LAY_ASSERT(earlier != later); // Must not be same item id
    lay_item_t *LAY_RESTRICT pearlier = lay_get_item(ctx, earlier);
    lay_item_t *LAY_RESTRICT plater = lay_get_item(ctx, later);
    lay_append_by_ptr(pearlier, later, plater);
}

void lay_insert(lay_context *ctx, lay_id parent, lay_id child)
{
    LAY_ASSERT(child != 0); // Must not be root item
    LAY_ASSERT(parent != child); // Must not be same item id
    lay_item_t *LAY_RESTRICT pparent = lay_get_item(ctx, parent);
    lay_item_t *LAY_RESTRICT pchild = lay_get_item(ctx, child);
    LAY_ASSERT(!(pchild->flags & LAY_ITEM_INSERTED));
    // Parent has no existing children, make inserted item the first child.
    if (pparent->first_child == LAY_INVALID_ID) {
        pparent->first_child = child;
        pchild->flags |= LAY_ITEM_INSERTED;
    // Parent has existing items, iterate to find the last child and append the
    // inserted item after it.
    } else {
        lay_id next = pparent->first_child;
        lay_item_t *LAY_RESTRICT pnext = lay_get_item(ctx, next);
        for (;;) {
            next = pnext->next_sibling;
            if (next == LAY_INVALID_ID) break;
            pnext = lay_get_item(ctx, next);
        }
        lay_append_by_ptr(pnext, child, pchild);
    }
}

void lay_push(lay_context *ctx, lay_id parent, lay_id new_child)
{
    LAY_ASSERT(new_child != 0); // Must not be root item
    LAY_ASSERT(parent != new_child); // Must not be same item id
    lay_item_t *LAY_RESTRICT pparent = lay_get_item(ctx, parent);
    lay_id old_child = pparent->first_child;
    lay_item_t *LAY_RESTRICT pchild = lay_get_item(ctx, new_child);
    LAY_ASSERT(!(pchild->flags & LAY_ITEM_INSERTED));
    pparent->first_child = new_child;
    pchild->flags |= LAY_ITEM_INSERTED;
    pchild->next_sibling = old_child;
}

lay_vec2 lay_get_size(lay_context *ctx, lay_id item)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    return pitem->size;
}

void lay_get_size_xy(
        lay_context *ctx, lay_id item,
        lay_scalar *x, lay_scalar *y)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec2 size = pitem->size;
    *x = size[0];
    *y = size[1];
}

void lay_set_size(lay_context *ctx, lay_id item, lay_vec2 size)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->size = size;
    uint32_t flags = pitem->flags;
    if (size[0] == 0)
        flags &= ~(uint32_t)LAY_ITEM_HFIXED;
    else
        flags |= LAY_ITEM_HFIXED;
    if (size[1] == 0)
        flags &= ~(uint32_t)LAY_ITEM_VFIXED;
    else
        flags |= LAY_ITEM_VFIXED;
    pitem->flags = flags;
}

void lay_set_size_xy(
        lay_context *ctx, lay_id item,
        lay_scalar width, lay_scalar height)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->size[0] = width;
    pitem->size[1] = height;
    // Kinda redundant, whatever
    uint32_t flags = pitem->flags;
    if (width == 0)
        flags &= ~(uint32_t)LAY_ITEM_HFIXED;
    else
        flags |= LAY_ITEM_HFIXED;
    if (height == 0)
        flags &= ~(uint32_t)LAY_ITEM_VFIXED;
    else
        flags |= LAY_ITEM_VFIXED;
    pitem->flags = flags;
}

void lay_set_behave(lay_context *ctx, lay_id item, uint32_t flags)
{
    LAY_ASSERT((flags & LAY_ITEM_LAYOUT_MASK) == flags);
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->flags = (pitem->flags & ~(uint32_t)LAY_ITEM_LAYOUT_MASK) | flags;
}

void lay_set_contain(lay_context *ctx, lay_id item, uint32_t flags)
{
    LAY_ASSERT((flags & LAY_ITEM_BOX_MASK) == flags);
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->flags = (pitem->flags & ~(uint32_t)LAY_ITEM_BOX_MASK) | flags;
}
void lay_set_margins(lay_context *ctx, lay_id item, lay_vec4 ltrb)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->margins = ltrb;
}
void lay_set_margins_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar l, lay_scalar t, lay_scalar r, lay_scalar b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    // Alternative, uses stack and addressed writes
    //pitem->margins = lay_vec4_xyzw(l, t, r, b);
    // Alternative, uses rax and left-shift
    //pitem->margins = (lay_vec4){l, t, r, b};
    // Fewest instructions, but uses more addressed writes?
    pitem->margins[0] = l;
    pitem->margins[1] = t;
    pitem->margins[2] = r;
    pitem->margins[3] = b;
}

lay_vec4 lay_get_margins(lay_context *ctx, lay_id item)
{ return lay_get_item(ctx, item)->margins; }

void lay_get_margins_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar *l, lay_scalar *t, lay_scalar *r, lay_scalar *b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec4 margins = pitem->margins;
    *l = margins[0];
    *t = margins[1];
    *r = margins[2];
    *b = margins[3];
}

// TODO restrict item ptrs correctly
static LAY_FORCE_INLINE
lay_scalar lay_calc_overlayed_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        // width = start margin + calculated width + end margin
        lay_scalar child_size = rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        need_size = lay_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    return need_size;
}

static LAY_FORCE_INLINE
lay_scalar lay_calc_stacked_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        need_size += rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        child = pchild->next_sibling;
    }
    return need_size;
}

static LAY_FORCE_INLINE
lay_scalar lay_calc_wrapped_overlayed_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_scalar need_size2 = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        if (pchild->flags & LAY_BREAK) {
            need_size2 += need_size;
            need_size = 0;
        }
        lay_scalar child_size = rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        need_size = lay_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    return need_size2 + need_size;
}

// Equivalent to uiComputeWrappedStackedSize
static LAY_FORCE_INLINE
lay_scalar lay_calc_wrapped_stacked_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_scalar need_size2 = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        if (pchild->flags & LAY_BREAK) {
            need_size2 = lay_scalar_max(need_size2, need_size);
            need_size = 0;
        }
        need_size += rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        child = pchild->next_sibling;
    }
    return lay_scalar_max(need_size2, need_size);
}

static void lay_calc_size(lay_context *ctx, lay_id item, int dim)
{
    lay_item_t *pitem = lay_get_item(ctx, item);

    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        // NOTE: this is recursive and will run out of stack space if items are
        // nested too deeply.
        lay_calc_size(ctx, child, dim);
        lay_item_t *pchild = lay_get_item(ctx, child);
        child = pchild->next_sibling;
    }

    // Set the mutable rect output data to the starting input data
    ctx->rects[item][dim] = pitem->margins[dim];

    // If we have an explicit input size, just set our output size (which other
    // calc_size and arrange procedures will use) to it.
    if (pitem->size[dim] != 0) {
        ctx->rects[item][2 + dim] = pitem->size[dim];
        return;
    }

    // Calculate our size based on children items. Note that we've already
    // called lay_calc_size on our children at this point.
    lay_scalar cal_size;
    switch (pitem->flags & LAY_ITEM_BOX_MODEL_MASK) {
    case LAY_COLUMN|LAY_WRAP:
        // flex model
        if (dim) // direction
            cal_size = lay_calc_stacked_size(ctx, item, 1);
        else
            cal_size = lay_calc_overlayed_size(ctx, item, 0);
        break;
    case LAY_ROW|LAY_WRAP:
        // flex model
        if (!dim) // direction
            cal_size = lay_calc_wrapped_stacked_size(ctx, item, 0);
        else
            cal_size = lay_calc_wrapped_overlayed_size(ctx, item, 1);
        break;
    case LAY_COLUMN:
    case LAY_ROW:
        // flex model
        if ((pitem->flags & 1) == (uint32_t)dim) // direction
            cal_size = lay_calc_stacked_size(ctx, item, dim);
        else
            cal_size = lay_calc_overlayed_size(ctx, item, dim);
        break;
    default:
        // layout model
        cal_size = lay_calc_overlayed_size(ctx, item, dim);
        break;
    }

    // Set our output data size. Will be used by parent calc_size procedures.,
    // and by arrange procedures.
    ctx->rects[item][2 + dim] = cal_size;
}

static LAY_FORCE_INLINE
void lay_arrange_stacked(
            lay_context *ctx, lay_id item, int dim, bool wrap)
{
    const int wdim = dim + 2;
    lay_item_t *pitem = lay_get_item(ctx, item);

    const uint32_t item_flags = pitem->flags;
    lay_vec4 rect = ctx->rects[item];
    lay_scalar space = rect[2 + dim];

    float max_x2 = (float)(rect[dim] + space);

    lay_id start_child = pitem->first_child;
    while (start_child != LAY_INVALID_ID) {
        lay_scalar used = 0;
        uint32_t count = 0; // count of fillers
        uint32_t squeezed_count = 0; // count of squeezable elements
        uint32_t total = 0;
        bool hardbreak = false;
        // first pass: count items that need to be expanded,
        // and the space that is used
        lay_id child = start_child;
        lay_id end_child = LAY_INVALID_ID;
        while (child != LAY_INVALID_ID) {
            lay_item_t *pchild = lay_get_item(ctx, child);
            const uint32_t child_flags = pchild->flags;
            const uint32_t flags = (child_flags & LAY_ITEM_LAYOUT_MASK) >> dim;
            const uint32_t fflags = (child_flags & LAY_ITEM_FIXED_MASK) >> dim;
            const lay_vec4 child_margins = pchild->margins;
            lay_vec4 child_rect = ctx->rects[child];
            lay_scalar extend = used;
            if ((flags & LAY_HFILL) == LAY_HFILL) {
                ++count;
                extend += child_rect[dim] + child_margins[wdim];
            } else {
                if ((fflags & LAY_ITEM_HFIXED) != LAY_ITEM_HFIXED)
                    ++squeezed_count;
                extend += child_rect[dim] + child_rect[2 + dim] + child_margins[wdim];
            }
            // wrap on end of line or manual flag
            if (wrap && (
                    total && ((extend > space) ||
                    (child_flags & LAY_BREAK)))) {
                end_child = child;
                hardbreak = (child_flags & LAY_BREAK) == LAY_BREAK;
                // add marker for subsequent queries
                pchild->flags = child_flags | LAY_BREAK;
                break;
            } else {
                used = extend;
                child = pchild->next_sibling;
            }
            ++total;
        }

        lay_scalar extra_space = space - used;
        float filler = 0.0f;
        float spacer = 0.0f;
        float extra_margin = 0.0f;
        float eater = 0.0f;

        if (extra_space > 0) {
            if (count > 0)
                filler = (float)extra_space / (float)count;
            else if (total > 0) {
                switch (item_flags & LAY_JUSTIFY) {
                case LAY_JUSTIFY:
                    // justify when not wrapping or not in last line,
                    // or not manually breaking
                    if (!wrap || ((end_child != LAY_INVALID_ID) && !hardbreak))
                        spacer = (float)extra_space / (float)(total - 1);
                    break;
                case LAY_START:
                    break;
                case LAY_END:
                    extra_margin = extra_space;
                    break;
                default:
                    extra_margin = extra_space / 2.0f;
                    break;
                }
            }
        }
#ifdef LAY_FLOAT
        // In floating point, it's possible to end up with some small negative
        // value for extra_space, while also have a 0.0 squeezed_count. This
        // would cause divide by zero. Instead, we'll check to see if
        // squeezed_count is > 0. I believe this produces the same results as
        // the original oui int-only code. However, I don't have any tests for
        // it, so I'll leave it if-def'd for now.
        else if (!wrap && (squeezed_count > 0))
#else
        // This is the original oui code
        else if (!wrap && (extra_space < 0))
#endif
            eater = (float)extra_space / (float)squeezed_count;

        // distribute width among items
        float x = (float)rect[dim];
        float x1;
        // second pass: distribute and rescale
        child = start_child;
        while (child != end_child) {
            lay_scalar ix0, ix1;
            lay_item_t *pchild = lay_get_item(ctx, child);
            const uint32_t child_flags = pchild->flags;
            const uint32_t flags = (child_flags & LAY_ITEM_LAYOUT_MASK) >> dim;
            const uint32_t fflags = (child_flags & LAY_ITEM_FIXED_MASK) >> dim;
            const lay_vec4 child_margins = pchild->margins;
            lay_vec4 child_rect = ctx->rects[child];

            x += (float)child_rect[dim] + extra_margin;
            if ((flags & LAY_HFILL) == LAY_HFILL) // grow
                x1 = x + filler;
            else if ((fflags & LAY_ITEM_HFIXED) == LAY_ITEM_HFIXED)
                x1 = x + (float)child_rect[2 + dim];
            else // squeeze
                x1 = x + lay_float_max(0.0f, (float)child_rect[2 + dim] + eater);

            ix0 = (lay_scalar)x;
            if (wrap)
                ix1 = (lay_scalar)lay_float_min(max_x2 - (float)child_margins[wdim], x1);
            else
                ix1 = (lay_scalar)x1;
            child_rect[dim] = ix0; // pos
            child_rect[dim + 2] = ix1 - ix0; // size
            ctx->rects[child] = child_rect;
            x = x1 + (float)child_margins[wdim];
            child = pchild->next_sibling;
            extra_margin = spacer;
        }

        start_child = end_child;
    }
}

static LAY_FORCE_INLINE
void lay_arrange_overlay(lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *pitem = lay_get_item(ctx, item);
    const lay_vec4 rect = ctx->rects[item];
    const lay_scalar offset = rect[dim];
    const lay_scalar space = rect[2 + dim];
    
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        const uint32_t b_flags = (pchild->flags & LAY_ITEM_LAYOUT_MASK) >> dim;
        const lay_vec4 child_margins = pchild->margins;
        lay_vec4 child_rect = ctx->rects[child];

        switch (b_flags & LAY_HFILL) {
        case LAY_HCENTER:
            child_rect[dim] += (space - child_rect[2 + dim]) / 2 - child_margins[wdim];
            break;
        case LAY_RIGHT:
            child_rect[dim] += space - child_rect[2 + dim] - child_margins[dim] - child_margins[wdim];
            break;
        case LAY_HFILL:
            child_rect[2 + dim] = lay_scalar_max(0, space - child_rect[dim] - child_margins[wdim]);
            break;
        default:
            break;
        }

        child_rect[dim] += offset;
        ctx->rects[child] = child_rect;
        child = pchild->next_sibling;
    }
}

static LAY_FORCE_INLINE
void lay_arrange_overlay_squeezed_range(
        lay_context *ctx, int dim,
        lay_id start_item, lay_id end_item,
        lay_scalar offset, lay_scalar space)
{
    int wdim = dim + 2;
    lay_id item = start_item;
    while (item != end_item) {
        lay_item_t *pitem = lay_get_item(ctx, item);
        const uint32_t b_flags = (pitem->flags & LAY_ITEM_LAYOUT_MASK) >> dim;
        const lay_vec4 margins = pitem->margins;
        lay_vec4 rect = ctx->rects[item];
        lay_scalar min_size = lay_scalar_max(0, space - rect[dim] - margins[wdim]);
        switch (b_flags & LAY_HFILL) {
            case LAY_HCENTER:
                rect[2 + dim] = lay_scalar_min(rect[2 + dim], min_size);
                rect[dim] += (space - rect[2 + dim]) / 2 - margins[wdim];
                break;
            case LAY_RIGHT:
                rect[2 + dim] = lay_scalar_min(rect[2 + dim], min_size);
                rect[dim] = space - rect[2 + dim] - margins[wdim];
                break;
            case LAY_HFILL:
                rect[2 + dim] = min_size;
                break;
            default:
                rect[2 + dim] = lay_scalar_min(rect[2 + dim], min_size);
                break;
        }
        rect[dim] += offset;
        ctx->rects[item] = rect;
        item = pitem->next_sibling;
    }
}

static LAY_FORCE_INLINE
lay_scalar lay_arrange_wrapped_overlay_squeezed(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_scalar offset = ctx->rects[item][dim];
    lay_scalar need_size = 0;
    lay_id child = pitem->first_child;
    lay_id start_child = child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        if (pchild->flags & LAY_BREAK) {
            lay_arrange_overlay_squeezed_range(ctx, dim, start_child, child, offset, need_size);
            offset += need_size;
            start_child = child;
            need_size = 0;
        }
        const lay_vec4 rect = ctx->rects[child];
        lay_scalar child_size = rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        need_size = lay_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    lay_arrange_overlay_squeezed_range(ctx, dim, start_child, LAY_INVALID_ID, offset, need_size);
    offset += need_size;
    return offset;
}

static void lay_arrange(lay_context *ctx, lay_id item, int dim)
{
    lay_item_t *pitem = lay_get_item(ctx, item);

    const uint32_t flags = pitem->flags;
    switch (flags & LAY_ITEM_BOX_MODEL_MASK) {
    case LAY_COLUMN | LAY_WRAP:
        if (dim != 0) {
            lay_arrange_stacked(ctx, item, 1, true);
            lay_scalar offset = lay_arrange_wrapped_overlay_squeezed(ctx, item, 0);
            ctx->rects[item][2 + 0] = offset - ctx->rects[item][0];
        }
        break;
    case LAY_ROW | LAY_WRAP:
        if (dim == 0)
            lay_arrange_stacked(ctx, item, 0, true);
        else
            // discard return value
            lay_arrange_wrapped_overlay_squeezed(ctx, item, 1);
        break;
    case LAY_COLUMN:
    case LAY_ROW:
        if ((flags & 1) == (uint32_t)dim) {
            lay_arrange_stacked(ctx, item, dim, false);
        } else {
            const lay_vec4 rect = ctx->rects[item];
            lay_arrange_overlay_squeezed_range(
                ctx, dim, pitem->first_child, LAY_INVALID_ID,
                rect[dim], rect[2 + dim]);
        }
        break;
    default:
        lay_arrange_overlay(ctx, item, dim);
        break;
    }
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        // NOTE: this is recursive and will run out of stack space if items are
        // nested too deeply.
        lay_arrange(ctx, child, dim);
        lay_item_t *pchild = lay_get_item(ctx, child);
        child = pchild->next_sibling;
    }
}

#endif // LAY_IMPLEMENTATION
