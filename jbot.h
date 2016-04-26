#ifndef _JBOT_H
#define _JBOT_H

#include <zlx.h>
#include <hbs.h>

#define S(_s) ((uint8_t const *) _s)

#define E(_rv, ...) do { zlx_fprint(hbs_err, __VA_ARGS__); return (_rv); } while (0)

uint8_t ZLX_CALL test_elal 
(
    size_t elem_size,
    uint32_t max_chain_len,
    size_t alloc_count,
    size_t free_count
);

#endif /* _JBOT_H */

