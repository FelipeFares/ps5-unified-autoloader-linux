# ps5-unified-autoloader (Linux Edition)

A standalone PS5 ELF payload that automates loading payloads specifically configured for Linux booting. This is intended for integration into the **WebKit Linux Loader** chain.

## What it does

When loaded via elfldr (e.g. as part of the WebKit Linux Loader):

1. **Returns to Home Screen** from the WebKit browser via `libSceShellUIUtil` so that WebKit closes cleanly.
2. **Kills entry point app / BD Disc Player** if running.
3. **Waits for elfldr** to be ready on port 9021.
4. **Looks for Linux configuration** (`autoload-linux` / `autoload-linux.txt`) in order:
   - **USB**: `/mnt/usb[0-7]/autoload-linux/autoload-linux.txt`
   - **USB**: `/mnt/usb[0-7]/autoload-linux/autoload-linux`
   - **USB**: `/mnt/usb[0-7]/autoload-linux.txt`
   - **USB**: `/mnt/usb[0-7]/autoload-linux`
   - **Internal**: `/data/autoload-linux/autoload-linux.txt`
   - **Internal**: `/data/autoload-linux/autoload-linux`
   - **Internal**: `/data/autoload-linux.txt`
   - **Internal**: `/data/autoload-linux`
   *(NOTE: Never looks for `autoload.txt`, completely preventing collisions with other loaders)*
5. **If config is found**: launches each payload listed in the config via `elfldr:9021`.
6. **If config is NOT found**: looks for the Bazzite bootc loader in
   `/data/pldmgr/payloads/local/bazzite-bootc-loader/` and launches it directly:
   - `bazzite-bootc-loader.elf` (unversioned name) if present — rename a build to this to pin it;
   - otherwise the **highest version** of `bazzite-bootc-loader-v<X.Y.Z>.elf`
     (compared numerically, so `v3.0.10` beats `v3.0.9`; `.sha256`/`.inputs` files are ignored).
   Just drop the new release ELF into that folder — no renaming or config edits needed.
7. **If neither is found**: falls back to starting the embedded **Payload Manager** for recovery.

## Config format (`autoload-linux` / `autoload-linux.txt`)

```text
# This is a comment — ignored
/data/pldmgr/payloads/local/bazzite-bootc-loader/bazzite-bootc-loader-v3.0.1.elf
```

- One entry per line
- Filenames starting with `/` are treated as absolute paths
- Lines starting with `!` are sleep commands: `!<ms>`
- Lines starting with `@` are directives (e.g. `@sync`)
