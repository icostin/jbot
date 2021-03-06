#include <zlx.h>
#include <hbs.h>
#include "jbot.h"

HBS_MAIN(jbot_main);

/* fchunk *******************************************************************/
uint8_t ZLX_CALL fchunk (uint8_t const * path, uint64_t ofs, uint64_t len)
{
    uint8_t buffer[0x1000];
    zlx_file_t * zf;
    zlx_file_status_t zfs;
    hbs_status_t hs;
    int64_t pos;
    ptrdiff_t r, w;
    uint64_t t;

    hs = hbs_file_open_ro(&zf, path);
    if (hs) E(1, "fchunk error: failed to open \"$es\" (hs: $i)\n", path, hs);

    pos = zlx_seek64(zf, ofs, ZLXF_SET);
    if (pos < 0)
    {
        zfs = hbs_file_close(zf);
        E(2, "fchunk error: seek failed (zfs: $i)\n", (int) -pos);
    }

    for (t = 0; t < len; )
    {
        size_t chunk_size = sizeof(buffer);
        if (t + chunk_size > len) chunk_size = (size_t) (len - t);
        r = zlx_read(zf, buffer, chunk_size);
        if (r < 0)
        {
            hbs_file_close(zf);
            E(4, "fchunk error: read error (zfs: $i)\n", (int) -r);
        }
        w = zlx_write(hbs_out, buffer, r);
        if (w < 0)
        {
            hbs_file_close(zf);
            E(5, "fchunk error: write error (zfs: $i)\n", (int) -w);
        }
        t += r;
        if ((size_t) r != chunk_size)
        {
            hbs_file_close(zf);
            E(6, "fchunk error: not enough data available (read $q=$xq)\n",
              t, t);
        }
    }
    zfs = hbs_file_close(zf);
    if (zfs) E(3, "fchunk error: close error (zfs: $i)\n", zfs);
    return 0;
}

/* logo *********************************************************************/
uint8_t logo ()
{
    return 0 > zlx_fprint(hbs_out,
 "jbot - just a bunch of tests -- ver 0.00\n");
}

/* ver **********************************************************************/
uint8_t ver ()
{
    if (logo()) return 1;
    if (zlx_fprint(hbs_out, "using $s\nusing $s\n",
                   zlx_lib_name, hbs_lib_name) < 0)
        return 1;
    return 0;
}


/* help *********************************************************************/
uint8_t help ()
{
    if (logo()) return 1;
    return 0 > zlx_fprint(hbs_out,
   "usage: jbot CMD [OPTS] ARGS\n"
   "commands:\n"
   "  help                      prints this\n"
   "  version                   prints the version of this tool\n"
   "  fchunk FILE OFS LEN       outputs a chunk from a file\n"
   "  elal-test ES MCL AC FC    tests element lookaside list allocator\n"
   "  mth-inc-test [-x] T I     T threads each increment I times a global;\n"
   "                            use '-x' switch to use dummy mutex locking\n"
   "options:\n"
   " -h --help                  prints this and exits\n"
   "    --version               prints the version of this tool and exits\n"
   );
}

/* run **********************************************************************/
static uint8_t ZLX_CALL run
(
    unsigned int argc,
    uint8_t const * const * argv,
    uint8_t const * * a
)
{
    unsigned int n, i, j, parse_opts = 1;
    uint8_t const * cmd = NULL;

    for (n = 0, i = 1; i < argc; ++i)
    {
        if (parse_opts && argv[i][0] == '-')
        {
            if (argv[i][1] == '-')
            {
                if (argv[i][2] == 0) { parse_opts = 0; continue; }
                /* long option */
                if (!zlx_u8a_zcmp(&argv[i][2], S("help"))) return help();
                if (!zlx_u8a_zcmp(&argv[i][2], S("version"))) return ver();
                E(124, "invoke error: unknown long option '$es'\n", argv[i]);
            }
            for (j = 1; argv[i][j]; ++j)
                switch (argv[i][j])
                {
                case 'h':
                    return help();
                }
        }
        if (!cmd) cmd = argv[i];
        else a[n++] = argv[i];
    }

    if (!cmd || !zlx_u8a_zcmp(cmd, S("help"))) return help();
    if (!zlx_u8a_zcmp(cmd, S("ver"))) return ver();

    if (!zlx_u8a_zcmp(cmd, S("fchunk")))
    {
        uint64_t ofs, len;

        if (n != 3) E(125, "invoke error: fchunk needs 3 args\n");

        if (zlx_u64_from_str(a[1], zlx_u8a_zlen(a[1]), 0, &ofs, NULL))
            E(125, "invoke error: bad number '$es'\n", a[1]);
        if (zlx_u64_from_str(a[2], zlx_u8a_zlen(a[2]), 0, &len, NULL))
            E(125, "invoke error: bad number '$s'\n", a[2]);
        return fchunk(a[0], ofs, len);
    }
    else if (!zlx_u8a_zcmp(cmd, S("elal-test")))
    {
        uint64_t elem_size, mcl, ac, fc;
        if (n != 4) E(125, "invoke error: elal-test needs 4 args\n");
        if (zlx_u64_from_str(a[0], zlx_u8a_zlen(a[0]), 0, &elem_size, NULL))
            E(125, "invoke error: bad number '$es'\n", a[0]);
        if (zlx_u64_from_str(a[1], zlx_u8a_zlen(a[1]), 0, &mcl, NULL))
            E(125, "invoke error: bad number '$es'\n", a[1]);
        if (zlx_u64_from_str(a[2], zlx_u8a_zlen(a[2]), 0, &ac, NULL))
            E(125, "invoke error: bad number '$es'\n", a[2]);
        if (zlx_u64_from_str(a[3], zlx_u8a_zlen(a[3]), 0, &fc, NULL))
            E(125, "invoke error: bad number '$es'\n", a[3]);
        return elal_test((size_t) elem_size, (uint32_t) mcl,
                         (size_t) ac, (size_t) fc);
    }
    else if (!zlx_u8a_zcmp(cmd, S("mth-inc-test")))
    {
        uint8_t fake_mutex = 0;
        uint64_t th_count, inc_count;
        if (n && !zlx_u8a_zcmp(a[0], S("-x"))) { fake_mutex = 1; --n; ++a; }
        if (n != 2) E(125, "invoke error: mth-inc-test needs 2 counters\n");
        if (zlx_u64_from_str(a[0], zlx_u8a_zlen(a[0]), 0, &th_count, NULL))
            E(125, "invoke error: bad number '$es'\n", a[0]);
        if (zlx_u64_from_str(a[1], zlx_u8a_zlen(a[1]), 0, &inc_count, NULL))
            E(125, "invoke error: bad number '$es'\n", a[1]);
        return mth_inc_test(th_count, inc_count, fake_mutex);
    }

    return 0;
}

/* jbot_main ****************************************************************/
uint8_t ZLX_CALL jbot_main (unsigned int argc, uint8_t const * const * argv)
{
    uint8_t r;
    uint8_t const * * a;

    HBS_DM("argc: $i; hbs_ma=$p", argc, hbs_ma);
    ZLX_ASSERT(argc > 0);

    a = hbs_alloc(argc * sizeof(uint8_t const *), "cmd args");
    if (!a) E(125, "error: no mem for processing arguments\n");

    r = run(argc, argv, a);

    hbs_free(a, argc * sizeof(uint8_t const *));
    return r;
}

