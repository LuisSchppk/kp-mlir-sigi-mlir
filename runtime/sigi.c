
#include "sigi.h"

// #include <cstdint>
#include <stdio.h>

// #define DEBUG

#ifdef DEBUG
#    define DEBUG_PRINT(x) printf x
#else
#    define DEBUG_PRINT(x)                                                                         \
        do {                                                                                       \
        } while (0)
#endif

typedef enum value_tag { TAG_I32, TAG_BOOL, TAG_CLOSURE } value_tag;

typedef struct sigi_value {
    enum value_tag tag;
    union value_data {
        int32_t i32;
        bool boolean;
        closure_t* closure;
    } data;
} sigi_value;

struct sigi_stack_impl {
    size_t count;
    size_t capacity;
    sigi_value* buffer;
};

/// @brief Initialize an empty stack.
void sigi_init_stack(sigi_stack_t* stack)
{
    struct sigi_stack_impl* impl = (struct sigi_stack_impl*)malloc(sizeof(struct sigi_stack_impl));
    impl->count = 0;
    impl->capacity = 4;
    impl->buffer = (sigi_value*)malloc(4 * sizeof(sigi_value));
    *stack = impl;
}

void sigi_free_stack(sigi_stack_t* stack) { free(*stack); }

void sigi_abort(const char msg[])
{
    puts(msg);
    exit(1);
}

void grow1(sigi_stack_t* stack, sigi_value new_value)
{
    struct sigi_stack_impl* impl = *stack;
    if (impl->count == impl->capacity) {
        size_t new_cap = impl->capacity * 2;
        sigi_value* grown_buf = (sigi_value*)realloc(impl->buffer, new_cap * sizeof(sigi_value));
        if (NULL == grown_buf) sigi_abort("Error (re)allocating memory for the stack");
        impl->buffer = grown_buf;
        impl->capacity = new_cap;
    }
    impl->buffer[impl->count] = new_value;
    impl->count++;

    DEBUG_PRINT(("+%d\n", new_value.tag));
}

sigi_value* peek1(sigi_stack_t* stack)
{
    struct sigi_stack_impl* impl = *stack;
    if (impl->count == 0) sigi_abort("Attempted to pop an element from an empty stack.");

    return &impl->buffer[impl->count - 1];
}

sigi_value pop1(sigi_stack_t* stack)
{
    struct sigi_stack_impl* impl = *stack;
    if (impl->count == 0) sigi_abort("Attempted to pop an element from an empty stack.");

    impl->count--;
    sigi_value top = impl->buffer[impl->count];
    DEBUG_PRINT(("-%d\n", top.tag));
    return top;
}

void sigi_push_i32(sigi_stack_t* stack, int32_t value)
{
    sigi_value v = {.tag = TAG_I32, .data = {.i32 = value}};
    grow1(stack, v);
}
void sigi_push_closure(sigi_stack_t* stack, closure_t* value)
{
    closure_incr(value);
    sigi_value v = {.tag = TAG_CLOSURE, .data = {.closure = value}};
    grow1(stack, v);
}
void sigi_push_bool(sigi_stack_t* stack, bool value)
{
    sigi_value v = {.tag = TAG_BOOL, .data = {.boolean = value}};
    grow1(stack, v);
}

void check_tag(sigi_value value, value_tag expected_tag)
{
    if (value.tag != expected_tag) {
        printf("Wrong tag: expected %d but got %d\n", expected_tag, value.tag);
        exit(1);
    }
}
closure_t* sigi_pop_closure(sigi_stack_t* stack)
{
    sigi_value value = pop1(stack);
    check_tag(value, TAG_CLOSURE);
    closure_decr(value.data.closure);
    return value.data.closure;
}
int32_t sigi_pop_i32(sigi_stack_t* stack)
{
    sigi_value value = pop1(stack);
    check_tag(value, TAG_I32);
    return value.data.i32;
}
bool sigi_pop_bool(sigi_stack_t* stack)
{
    sigi_value value = pop1(stack);
    check_tag(value, TAG_BOOL);
    return value.data.boolean;
}

/// @brief Print an integer to standard output.
void sigi_show_int(int32_t i) { printf("%d\n", i); }

void sigi_print_stack_top_ln(sigi_stack_t* stack)
{
    sigi_value* top = peek1(stack);
    switch (top->tag) {
    case TAG_BOOL:
        if (top->data.boolean)
            printf("true\n");
        else
            printf("false\n");
        break;
    case TAG_I32: printf("%d\n", top->data.i32); break;
    case TAG_CLOSURE: printf("(opaque closure, rc=%d)\n", top->data.closure->refcount); break;
    }
}

void sigi_builtin__pp_i32(int32_t integer)
{
    printf("DEBUG: sigi_builtin__pp_i32 called\n");
    printf("%d\n", integer);
}

void sigi_builtin__pp_i1(bool boolean)
{
    if (boolean)
        printf("true\n");
    else
        printf("false\n");
}

void sigi_builtin__pp_closure(closure_t* closure)
{
    printf("(opaque closure, rc=%d)\n", closure->refcount);
}

sigi_stack_t* sigi_builtin__pp(sigi_stack_t* stack)
{
    sigi_print_stack_top_ln(stack);
    return stack;
}
