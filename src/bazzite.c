#include "bazzite.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int bazzite_parse_version(const char *name, unsigned ver[BAZZITE_VERSION_PARTS]) {
    const size_t prefix_len = sizeof(BAZZITE_LOADER_PREFIX) - 1;
    if (strncmp(name, BAZZITE_LOADER_PREFIX, prefix_len) != 0)
        return 0;

    const char *p = name + prefix_len;
    memset(ver, 0, BAZZITE_VERSION_PARTS * sizeof(ver[0]));

    for (int part = 0; part < BAZZITE_VERSION_PARTS; part++) {
        if (*p < '0' || *p > '9')
            return 0;
        unsigned value = 0;
        while (*p >= '0' && *p <= '9') {
            if (value > 99999)          /* reject absurd numbers / overflow */
                return 0;
            value = value * 10 + (unsigned)(*p - '0');
            p++;
        }
        ver[part] = value;

        if (strcmp(p, ".elf") == 0)
            return 1;
        if (*p != '.')
            return 0;
        p++;
    }
    return 0;                           /* too many version components */
}

int bazzite_compare_versions(const unsigned a[BAZZITE_VERSION_PARTS],
                             const unsigned b[BAZZITE_VERSION_PARTS]) {
    for (int i = 0; i < BAZZITE_VERSION_PARTS; i++) {
        if (a[i] != b[i])
            return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

static int is_regular_file(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

int bazzite_find_loader(const char *dir, char *out_buf, size_t out_size) {
    char path[512];

    /* 1. Unversioned name lets the user pin a build by renaming it */
    snprintf(path, sizeof(path), "%s/%s", dir, BAZZITE_LOADER_FIXED_NAME);
    if (is_regular_file(path)) {
        snprintf(out_buf, out_size, "%s", path);
        return 0;
    }

    /* 2. Highest bazzite-bootc-loader-v*.elf */
    DIR *d = opendir(dir);
    if (!d)
        return -1;

    unsigned best[BAZZITE_VERSION_PARTS] = {0};
    unsigned ver[BAZZITE_VERSION_PARTS];
    int found = 0;
    struct dirent *entry;

    while ((entry = readdir(d)) != NULL) {
        if (!bazzite_parse_version(entry->d_name, ver))
            continue;
        if (found && bazzite_compare_versions(ver, best) <= 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if (!is_regular_file(path))
            continue;

        memcpy(best, ver, sizeof(best));
        snprintf(out_buf, out_size, "%s", path);
        found = 1;
    }
    closedir(d);

    return found ? 0 : -1;
}
