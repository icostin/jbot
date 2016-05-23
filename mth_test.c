#include "jbot.h"

typedef struct context_s context_t;
struct context_s
{
    zlx_mutex_xfc_t * mutex_xfc;
    zlx_mutex_t * mutex;
    uint64_t result;
    uint64_t inc_count;
    zlx_tid_t * tid_table;
};

/* worker *******************************************************************/
uint_fast8_t ZLX_CALL worker (void * arg)
{
    context_t * c = arg;
    uint64_t i;

    for (i = 0; i < c->inc_count; ++i)
    {
        c->mutex_xfc->lock(c->mutex);
        c->result++;
        c->mutex_xfc->unlock(c->mutex);
    }

    return 0;
}

/* test *********************************************************************/
static uint8_t ZLX_CALL test
(
    context_t * c,
    size_t nt,
    uint64_t ni,
    uint8_t fake_mutex
)
{
    size_t tx, itx;
    zlx_mth_status_t ts;

    ZLX_ARRAY_ALLOC(c->tid_table, hbs_ma, nt, "mth_inc_test.tid_table");
    if (!c->tid_table) E(2, "no mem for thread table\n");

    c->mutex_xfc = fake_mutex ? &zlx_nosup_mth_xfc.mutex : &hbs_mth_xfc.mutex;
    c->mutex = zlx_mutex_create(hbs_ma, c->mutex_xfc, "mth_inc_test.mutex");
    c->result = 0;
    c->inc_count = ni;

    for (tx = 0; tx < nt; ++tx)
    {
        ts = hbs_thread_create(c->tid_table + tx, worker, c);
        if (ts)
        {
            HBS_LE("mth-inc-test: failed to create worker thread #$z: "
                   "(error code $i)\n", tx, ts);
            break;
        }
    }
    for (itx = 0; itx < tx; ++itx)
    {
        ts = hbs_thread_join(c->tid_table[itx], NULL);
        if (ts)
        {
            HBS_LE("mth-inc-test: failed to join worker thread #$z: "
                   "(error code $i)\n", itx, ts);
        }
    }
    ZLX_ARRAY_FREE(c->tid_table, hbs_ma, nt);
    zlx_mutex_destroy(c->mutex, hbs_ma, c->mutex_xfc);

    return !(c->result == ni * nt);
}

/* mth_inc_test *************************************************************/
uint8_t ZLX_CALL mth_inc_test
(
    size_t nt,
    uint64_t ni,
    uint8_t fake_mutex
)
{
    context_t c;
    uint8_t r;

    r = test(&c, nt, ni, fake_mutex);
    zlx_fprint(hbs_out, "mth-inc-test: $z * $q -> $q $s\n", 
               nt, ni, c.result, r ? "FAILED" : "ok");

    return r;
}


