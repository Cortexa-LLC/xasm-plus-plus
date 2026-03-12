# Contract: Verify SYSCALL-only diffs and +2B x.fileenum batch

## Task ID
xasm-g8i

## Problem
Two groups of files need classification:

### Group A: SYSCALL-only +0B files (suspected source changes)
These files have same size but content differs — first diff bytes suggest SYSCALL
address changes ($1000→$0140 or $E200→$0153). Need confirmation.
- `bin/dnsinfo`: 26 diffs, v$2025 stable=$00 built=$40
- `bin/hmacmd5`: 62 diffs, v$201b stable=$00 built=$40
- `bin/md4`: 36 diffs, v$2017 stable=$00 built=$40
- `bin/md5`: 32 diffs, v$201b stable=$00 built=$40
- `bin/netstat`: 8 diffs, v$2045 stable=$00 built=$40
- `bin/rpcdump`: 44 diffs, v$202b stable=$00 built=$40
- `bin/xargs`: 30 diffs, v$2018 stable=$10 built=$50
- `sbin/gui`: 14 diffs, v$201f stable=$00 built=$40
- `sbin/vedd`: 50 diffs, v$2025 stable=$00 built=$40
- `sys/pm/pm.appletalk`: 5 diffs, v$2079 stable=$42 built=$00
- `bin/cc`: 8 diffs, v$51bf stable=$0a built=$fe
- `bin/ls`: 1 diff, v$216e stable=$c2 built=$c4
- `bin/acos`: 1 diff, v$3425 stable=$03 built=$fe
- `bin/edit`: 6 diffs, v$370a stable=$6c built=$00
- `bin/forth`: 4 diffs, v$2583 stable=$fe built=$b6

### Group B: +2B uniform batch (suspected x.fileenum.s source change)
12 files all with identical +2B delta (stable is 2 bytes larger):
bin/attr, bin/chaux, bin/chgrp, bin/chmod, bin/chown, bin/chtyp,
bin/cp, bin/lc, bin/pak, bin/rm, bin/uc, bin/wc
Also: lib/libblkdev (+1B)

## Investigation Method

### For Group A — SYSCALL pattern check:
```python
# For each file, check if ALL diffs match known SYSCALL/SLEEP address byte changes
SYSCALL_OLD, SYSCALL_NEW = 0x1000, 0x0140
SYSCALL2_OLD, SYSCALL2_NEW = 0xE200, 0x0153
SLEEP_OLD, SLEEP_NEW = 0x1010, 0x0150

def check_syscall_only(stable_path, built_path):
    s = open(stable_path,'rb').read()[16:]
    b = open(built_path,'rb').read()[16:]
    diffs = [(i,s[i],b[i]) for i in range(min(len(s),len(b))) if s[i]!=b[i]]
    
    for i,sv,bv in diffs:
        # SYSCALL lo/hi, SYSCALL2 lo/hi, SLEEP lo/hi
        pairs = [
            (SYSCALL_OLD&0xff, SYSCALL_NEW&0xff), (SYSCALL_OLD>>8, SYSCALL_NEW>>8),
            (SYSCALL2_OLD&0xff, SYSCALL2_NEW&0xff), (SYSCALL2_OLD>>8, SYSCALL2_NEW>>8),
            (SLEEP_OLD&0xff, SLEEP_NEW&0xff), (SLEEP_OLD>>8, SLEEP_NEW>>8),
        ]
        matched = any(sv==old and bv==new for old,new in pairs)
        if not matched:
            print(f"  UNEXPECTED diff at v${0x2000+i:04x}: stable=${sv:02x} built=${bv:02x}")
            return False
    return True  # all diffs explained by SYSCALL changes
```

### For Group B — verify x.fileenum.s change:
Check if ALL +2B files use the same common include (e.g. X.FileEnum or similar macro)
and if the include has exactly 2 more bytes in 335cd122 vs stable era source.
```bash
# Find the common include
head -20 /tmp/A2osX-335cd122/BIN/ATTR.S.txt
# Look for .INB or .USE of a common file
# Then check that file for +2B source change
```

## Knowledge Graph Instructions

**MANDATORY: Query KG before reading any source files:**
```
# xasm++ KG
mcp__kg__get_preflight_context({task: "verify SYSCALL-only diffs and x.fileenum +2B batch"})
mcp__kg__search_knowledge({query: "SYSCALL address change verification"})
mcp__kg__search_knowledge({query: "x.fileenum source change +2B"})
mcp__kg__search_knowledge({query: "bin/acos bin/edit bin/forth remaining diffs"})

# A2osX KG
mcp__kg__get_preflight_context({task: "SYSCALL address fileenum source changes", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "A2osX SYSCALL address constant change", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
mcp__kg__search_knowledge({query: "FileEnum x.fileenum shared include", project: "/Users/bryanw/Projects/Vintage/Apple/A2osX"})
```

**Write every finding** to KG with `[INVESTIGATION]` prefix.
**Write `[COMPLETION]` summary** before TaskComplete.

## Acceptance Criteria
- [ ] Each Group A file classified (SYSCALL-only vs other)
- [ ] Group B: x.fileenum.s source change confirmed
- [ ] Bug tracker updated: confirmed source changes moved to Known Source Version Diffs
- [ ] If any new assembler bug found: fix + 1834 tests pass

## Related Projects
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX
