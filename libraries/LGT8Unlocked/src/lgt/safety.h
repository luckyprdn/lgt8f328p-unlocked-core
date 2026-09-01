#ifndef LGT8_UNLOCKED_SAFETY_H
#define LGT8_UNLOCKED_SAFETY_H

/*
 * Recovery-safe mode is ON by default for this handoff.
 *
 * It intentionally blocks operations that can:
 *  - modify program flash / bootloader contents,
 *  - disable the SWD/ISP recovery path,
 *  - relocate interrupt vectors,
 *  - switch the main clock source away from the known-good source,
 *  - enter DPS2 (state-loss / reset-like deep power mode),
 *  - enable reset-generating LVD configuration,
 *  - reload protected device configuration while running.
 *
 * Do NOT override this macro while you do not have a verified hardware
 * recovery path.  A future hardware-qualified build can opt in explicitly.
 */
#ifndef LGT8_UNLOCKED_RECOVERY_SAFE
#define LGT8_UNLOCKED_RECOVERY_SAFE 1
#endif

/* Recovery-safe exception: high-drive GPIO is independently opt-in. */
#ifndef LGT8_UNLOCKED_ALLOW_HIGH_DRIVE
#define LGT8_UNLOCKED_ALLOW_HIGH_DRIVE 0
#endif

#endif
