#ifndef _JBOT_H
#define _JBOT_H

#include <zlx.h>
#include <hbs.h>

#define S(_s) ((uint8_t const *) _s)

#define E(_rv, ...) do { zlx_fprint(hbs_err, __VA_ARGS__); return (_rv); } while (0)

uint8_t ZLX_CALL elal_test
(
    size_t elem_size,
    uint32_t max_chain_len,
    size_t alloc_count,
    size_t free_count
);

/* mth_inc_test *************************************************************/
uint8_t ZLX_CALL mth_inc_test
(
    size_t nt,
    uint64_t ni,
    uint8_t fake_mutex
);

#endif /* _JBOT_H */

