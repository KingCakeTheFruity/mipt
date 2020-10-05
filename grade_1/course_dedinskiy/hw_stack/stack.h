/**
    \file
    \brief Stack with its full implementation
*/

#include <stdlib.h>
#include "general.h"

#ifndef STACK_VALUE_TYPE
#error STACK_VALUE_TYPE must be defined to define the stack properly
#endif

#ifndef STACK_VALUE_PRINTF_SPEC
#error STACK_VALUE_PRINTF_SPEC must be defined to dump the stack properly
#endif

//[SECURITY_SETTINGS]==========================================================

#ifndef STACK_SECURITY_LEVEL
#define STACK_SECURITY_LEVEL 10
#endif // STACK_SECURITY_LEVEL

#if STACK_SECURITY_LEVEL > 0
#define SEC_CANARY
#endif

#if STACK_SECURITY_LEVEL > 2
#define SEC_HASH
#endif

//=============================================================================
//[ONCE_INCLUDING_CONSTANTS]===================================================

#ifndef KCTF_STACK_CONSTANTS
#define KCTF_STACK_CONSTANTS

#define STACK_GENERIC(func) OVERLOAD(Stack_##func, STACK_VALUE_TYPE)
#define STACK_GENERIC_TYPE OVERLOAD(Stack, STACK_VALUE_TYPE)
#define STACK_OK(stack) do {RETURNING_ASSERT_OK(STACK_GENERIC(valid)(stack));} while(0)
#define STACK_HASH(stack) STACK_GENERIC(hash)(stack)

typedef enum stack_code {
    ERR_STACK_NOT_CREATED = -99,
    ERR_STACK_NOT_EXIST,
    ERR_BUFFER_NOT_EXIST,
    ERR_OVERFLOW,
    ERR_REALLOC_FAILED,

    OK = 0,
} stack_code;

const STACK_VALUE_TYPE SVT_P = -7777777.0;
const long long STACK_CANARY = 77777777777777;

const size_t STACK_DUMP_DEPTH = 10;

const double STACK_REALLOC_UP_COEF = 1.5;
const double STACK_REALLOC_DOWN_COEF = 2;
#endif // KCTF_STACK_CONSTANTS

//=============================================================================
//<KCTF>[STACK_H]==============================================================

struct STACK_GENERIC(t) {
//[SOME_AWFUL_SECURITY]========================================================
#ifdef SEC_HASH
    long long hash_left;
#else
#ifdef SEC_CANARY
    long long canary_left;
#endif
#endif
//=============================================================================
//[STACK]======================================================================
    size_t capacity;
    size_t size;
    STACK_VALUE_TYPE *buffer;
//=============================================================================
//[SOME_AWFUL_SECURITY]========================================================
#ifdef SEC_HASH
    long long hash_right;
#else
#ifdef SEC_CANARY
    long long canary_right;
#endif
#endif
//=============================================================================
};

//[DOCUMENTARY]================================================================

typedef struct STACK_GENERIC(t) STACK_GENERIC_TYPE;

long long STACK_GENERIC(hash)(const STACK_GENERIC_TYPE *cake); ///< Calculates hash(cake) the right way

void      STACK_GENERIC(recalculate_hash)   (STACK_GENERIC_TYPE *cake); ///< Calls hash(cake) the right way and sets hash_lr
int       STACK_GENERIC(recalcute_security) (STACK_GENERIC_TYPE *cake); ///< Calls recalculate_security(cake) and sets security vars

int       STACK_GENERIC(construct)      (STACK_GENERIC_TYPE *cake); ///< Constructs stack the right way. Must be called before any operations with stack
int       STACK_GENERIC(destruct)       (STACK_GENERIC_TYPE *cake); ///< Destructs stack the right way. Must be called after all operations with stack

int       STACK_GENERIC(dump)     (const STACK_GENERIC_TYPE *cake); ///< Dumps stack in a pretty way
int       STACK_GENERIC(valid)    (const STACK_GENERIC_TYPE *cake); ///< Checks, if stack is valid, returns 0 if is, err_code otherwise

