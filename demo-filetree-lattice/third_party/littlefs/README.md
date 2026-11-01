# littlefs

Optional filesystem for the SD-card flight log and mission storage. The
scaffold only defines the `log_storage`/`storage` interfaces; no filesystem is
linked yet.

If you add littlefs, pin the upstream commit, provide the block-device glue in
the board HAL, and test power-loss recovery before trusting it with logs.
