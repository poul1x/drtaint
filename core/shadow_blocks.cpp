/**
 * \file
 * Implementation of shadow blocks
 */

#include "shadow_blocks.h"
#include "shadow_memory.h"

/**
 * Get count of elements in array \p arr at compile time
 */
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

/**
 * Size of machine word in bytes.
 * Can be 4 on 32bit system and 8 on 64bit.
 */
#define MACHINE_WORD_SIZE (sizeof(size_t))

/**
 * Get size of \p x in bits
 */
#define SIZE_IN_BITS(x) (x * N_BITS_PER_BYTE)

/**
 * Get size of \p x in bytes
 */
#define SIZE_IN_BYTES(x) (x / N_BITS_PER_BYTE)

// must be > 64 * N
#define APP_MEMORY_BUF_SIZE 512

/**
 * aaa
 */
#if SHADOW_IS_SCALE_DOWN
    #define SHADOW_SCALE (N_BITS_PER_BYTE / N_BITS_PER_SHADOW_BYTE)
#elif SHADOW_IS_SCALE_UP
    #define SHADOW_SCALE (N_BITS_PER_SHADOW_BYTE / N_BITS_PER_BYTE)
#else // SHADOW_IS_SCALE_SAME
    #define SHADOW_SCALE 1
#endif

/**
 * bbb
 */
#if SHADOW_IS_SCALE_DOWN
    #define GET_SHADOW_SIZE(app_size) (app_size / SHADOW_SCALE)
#elif SHADOW_IS_SCALE_UP
    #define GET_SHADOW_SIZE(app_size) (app_size * SHADOW_SCALE)
#else // SHADOW_IS_SCALE_SAME
    #define GET_SHADOW_SIZE(app_size) app_size
#endif

/**
 * ccc
 */
#if SHADOW_IS_SCALE_DOWN
    #define GET_APP_SIZE(shadow_size) (shadow_size * SHADOW_SCALE)
#elif SHADOW_IS_SCALE_UP
    #define GET_APP_SIZE(shadow_size) (shadow_size / SHADOW_SCALE)
#else // SHADOW_IS_SCALE_SAME
    #define GET_APP_SIZE(shadow_size) shadow_size
#endif

typedef bool (*sblocks_iter_read_block_func_t)(uint8_t *addr, size_t sblocks,
                                               size_t n_bits, void *user_data);

typedef void (*sblocks_iter_write_block_func_t)(uint8_t *addr, size_t *sblocks,
                                                size_t n_bits, void *user_data);

typedef struct _shadow_byte_read_iterator_t {
    sblocks_iter_read_byte_func_t iterate_func;
    void *user_data;
} shadow_byte_read_iterator_t;

typedef struct _shadow_byte_write_iterator_t {
    sblocks_iter_write_byte_func_t iterate_func;
    void *user_data;
} shadow_byte_write_iterator_t;

typedef struct _find_taint_byte_t {
    uint8_t *addr;
    taint_byte_t tbyte;
} find_taint_byte_t;

typedef struct _write_taint_t {
    taint_byte_t tbyte;
    size_t tblock;
} write_taint_t;

#pragma region UTILITY

// TODO: Think about this: On SCALE_DOWN_8X, X86 1 shadow byte holds
// 2 registers (4 bits). Unable to write one register to shadow byte
// GET_APP_SIZE(4 / 8) = 0

static void
read_shadow_memory(uint8_t *addr, void *buf, size_t shadow_size)
{
    size_t num_read = 0;
    size_t app_size = GET_APP_SIZE(shadow_size);
    smemory_read_shadow(addr, app_size, shadow_size, buf);
}

static void
write_shadow_memory(uint8_t *addr, void *buf, size_t shadow_size)
{
    size_t num_written = 0;
    size_t app_size = GET_APP_SIZE(shadow_size);
    smemory_write_shadow(addr, app_size, shadow_size, buf);
}

static size_t
read_shadow_memory_machine_word(uint8_t *addr)
{
    size_t res = 0;
    size_t num_read = 0;
    size_t shadow_size = MACHINE_WORD_SIZE;
    size_t app_size = GET_APP_SIZE(shadow_size);
    smemory_read_shadow(addr, app_size, shadow_size, (void *)&res);
    return res;
}