size_t    STACK_GENERIC(size)     (const STACK_GENERIC_TYPE *cake); ///< Returs current number of elements in stack
size_t    STACK_GENERIC(capacity) (const STACK_GENERIC_TYPE *cake); ///< Return current max_number of elements in stack

size_t    STACK_GENERIC(is_empty) (const STACK_GENERIC_TYPE *cake); ///< Returns 1 if stack is empty, 0 otherwise
size_t    STACK_GENERIC(is_full)  (const STACK_GENERIC_TYPE *cake); ///< Returns 1 if cake->size == cake->capacity, 0 otherwise

int STACK_GENERIC(push) (STACK_GENERIC_TYPE *cake, const STACK_VALUE_TYPE val); ///< Pushes val on the top of stack
int STACK_GENERIC(pop)  (STACK_GENERIC_TYPE *cake); ///< Pop top val from stack. DOES NOT return value
int STACK_GENERIC(clear)(STACK_GENERIC_TYPE *cake); ///< Pops all elements from stack

int STACK_GENERIC(realloc)(STACK_GENERIC_TYPE *cake, const size_t new_capacity); ///< Tries to make cake->capacity be new_capacity

//=============================================================================
//=============================================================================
//<KCTF>[STACK_C]==============================================================

long long STACK_GENERIC(hash)(const STACK_GENERIC_TYPE *cake) {
    RETURNING_ASSERT(cake != NULL);
    RETURNING_ASSERT(cake->buffer != NULL);
    return + do_hash((char*)cake + sizeof(long long), sizeof(STACK_GENERIC_TYPE) - 2 * sizeof(long long))
           + do_hash(cake->buffer, cake->capacity * sizeof(STACK_VALUE_TYPE));
}

#ifdef SEC_HASH
void STACK_GENERIC(recalculate_hash)(STACK_GENERIC_TYPE *cake) {
    long long int new_hash = STACK_HASH(cake);
    cake->hash_left = new_hash;
    cake->hash_right = new_hash;
}
#endif

int STACK_GENERIC(valid)(const STACK_GENERIC_TYPE *cake) {
    if (!cake) {
        RETURN_ERROR_ASSERT(ERR_STACK_NOT_EXIST);
    }

    if (!cake->buffer) {
        RETURN_ERROR_ASSERT(ERR_BUFFER_NOT_EXIST);
    }

    if (cake->size > cake->capacity) {
        RETURN_ERROR_ASSERT(ERR_OVERFLOW);
    }

#ifdef SEC_HASH
    if (cake->hash_left != STACK_HASH(cake) || cake->hash_left != cake->hash_right) {
        RETURN_ERROR_ASSERT(ERROR_BAD_HASH);
    }
#else
#ifdef SEC_CANARY
    if (cake->canary_left != STACK_CANARY || cake->canary_left != cake->canary_right) {
        RETURN_ERROR_ASSERT(ERROR_BAD_CANARY);
    }
#endif
#endif

    return OK;
}

int STACK_GENERIC(recalcute_security)(STACK_GENERIC_TYPE *cake) {
    RETURNING_ASSERT(cake != NULL);

#ifdef SEC_HASH
    STACK_GENERIC(recalculate_hash)(cake);
#else
#ifdef SEC_CANARY
    cake->canary_left = STACK_CANARY;
    cake->canary_right = STACK_CANARY;
#endif
#endif

    return OK;
}

int STACK_GENERIC(construct)(STACK_GENERIC_TYPE *cake) {
    RETURNING_ASSERT(cake != NULL);

    const size_t capacity = 32;
    cake->buffer = (STACK_VALUE_TYPE*) calloc(capacity, sizeof(STACK_VALUE_TYPE));
    RETURNING_ASSERT(cake->buffer != NULL);

    cake->capacity = capacity;
    cake->size = 0;

    RETURNING_ASSERT_OK(STACK_GENERIC(recalcute_security)(cake));
    STACK_OK(cake);
    return OK;
}

int STACK_GENERIC(destruct)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);

    free(cake->buffer);
    cake->capacity = SIZE_T_P;
    cake->size = SIZE_T_P;

    return OK;
}

size_t STACK_GENERIC(size)(const STACK_GENERIC_TYPE *cake) {
    RETURNING_ASSERT_OK(STACK_GENERIC(valid)(cake));
    return cake->size;
}

