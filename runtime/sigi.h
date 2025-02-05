
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "closure.h"

typedef struct sigi_stack_impl* sigi_stack_t;

void sigi_init_stack(sigi_stack_t* stack);
void sigi_free_stack(sigi_stack_t* stack);

void sigi_push_i32(sigi_stack_t* stack, int32_t value);
void sigi_push_bool(sigi_stack_t* stack, bool value);
void sigi_push_closure(sigi_stack_t* stack, closure_t* value);

closure_t* sigi_pop_closure(sigi_stack_t* stack);
int32_t sigi_pop_i32(sigi_stack_t* stack);
bool sigi_pop_bool(sigi_stack_t* stack);

// This is the implementation of the pp method.
// Prints the top but does not pop it.
void sigi_print_stack_top_ln(sigi_stack_t*);

void sigi_print_i32(int32_t);

void sigi_print_bool(bool);

void sigi_print_closure(closure_t* );

// builtin implementations
sigi_stack_t* sigi_builtin__pp(sigi_stack_t*);
