#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <zlx.h>
#include <hbs.h>

HBS_MAIN(jbot_main);

#define S(_s) ((uint8_t const *) _s)

#define E(_rv, ...) do { fprintf(stderr, __VA_ARGS__); return (_rv); } while (0)

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
    if (hs) E(1, "fchunk error: open failed (hs: %u)\n", hs);

    pos = zlx_seek64(zf, ofs, ZLXF_SET);
    if (pos < 0)
    {
        zfs = hbs_file_close(zf);
        E(2, "fchunk error: seek failed (zfs: %u)\n", (int) -pos);
    }

    for (t = 0; t < len; )
    {
        size_t chunk_size = sizeof(buffer);
        if (t + chunk_size > len) chunk_size = (size_t) (len - t);
        r = zlx_read(zf, buffer, chunk_size);
        if (r < 0)
        {
            hbs_file_close(zf);
            E(4, "fchunk error: read error (zfs: %u)\n", (int) -r);
        }
        w = zlx_write(hbs_out, buffer, r);
        if (w < 0)
        {
            hbs_file_close(zf);
            E(5, "fchunk error: write error (zfs: %u)\n", (int) -w);
        }
        if ((size_t) r != chunk_size)
        {
            hbs_file_close(zf);
            E(6, "fchunk error: not enough data available (read 0x%"PRIX64")\n", t);
        }
        t += chunk_size;
    }
    zfs = hbs_file_close(zf);
    if (zfs) E(3, "fchunk error: close error (zfs: %u)\n", zfs);
    return 0;
}

/* jbot_main ****************************************************************/
uint8_t ZLX_CALL jbot_main (unsigned int argc, uint8_t const * const * argv)
{
    ptrdiff_t z;
    char const * logo = "jbot - just a bunch of tests\n";
    
    HBS_DM("argc: $i; hbs_ma=$p", argc, hbs_ma);
    ZLX_ASSERT(argc > 0);

    if (argc > 1 && !zlx_u8a_zcmp(argv[1], S("fchunk")))
    {
        uint64_t ofs, len;

        if (argc != 5) E(125, "invoke error: fchunk needs 3 args\n");

        if (zlx_u64_from_str(argv[3], zlx_u8a_zlen(argv[3]), 0, &ofs, NULL))
            E(125, "invoke error: bad number '%s'\n", argv[3]);
        if (zlx_u64_from_str(argv[4], zlx_u8a_zlen(argv[4]), 0, &len, NULL))
            E(125, "invoke error: bad number '%s'\n", argv[4]);
        return fchunk(argv[2], ofs, len);
    }

    z = zlx_write(hbs_out, logo, strlen(logo));
    if (z < 0) { fprintf(stderr, "write failed: %u\n", (int) -z); }

    return 0;
}