size_t STACK_GENERIC(capacity)(const STACK_GENERIC_TYPE *cake) {
    RETURNING_ASSERT_OK(STACK_GENERIC(valid)(cake));
    return cake->capacity;
}

int STACK_GENERIC(dump)(const STACK_GENERIC_TYPE *cake) {
    const int validity = STACK_GENERIC(valid)(cake);
    printf("[DMP]<stack>: [ptr](%p) [valid](%s)\n", (void*) cake, validity ? "FALSE" : "true");
#ifdef SEC_HASH
    printf("[   ]<     >: [hash_l](%lld)\n", cake->hash_left);
    printf("[   ]<     >: [hash_r](%lld)\n", cake->hash_right);
#else
#ifdef SEC_CANARY
    printf("[   ]<     >: [canary_l](%lld)\n", cake->canary_left);
    printf("[   ]<     >: [canary_r](%lld)\n", cake->canary_right);
#endif
#endif
    printf("[   ]<     >: [size](%zu) [capacity](%zu)\n", STACK_GENERIC(size)(cake), STACK_GENERIC(capacity)(cake));
    printf("[   ]<.buf.>: [buffer](%p)\n", (void*) (cake->buffer));

    const size_t print_depth = min(cake->size, STACK_DUMP_DEPTH);
    for (size_t i = 0; i < print_depth; ++i) {
        printf("[   ]<%3zu  >: [](" STACK_VALUE_PRINTF_SPEC ")\n", print_depth - 1 - i, cake->buffer[cake->size - 1 - i]);
    }

    return OK;
}

int STACK_GENERIC(realloc)(STACK_GENERIC_TYPE *cake, const size_t new_capacity) {
    STACK_OK(cake);

    if (new_capacity < cake->size) {
         RETURN_ERROR_ASSERT(ERR_REALLOC_FAILED);
    }

    STACK_VALUE_TYPE *new_buffer = (STACK_VALUE_TYPE*) realloc(cake->buffer, sizeof(STACK_VALUE_TYPE) * new_capacity);
    if (!new_buffer) {
        RETURN_ERROR_ASSERT(ERR_REALLOC_FAILED);
    }

    cake->buffer   = new_buffer;
    cake->capacity = new_capacity;
    STACK_GENERIC(recalcute_security)(cake);
    STACK_OK(cake);

    return OK;
}

int STACK_GENERIC(push)(STACK_GENERIC_TYPE *cake, const STACK_VALUE_TYPE val) {
    STACK_OK(cake);

    if (STACK_GENERIC(is_full)(cake)) {
        RETURNING_ASSERT_OK(STACK_GENERIC(realloc)(cake, (double) STACK_GENERIC(capacity)(cake) * STACK_REALLOC_UP_COEF));
    }

    cake->buffer[cake->size++] = val;
    STACK_GENERIC(recalcute_security)(cake);
    STACK_OK(cake);

    return OK;
}

int STACK_GENERIC(pop)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);
    RETURNING_ASSERT(cake->size > 0);

    --cake->size;
    STACK_GENERIC(recalcute_security)(cake);

    if (cake->capacity / (cake->size + 1) > STACK_REALLOC_DOWN_COEF) {
        RETURNING_ASSERT(STACK_GENERIC(realloc)(cake, (double) STACK_GENERIC(capacity)(cake) / STACK_REALLOC_DOWN_COEF * 1.5) == 0);
    }

    STACK_GENERIC(recalcute_security)(cake);
    STACK_OK(cake);

    return OK;
}

int STACK_GENERIC(clear)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);

    size_t init_size = STACK_GENERIC(size)(cake);
    for (size_t i = 0; i < init_size; ++i) {
        RETURNING_ASSERT_OK(STACK_GENERIC(pop)(cake));
    }

    STACK_OK(cake);
    return OK;
}

size_t STACK_GENERIC(is_empty)(const STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);
    return STACK_GENERIC(size)(cake) == 0;
}

size_t STACK_GENERIC(is_full)(const STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);
    return cake->size == cake->capacity;
}
//=============================================================================
