# Contract: A2osX Mini Image Boot Failure Investigation

**Task ID:** xasm-qjh
**Priority:** P0
**Working Directory:** /Users/bryanw/Projects/Vintage/tools/xasm++
**Related Projects:** /Users/bryanw/Projects/Vintage/Apple/A2osX

## Symptom

A2osX mini image (800K ProDOS) boots to:
```
A2osX[Stage2]:Init
Relocating Kernel...
Reading Kernel Config File...Error, Using Default.
```
Then stops. Screen shows DLGR checkerboard with 4-line text window. Never reaches login prompt.

## Known Context

- "Reading Kernel Config File...Error, Using Default." is **NORMAL** on first boot (kconfig file doesn't exist until user runs /bin/kconfig). Boot should continue past this.
- The kernel binary (sys/kernel, 29639 bytes) was assembled Mar 8 with an old xasm++ — needs forced rebuild.
- After changing xasm++ binary, must delete `build/stage/sys/kernel` to force cmake reassembly.
- ETC/INIT starts 8 getty processes on /dev/tty1..tty8 with /sbin/login.
- A2osX source: /Users/bryanw/Projects/Vintage/Apple/A2osX/
- Mini image: /Users/bryanw/Projects/Vintage/Apple/A2osX/build/a2osx-mini.po
- STABLE reference: /Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po
- acx tool: java -jar ~/.local/share/acx/acx.jar

## Investigation Steps

1. Force kernel rebuild: rm build/stage/sys/kernel, then cmake --build .../A2osX/build
2. Binary diff new kernel vs old — meaningful differences?
3. Validate: no JMP $0000 in kernel binary
4. Read SYS/KERNEL.S.txt — trace what runs after config read, find hang point
5. Verify ETC/INIT in image with correct content
6. Verify sbin/getty and sbin/login present and non-zero size
7. Compare mini image file listing vs STABLE.800.po
8. Fix any xasm++ bugs found

## Acceptance Criteria

- Kernel rebuilt today with current xasm++
- Mini image rebuilt and updated
- Root cause of boot hang identified and fixed (or documented)
- xasm++ builds and all tests pass