static void
write_shadow_memory_machine_word(uint8_t *addr, size_t val)
{
    size_t num_written = 0;
    size_t shadow_size = MACHINE_WORD_SIZE;
    size_t app_size = GET_APP_SIZE(shadow_size);
    smemory_write_shadow(addr, app_size, shadow_size, (void *)&val);
}

/**
 * \warning Only for internal use
 */
static size_t
read_taint_block_partial(size_t base_val, size_t n_bits)
{
    size_t res, n_bps, i;

    if (base_val == 0) {
        return 0;
    }

    DR_ASSERT(n_bits <= SIZE_IN_BITS(MACHINE_WORD_SIZE));

    res = 0;
    n_bps = N_BITS_PER_SHADOW_BYTE;

    for (i = 0; i < n_bits; i += n_bps)
        res |= (SHADOW_BYTE_MAX_VALUE << i);

    return base_val & res;
}

/**
 * \warning Only for internal use
 * \details Construct machine word sized taint value from provided
 * \p tbyte and \p base_val filling lower \p n_bits of \p base_val with
 * generated taint value. Lower \p n_bits of \p base_val will be filled with \p
 * tbyte. Rest higher bits of \p base_val will be untouched.
 *
 * \param[in] base_val Initial value which lower bits will be overwritten
 * \param[in] n_bits Count of bits to write. Must be in range of [0, #MACHINE_WORD_SIZE * 8]
 * \param[in] tbyte Value to use for machine word sized taint constructon.
 * \return Generated machine word sized taint value
 */
static size_t
create_taint_block_partial(size_t base_val, size_t n_bits, taint_byte_t tbyte)
{
    size_t val, res;
    size_t n_bps, i;

    DR_ASSERT(n_bits <= SIZE_IN_BITS(MACHINE_WORD_SIZE));

    if (tbyte == 0) {
        base_val >>= n_bits;
        base_val <<= n_bits;
        return base_val;
    }

    res = 0;
    val = tbyte;
    n_bps = N_BITS_PER_SHADOW_BYTE;

    for (i = 0; i < n_bits; i += n_bps)
        res |= (val << i);

    if (base_val == 0)
        return res;

    base_val >>= n_bits;
    base_val <<= n_bits;
    return base_val | res;
}

/**
 * \warning Only for internal use
 * \details Construct machine word sized taint value from provided \p tbyte.
 * All bits of generated taint value will be filled with value specified in \p
 * tbyte.
 * \param[in] tbyte Value to use for machine word sized taint
 * constructon.
 * \return Generated machine word sized taint value
 */
static size_t
create_taint_block(taint_byte_t tbyte)
{
    return create_taint_block_partial(0, SIZE_IN_BITS(MACHINE_WORD_SIZE), tbyte);
}

#pragma endregion UTILITY

#pragma region READ

