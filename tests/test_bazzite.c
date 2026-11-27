/* Host-side test for src/bazzite.c: cc -Iinclude tests/test_bazzite.c src/bazzite.c */
#include "bazzite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures = 0;

#define CHECK(cond) do { if (!(cond)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static void touch(const char *dir, const char *name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    FILE *f = fopen(path, "w");
    if (f) fclose(f);
}

static int ends_with(const char *s, const char *suffix) {
    size_t a = strlen(s), b = strlen(suffix);
    return a >= b && strcmp(s + a - b, suffix) == 0;
}

static void test_parse(void) {
    unsigned v[BAZZITE_VERSION_PARTS];
    CHECK(bazzite_parse_version("bazzite-bootc-loader-v3.0.4.elf", v));
    CHECK(v[0] == 3 && v[1] == 0 && v[2] == 4 && v[3] == 0);
    CHECK(bazzite_parse_version("bazzite-bootc-loader-v3.0.elf", v));
    CHECK(v[0] == 3 && v[1] == 0 && v[2] == 0);
    CHECK(bazzite_parse_version("bazzite-bootc-loader-v12.elf", v) && v[0] == 12);

    CHECK(!bazzite_parse_version("bazzite-bootc-loader-v3.0.4.elf.sha256", v));
    CHECK(!bazzite_parse_version("bazzite-bootc-loader-v3.0.4.elf.inputs", v));
    CHECK(!bazzite_parse_version("bazzite-bootc-loader.elf", v));
    CHECK(!bazzite_parse_version("bazzite-bootc-loader-v.elf", v));
    CHECK(!bazzite_parse_version("bazzite-bootc-loader-v3..0.elf", v));
    CHECK(!bazzite_parse_version("bazzite-bootc-loader-v3.0-rc1.elf", v));
    CHECK(!bazzite_parse_version("bazzite-bootc-loader-v1.2.3.4.5.elf", v));
    CHECK(!bazzite_parse_version("other-v3.0.elf", v));
}

static void test_compare(void) {
    unsigned a[BAZZITE_VERSION_PARTS], b[BAZZITE_VERSION_PARTS];
    bazzite_parse_version("bazzite-bootc-loader-v3.0.10.elf", a);
    bazzite_parse_version("bazzite-bootc-loader-v3.0.9.elf", b);
    CHECK(bazzite_compare_versions(a, b) > 0);   /* numeric, not lexical */

    bazzite_parse_version("bazzite-bootc-loader-v3.0.elf", a);
    bazzite_parse_version("bazzite-bootc-loader-v3.0.1.elf", b);
    CHECK(bazzite_compare_versions(a, b) < 0);

    bazzite_parse_version("bazzite-bootc-loader-v3.0.elf", b);
    CHECK(bazzite_compare_versions(a, b) == 0);
}

static void test_find(void) {
    char dir[] = "/tmp/bazzite-test-XXXXXX";
    char out[512];
    if (!mkdtemp(dir)) { perror("mkdtemp"); failures++; return; }

    CHECK(bazzite_find_loader(dir, out, sizeof(out)) == -1);     /* empty */
    CHECK(bazzite_find_loader("/nonexistent-dir-xyz", out, sizeof(out)) == -1);

    touch(dir, "bazzite-bootc-loader-v3.0.elf");
    touch(dir, "bazzite-bootc-loader-v3.0.1.elf");
    touch(dir, "bazzite-bootc-loader-v3.0.4.elf");
    touch(dir, "bazzite-bootc-loader-v3.0.4.elf.sha256");
    touch(dir, "bazzite-bootc-loader-v3.0.9.elf");
    touch(dir, "bazzite-bootc-loader-v3.0.10.elf");
    touch(dir, "bazzite-bootc-loader-v2.16.2.elf");
    touch(dir, "bazzite-bootc-loader-v9.9.9.elf.inputs");

    /* a directory with a matching name must be ignored */
    char sub[600];
    snprintf(sub, sizeof(sub), "%s/bazzite-bootc-loader-v99.elf", dir);
    mkdir(sub, 0755);

    CHECK(bazzite_find_loader(dir, out, sizeof(out)) == 0);
    CHECK(ends_with(out, "/bazzite-bootc-loader-v3.0.10.elf"));

    /* unversioned name overrides the version scan */
    touch(dir, BAZZITE_LOADER_FIXED_NAME);
    CHECK(bazzite_find_loader(dir, out, sizeof(out)) == 0);
    CHECK(ends_with(out, "/" BAZZITE_LOADER_FIXED_NAME));

    char cmd[600];
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", dir);
    if (system(cmd) != 0) fprintf(stderr, "warning: cleanup failed\n");
}

int main(void) {
    test_parse();
    test_compare();
    test_find();
    if (failures) { fprintf(stderr, "%d failure(s)\n", failures); return 1; }
    printf("test_bazzite: all tests passed\n");
    return 0;
}
