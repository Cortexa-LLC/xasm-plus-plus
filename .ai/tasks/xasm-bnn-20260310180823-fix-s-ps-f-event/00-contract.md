# Fix: Missing S.PS.F.EVENT symbol (20 files, diff@0x0006)

## Task ID
xasm-bnn

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/xasm-bnn-20260310180823-fix-s-ps-f-event/

## Context

20 files differ at byte offset 6 because S.PS.F.EVENT is undefined in the 335cd122 A2osX source.
Stable reference has value 0x02; xasm++ produces 0x00 (undefined symbol → 0).

## Step 1: Find where S.PS.F.EVENT should be defined

Search A2osX source:
```bash
grep -r "S\.PS\.F" /Users/bryanw/Projects/Vintage/Apple/A2osX/INC/ | head -20
grep -r "S\.PS\.F\.EVENT" /Users/bryanw/Projects/Vintage/Apple/A2osX/ --include="*.txt" | head -20
```

Look for other S.PS.F.* symbols to find the right file. Also check what file defines S.PS:
```bash
grep -r "S\.PS\b\|S\.PS\.F\b" /Users/bryanw/Projects/Vintage/Apple/A2osX/INC/ | head -20
```

## Step 2: Add the missing definition

Find the INC file that defines S.PS.F.* flags (e.g. a2osx.s.i, a2osx.i, or similar).
Add: `S.PS.F.EVENT .EQ 2`

The value 2 is confirmed from the stable binary (byte[6] = 0x02).

## Step 3: Verify ONE file fixes (no full rebuild needed for verification)

Pick a small affected file and rebuild just that target:
```bash
cd /Users/bryanw/Projects/Vintage/Apple/A2osX/build
make bin_aarp  # or any small affected target
xxd stage/bin/aarp | head -3  # check byte[6] is now 0x02
```

## Affected files (for reference)
BIN/AARP, BIN/ARP, BIN/ETCONFIG, BIN/GOPHER, BIN/HTTPGET, BIN/IPCONFIG,
BIN/IRC, BIN/KCONFIG, BIN/NTPDATE, BIN/PING, BIN/RPCDUMP, BIN/SHUTDOWN, BIN/TELNET,
SBIN/BBSD, SBIN/CIFSD, SBIN/GUI, SBIN/HTTPD, SBIN/NETWORKD, SBIN/TELNETD, SBIN/VEDD

## Deliverable

Write investigation-report.md:
- Where S.PS.F.EVENT was found (or wasn't found)
- What file was modified and what line was added
- Verification that byte[6] is now 0x02 for at least one file