static void
sblocks_iterate_app_read(uint8_t *addr, size_t app_size,
                         sblocks_iter_read_block_func_t iterate_func, void *user_data)
{
    size_t shadow_size_bits, shadow_size_bytes;
    size_t n_chunks, n_rest, n_block_bits;
    uint8_t *chunk_addr, *block_addr;
    size_t shadow;
    size_t i, j;

    //
    // Buffer for reading application memory.
    // Stored as set of machine words
    //

    size_t buf[GET_SHADOW_SIZE(APP_MEMORY_BUF_SIZE / sizeof(size_t))];

    //
    // Check taint value can be read into machine word
    //

    shadow_size_bits = GET_SHADOW_SIZE(SIZE_IN_BITS(app_size));
    if (shadow_size_bits <= SIZE_IN_BITS(MACHINE_WORD_SIZE)) {
        shadow = read_shadow_memory_machine_word(addr);
        iterate_func(addr, shadow, shadow_size_bits, user_data);
        return;
    }

    //
    // If taint value can not be read into machine word,
    // use buffer for reading shadow memory
    //

    n_block_bits = SIZE_IN_BITS(MACHINE_WORD_SIZE);
    shadow_size_bytes = SIZE_IN_BYTES(shadow_size_bits);
    n_chunks = shadow_size_bytes / sizeof(buf);
    n_rest = shadow_size_bytes % sizeof(buf);

    for (i = 0; i < n_chunks; i++) {
        chunk_addr = &addr[GET_APP_SIZE(i * sizeof(buf))];
        read_shadow_memory(chunk_addr, buf, sizeof(buf));

        for (j = 0; j < ARRAY_LENGTH(buf); j++) {
            block_addr = &chunk_addr[GET_APP_SIZE(j * MACHINE_WORD_SIZE)];
            if (!iterate_func(block_addr, buf[j], n_block_bits, user_data))
                return;
        }
    }

    //
    // Read rest bytes which didn't fit into buffer
    //

    if (n_rest > 0) {
        chunk_addr = &addr[GET_APP_SIZE(i * sizeof(buf))];
        n_chunks = n_rest / MACHINE_WORD_SIZE;
        n_rest = n_rest % MACHINE_WORD_SIZE;

        read_shadow_memory(chunk_addr, buf, sizeof(buf));

        for (j = 0; j < n_chunks; j++) {
            block_addr = &chunk_addr[GET_APP_SIZE(j * MACHINE_WORD_SIZE)];
            if (!iterate_func(block_addr, buf[j], n_block_bits, user_data))
                return;
        }

        if (n_rest > 0) {
            block_addr = &chunk_addr[GET_APP_SIZE(j * MACHINE_WORD_SIZE)];
            if (!iterate_func(block_addr, buf[j], SIZE_IN_BITS(n_rest), user_data))
                return;
        }
    }

#if SHADOW_IS_SCALE_DOWN

    //
    // Read rest bits.
    // Can be possible only when count of
    // bits per shadow byte is less than 8
    //

    n_chunks = shadow_size_bits / N_BITS_PER_BYTE;
    n_rest = shadow_size_bits % N_BITS_PER_BYTE;

    if (n_rest) {
        chunk_addr = &addr[GET_APP_SIZE(n_chunks)];
        shadow = read_shadow_memory_machine_word(chunk_addr);
        iterate_func(chunk_addr, shadow, n_rest, user_data);
    }

#endif
}

#pragma region ITERATOR

static bool
sblocks_iter_find_zero_taint(uint8_t *addr, size_t shadow_block, size_t n_shadow_bits,
                             void *user_data)
{
    size_t taint = n_shadow_bits < SIZE_IN_BITS(MACHINE_WORD_SIZE)
                       ? read_taint_block_partial(shadow_block, n_shadow_bits)
                       : shadow_block;

    if (taint == 0) {
        *(uint8_t **)user_data = addr;
        return false;
    }

    return true;
}

static bool
sblocks_iter_find_non_zero_taint(uint8_t *addr, size_t shadow_block, size_t n_shadow_bits,
                                 void *user_data)
{
    size_t taint = n_shadow_bits < SIZE_IN_BITS(MACHINE_WORD_SIZE)
                       ? read_taint_block_partial(shadow_block, n_shadow_bits)
                       : shadow_block;

    if (taint != 0) {
        *(uint8_t **)user_data = addr;
        return false;
    }

    return true;
}

static bool
sblocks_iter_find_non_zero_byte(uint8_t *addr, taint_byte_t tbyte, void *user_data)
{
    find_taint_byte_t *find = (find_taint_byte_t *)user_data;
    if (tbyte == find->tbyte) {
        find->addr = addr;
        return false;
    }

    return true;
}

static bool
sblocks_iter_shadow_bytes_read(uint8_t *addr, size_t shadow_block, size_t n_shadow_bits,
                               void *user_data)
{
    size_t i, j;
    taint_byte_t tbyte = 0;
    size_t n_bps = N_BITS_PER_SHADOW_BYTE;
    shadow_byte_read_iterator_t *it = (shadow_byte_read_iterator_t *)user_data;

    DR_ASSERT(n_shadow_bits <= SIZE_IN_BITS(MACHINE_WORD_SIZE));

    for (i = 0, j = 0; i < n_shadow_bits; i += n_bps, j++) {
        tbyte = (taint_byte_t)((shadow_block >> i) & SHADOW_BYTE_MAX_VALUE);
        if (!it->iterate_func(&addr[j], tbyte, it->user_data))
            return false;
    }

    return true;
}

