#pragma once

#include <stddef.h>

/* -----------------------------------------------------------------------
 * Bazzite bootc loader discovery
 *
 * Upstream (VivaLaVent/ps5-bazzite-deck) renames the ELF on every release:
 *   bazzite-bootc-loader-v3.0.elf, bazzite-bootc-loader-v3.0.1.elf, ...
 * so the fallback path cannot be hardcoded.
 * ----------------------------------------------------------------------- */

/* Directory Payload Manager keeps the Bazzite loader in */
#define BAZZITE_LOADER_DIR "/data/pldmgr/payloads/local/bazzite-bootc-loader"

/* Optional unversioned name: if present it always wins over versioned files */
#define BAZZITE_LOADER_FIXED_NAME "bazzite-bootc-loader.elf"

/* Versioned file name: BAZZITE_LOADER_PREFIX "<N>[.<N>...]" ".elf" */
#define BAZZITE_LOADER_PREFIX "bazzite-bootc-loader-v"

/* Max numeric components compared in a version (e.g. 3.0.4 uses 3) */
#define BAZZITE_VERSION_PARTS 4

/*
 * Parses a file name of the form "bazzite-bootc-loader-v<N>[.<N>...].elf".
 * On success fills ver[BAZZITE_VERSION_PARTS] (missing parts = 0) and
 * returns 1; returns 0 for any other name (e.g. *.elf.sha256).
 */
int bazzite_parse_version(const char *name, unsigned ver[BAZZITE_VERSION_PARTS]);

/* Compares two parsed versions numerically: <0, 0 or >0 like strcmp. */
int bazzite_compare_versions(const unsigned a[BAZZITE_VERSION_PARTS],
                             const unsigned b[BAZZITE_VERSION_PARTS]);

/*
 * Looks inside dir for the loader to launch:
 *   1. dir/BAZZITE_LOADER_FIXED_NAME, if it exists
 *   2. otherwise the highest-versioned bazzite-bootc-loader-v*.elf
 * Writes the full path into out_buf and returns 0, or -1 if none found.
 */
int bazzite_find_loader(const char *dir, char *out_buf, size_t out_size);
