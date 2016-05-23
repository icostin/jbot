#include "jbot.h"

static uint8_t ZLX_CALL test
(
    zlx_elal_t * ZLX_RESTRICT ea,
    void * * etab,
    size_t alloc_count,
    size_t free_count
)
{
    size_t i, n;

    if (!etab) E(1, "error: no mem for elal-test\n");

    for (i = 0; i < alloc_count; ++i)
    {
        etab[i] = zlx_elal_alloc(ea, "elem");
        if (!etab[i]) E(2, "error: elal alloc failed after $z elements\n", i);
    }

    for (i = 0; i < free_count; ++i)
    {
        zlx_elal_free(ea, etab[i]);
        if (ea->chain_len != (i + 1 > ea->max_chain_len ? ea->max_chain_len : i + 1))
            E(3, "error: unexpected chain len of $z "
              "after freeing element #$z\n",
              ea->chain_len, i);
    }

    n = ea->chain_len;

    for (i = 0; i < free_count; ++i)
    {
        if ((i <= n && ea->chain_len != n - i)
            || (i > n && ea->chain_len != 0))
            E(4, "error: unexpected chain len when reallocating item #$z\n", i);
        etab[i] = zlx_elal_alloc(ea, "elem2");
        if (!etab[i]) E(2, "error: elal alloc failed after $z elements\n", i);
    }
    if ((i <= n && ea->chain_len != n - i)
        || (i > n && ea->chain_len != 0))
    E(4, "error: unexpected chain len when reallocating item #$z\n", i);

    for (i = 0; i < alloc_count; ++i)
    {
        zlx_elal_free(ea, etab[i]);
        if (ea->chain_len != (i + 1 > ea->max_chain_len ? ea->max_chain_len : i + 1))
            E(3, "error: unexpected chain len of $z "
              "after freeing element #$z\n",
              ea->chain_len, i);
    }

    return 0;
}

uint8_t ZLX_CALL elal_test
(
    size_t elem_size,
    uint32_t max_chain_len,
    size_t alloc_count,
    size_t free_count
)
{
    zlx_elal_t ea;
    unsigned int s;
    void * * etab;

    if (free_count > alloc_count) free_count = alloc_count;

    s = zlx_elal_init(&ea, hbs_ma, NULL, NULL, elem_size, max_chain_len);
    if (s) E(1, "elal-test: failed to init elal mutex\n");

    etab = hbs_alloc(sizeof(void *) * alloc_count, "etab");
    s = test(&ea, etab, alloc_count, free_count);
    zlx_elal_finish(&ea);
    if (etab) hbs_free(etab, sizeof(void *) * alloc_count);

    return s;
}