#pragma endregion ITERATOR

#pragma region API

bool
sblocks_is_app_zero_tainted(uint8_t *addr, size_t app_size)
{
    uint8_t *found_addr = NULL;
    sblocks_iterate_app_read(addr, app_size, sblocks_iter_find_zero_taint, &found_addr);
    return found_addr != NULL;
}

bool
sblocks_is_app_non_zero_tainted(uint8_t *addr, size_t app_size)
{
    uint8_t *found_addr = NULL;
    sblocks_iterate_app_read(addr, app_size, sblocks_iter_find_non_zero_taint,
                             &found_addr);
    return found_addr != NULL;
}

void
sblocks_iterate_app_taint_read(uint8_t *addr, size_t size,
                               sblocks_iter_read_byte_func_t iterate_func,
                               void *user_data)
{
    shadow_byte_read_iterator_t iterator;
    iterator.iterate_func = iterate_func;
    iterator.user_data = user_data;

    sblocks_iterate_app_read(addr, size, sblocks_iter_shadow_bytes_read, &iterator);
}

bool
sblocks_is_app_non_zero_tainted_ex(uint8_t *addr, size_t size, size_t tbyte)
{
    find_taint_byte_t find;
    find.tbyte = (taint_byte_t)tbyte;
    find.addr = NULL;

    DR_ASSERT(tbyte > SHADOW_BYTE_MIN_VALUE && tbyte <= SHADOW_BYTE_MAX_VALUE);
    sblocks_iterate_app_taint_read(addr, size, sblocks_iter_find_non_zero_byte, &find);
    return find.addr != NULL;
}

#pragma endregion API

#pragma endregion READ

#pragma region WRITE

void
sblocks_iterate_app_write(uint8_t *addr, size_t app_size,
                          sblocks_iter_write_block_func_t iterate_func, void *user_data)
{

    size_t shadow_size_bits, shadow_size_bytes;
    size_t n_chunks, n_rest, n_block_bits;
    uint8_t *chunk_addr, *block_addr;
    size_t shadow;
    size_t i, j;

    //
    // Buffer for tainting application memory
    // Stored as set of machine words
    //

    size_t buf[GET_SHADOW_SIZE(APP_MEMORY_BUF_SIZE / sizeof(size_t))];

    //
    // Check taint value can be placed to machine word
    //

    shadow_size_bits = GET_SHADOW_SIZE(SIZE_IN_BITS(app_size));
    if (shadow_size_bits <= SIZE_IN_BITS(MACHINE_WORD_SIZE)) {
        shadow = read_shadow_memory_machine_word(addr);
        iterate_func(addr, &shadow, shadow_size_bits, user_data);
        write_shadow_memory_machine_word(addr, shadow);
        return;
    }

    //
    // If taint value can not be placed to machine word,
    // use buffer for writing to shadow memory
    //

    n_block_bits = SIZE_IN_BITS(MACHINE_WORD_SIZE);
    shadow_size_bytes = SIZE_IN_BYTES(shadow_size_bits);
    n_chunks = shadow_size_bytes / sizeof(buf);
    n_rest = shadow_size_bytes % sizeof(buf);

    for (i = 0; i < n_chunks; i++) {
        chunk_addr = &addr[GET_APP_SIZE(i * sizeof(buf))];
        read_shadow_memory(chunk_addr, buf, sizeof(buf));

        for (j = 0; j < ARRAY_LENGTH(buf); j++) {
            block_addr = &chunk_addr[GET_APP_SIZE(j * MACHINE_WORD_SIZE)];
            iterate_func(block_addr, &buf[j], n_block_bits, user_data);
        }

        write_shadow_memory(chunk_addr, buf, sizeof(buf));
    }

    //
    // Write rest bytes which didn't fit into buffer
    //

    if (n_rest > 0) {
        chunk_addr = &addr[GET_APP_SIZE(i * sizeof(buf))];
        n_chunks = n_rest / MACHINE_WORD_SIZE;
        n_rest = n_rest % MACHINE_WORD_SIZE;

        read_shadow_memory(chunk_addr, buf, sizeof(buf));

        for (j = 0; j < n_chunks; j++) {
            block_addr = &chunk_addr[GET_APP_SIZE(j * MACHINE_WORD_SIZE)];
            iterate_func(block_addr, &buf[j], n_block_bits, user_data);
        }

        if (n_rest > 0) {
            block_addr = &chunk_addr[GET_APP_SIZE(j * MACHINE_WORD_SIZE)];
            iterate_func(block_addr, &buf[j], SIZE_IN_BITS(n_rest), user_data);
        }

        write_shadow_memory(chunk_addr, buf, sizeof(buf));
    }

#if SHADOW_IS_SCALE_DOWN

    //
    // Write rest bits.
    // Can be possible only when count of
    // bits per shadow byte is less than 8
    //

    n_chunks = shadow_size_bits / N_BITS_PER_BYTE;
    n_rest = shadow_size_bits % N_BITS_PER_BYTE;

    if (n_rest > 0) {
        chunk_addr = &addr[GET_APP_SIZE(n_chunks)];
        shadow = read_shadow_memory_machine_word(chunk_addr);
        iterate_func(chunk_addr, &shadow, n_rest, user_data);
        write_shadow_memory_machine_word(chunk_addr, shadow);
    }

#endif
}

