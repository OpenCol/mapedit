/* msc_selftest.c -- port/msc_runtime.c behaves as Microsoft C 6.00A's library did where the
 * editor can tell: 16-bit numbers, DOS file names, text mode, MSC's FILE record. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MSC_RUNTIME_IMPL
#include "msc.h"

void msc_set_root(const char *dir);

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static void write_file(const char *dir, const char *name, const char *bytes, size_t n)
{
    char path[1024];
    FILE *f;
    snprintf(path, sizeof path, "%s/%s", dir, name);
    f = fopen(path, "wb");
    fwrite(bytes, 1, n, f);
    fclose(f);
}

int main(void)
{
    char buf[64], dir[] = "/tmp/msc_selftest_XXXXXX";
    MSC_FILE *f;
    struct find_t ft;
    int count;

    /* numbers */
    CHECK(strcmp(msc_itoa(-5, buf, 10), "-5") == 0, "itoa -5: %s", buf);
    CHECK(strcmp(msc_itoa(-1, buf, 16), "ffff") == 0, "itoa -1 hex is 16 bits: %s", buf);
    CHECK(strcmp(msc_itoa(5, buf, 2), "101") == 0, "itoa 5 binary: %s", buf);
    CHECK(strcmp(msc_ltoa(-1, buf, 16), "ffffffff") == 0, "ltoa -1 hex: %s", buf);
    CHECK(strcmp(msc_ltoa(123456, buf, 10), "123456") == 0, "ltoa: %s", buf);
    CHECK(msc_atoi("  -42x") == -42, "atoi");
    CHECK(msc_atoi("70000") == (i16)(u16)70000, "atoi wraps at 16 bits: %d", msc_atoi("70000"));
    CHECK(msc_strnicmp("MapEdit", "MAPEDIT", 7) == 0, "strnicmp");
    CHECK(strcmp(msc_strupr(strcpy(buf, "a.mp")), "A.MP") == 0, "strupr");
    CHECK((_ctype + 1)['a'] & _LOWER && (_ctype + 1)['F'] & _HEX && !((_ctype + 1)[0xe9] & 0xff), "ctype");

    /* files: DOS names in the root, without regard to case */
    if (!mkdtemp(dir)) {
        printf("FAIL mkdtemp\n");
        return 1;
    }
    msc_set_root(dir);
    write_file(dir, "MapEdit.Txt", "one\r\ntwo\r\n\x1athree", 15);
    f = msc_fopen("C:\\MPS\\COLONIZE\\MAPEDIT.TXT", "rt");
    CHECK(f != NULL, "fopen through a DOS path, another case");
    if (f) {
        CHECK(msc_fgets(buf, sizeof buf, f) && strcmp(buf, "one\n") == 0, "text mode drops CR: '%s'", buf);
        CHECK(msc_fgets(buf, sizeof buf, f) && strcmp(buf, "two\n") == 0, "second line");
        CHECK(msc_fgets(buf, sizeof buf, f) == NULL, "Ctrl-Z ends a text file");
        msc_fclose(f);
    }
    f = msc_fopen("mapedit.txt", "rb");
    CHECK(f != NULL, "binary open");
    if (f) {
        CHECK(msc_fread(buf, 1, 5, f) == 5 && memcmp(buf, "one\r\n", 5) == 0, "binary keeps CR");
        /* MSC's FILE record: the buffer holds the rest, and the library reads it directly */
        CHECK(f->_cnt == 10 && *f->_ptr == 't', "_cnt %d, _ptr '%c'", f->_cnt, *f->_ptr);
        msc_fclose(f);
    }

    /* writing: a new file is created in upper case; text mode writes CR-LF */
    f = msc_fopen("new.mp", "wt");
    CHECK(f != NULL, "create");
    if (f) {
        msc_fwrite("a\nb", 1, 3, f);
        msc_fclose(f);
        f = msc_fopen("NEW.MP", "rb");
        CHECK(f && msc_fread(buf, 1, 10, f) == 4 && memcmp(buf, "a\r\nb", 4) == 0, "CR-LF on write");
        if (f)
            msc_fclose(f);
        snprintf(buf, sizeof buf, "%s/NEW.MP", dir);
        CHECK(access(buf, F_OK) == 0, "created as NEW.MP");
    }

    /* _dos_findfirst: DOS wildcards */
    write_file(dir, "amer2.mp", "x", 1);
    count = 0;
    if (msc__dos_findfirst("*.MP", _A_NORMAL, &ft) == 0) {
        do {
            count++;
            CHECK(strcmp(ft.name, "NEW.MP") == 0 || strcmp(ft.name, "AMER2.MP") == 0, "found %s", ft.name);
        } while (msc__dos_findnext(&ft) == 0);
    }
    CHECK(count == 2, "*.MP found %d files", count);

    msc_remove("new.mp");
    msc_remove("amer2.mp");
    msc_remove("mapedit.txt");
    rmdir(dir);
    printf("%s (%d failure%s)\n", failures ? "FAILED" : "msc run-time self-test passed", failures,
           failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