#pragma region ITERATOR

static void
sblocks_iter_write(uint8_t *addr, size_t *shadow_block, size_t n_shadow_bits,
                   void *user_data)
{
    write_taint_t *data = (write_taint_t *)user_data;
    size_t res = *shadow_block;

    res = n_shadow_bits >= SIZE_IN_BITS(MACHINE_WORD_SIZE)
              ? create_taint_block_partial(res, n_shadow_bits, data->tbyte)
              : data->tblock;

    *shadow_block = res;
}

static void
sblocks_iter_shadow_bytes_write(uint8_t *addr, size_t *shadow_block, size_t n_shadow_bits,
                                void *user_data)
{
    size_t i, j;
    size_t tbyte;
    size_t n_bps = N_BITS_PER_SHADOW_BYTE;
    shadow_byte_write_iterator_t *it = (shadow_byte_write_iterator_t *)user_data;

    DR_ASSERT(n_shadow_bits <= SIZE_IN_BITS(MACHINE_WORD_SIZE));

    for (i = 0, j = 0; i < n_shadow_bits; i += n_bps, j++) {

        // Get i-th taint byte value from
        // shadow block and pass it to iterator func
        tbyte = ((*shadow_block) & (SHADOW_BYTE_MAX_VALUE << i)) >> i;

        it->iterate_func(&addr[j], &tbyte, it->user_data);
        DR_ASSERT(tbyte >= SHADOW_BYTE_MIN_VALUE && tbyte <= SHADOW_BYTE_MAX_VALUE);

        // Set modified i-th taint byte value back to shadow block
        *shadow_block |= ((tbyte & SHADOW_BYTE_MAX_VALUE) << i);
    }
}

#pragma endregion ITERATOR

#pragma region API

void
sblocks_set_app_non_zero_taint(uint8_t *addr, size_t size, size_t tbyte)
{
    write_taint_t data;

    DR_ASSERT(tbyte > SHADOW_BYTE_MIN_VALUE && tbyte <= SHADOW_BYTE_MAX_VALUE);

    data.tbyte = (taint_byte_t)tbyte;
    data.tblock = create_taint_block(data.tbyte);
    sblocks_iterate_app_write(addr, size, sblocks_iter_write, &data);
}

void
sblocks_set_app_zero_taint(uint8_t *addr, size_t size)
{
    write_taint_t data = { 0 };
    sblocks_iterate_app_write(addr, size, sblocks_iter_write, &data);
}

void
sblocks_iterate_app_taint_write(uint8_t *addr, size_t size,
                                sblocks_iter_write_byte_func_t iterate_func,
                                void *user_data)
{
    shadow_byte_write_iterator_t iterator;
    iterator.user_data = user_data;
    iterator.iterate_func = iterate_func;
    sblocks_iterate_app_write(addr, size, sblocks_iter_shadow_bytes_write, &iterator);
}

#pragma endregion API

#pragma endregion WRITE