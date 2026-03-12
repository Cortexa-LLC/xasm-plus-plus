# Fix Branch Relaxation False-Positives: dhgr.drv, pppssc.drv, bin files

**Task ID:** xasm-oop-20260311-branch-relax-fix
**Workflow:** Bugfix
**Working Directory:** `/Users/bryanw/Projects/Vintage/tools/xasm++`
**Related Projects:** /Users/bryanw/Projects/Vintage/Apple/A2osX

---

## Task Description

Fix xasm++ so that branches at the 127-byte limit are NOT falsely relaxed when
assembling A2osX source, making dhgr.drv, pppssc.drv, and several bin/* files
byte-identical to the STABLE.800.po reference image.

### Background

SCMASM does NOT do branch relaxation — it errors on out-of-range branches.
The stable A2osX binaries have branches at exactly 127 bytes (`B0 7F` etc).
xasm++ incorrectly relaxes these to `B!cc +3 / JMP target` (5 bytes vs 2),
producing +3 byte deltas. **mkboard.drv is already fixed** by the ZP correction
in `src/syntax/scmasm_syntax.cpp` ~line 1157.

### Current State

| File | Delta |
|------|-------|
| drv/dhgr.drv    | +3 |
| drv/pppssc.drv  | +3 |
| bin/sh          | +3 |
| bin/mv          | +4 |
| bin/cp          | +2 |
| bin/du          | +2 |
| bin/csh         | +1 |
| bin/forth       | +2 |

---

## dhgr.drv Investigation Findings

- Source: `/tmp/A2osX-335cd122/DRV/DHGR.DRV.S.BLT.txt`
- `BITBLT.0:` at line 85, `bne .1` at line 90, `.1:` at line 97
- Gap between BNE and .1 is **9 bytes**: `tya`(1B) + `>SYSCALL2 GetStkObjPtr`(5B) + `sta IO.SETWRITEAUX`(3B)
- SYSCALL2 = `ldx #_x`(2B) + `jsr A2osX.SYSCALL2`(3B) — **no ZP instructions in this gap**
- So the ZP fix alone does NOT fix dhgr.drv
- The BEQ at code+0x03D6 (`bit #S.CB.OP.SAVE / beq BITBLT.1`) also shows +3 extra offset
  in built vs stable (+76 vs +73), meaning 3 extra bytes accumulate somewhere between
  that BEQ and BITBLT.1 (73 bytes forward in stable)
- The extra 3 bytes come from the false relaxation of `bne .1` at code+0x03DE
- All CB.Cache/CBX.Cache symbol refs are +3 higher in built — these symbols are defined
  AFTER the false relaxation point, so they shift up by +3

### Key Binary Evidence
```
Stable code+0x03D4: 89 20 F0 49 AC 4E 31 AD 4F 31 D0 09 ...
                    BIT#$20 BEQ+73 LDY$314E LDA$314F BNE+9

Built  code+0x03D4: 89 20 F0 4C AC 51 31 AD 52 31 F0 03 4C 83 24 ...
                    BIT#$20 BEQ+76 LDY$3151 LDA$3152 BEQ+3 JMP$2483
```
- `D0 09` (BNE +9) in stable → `F0 03 4C 83 24` (relaxed BNE, wrong target $2483) in built
- Correct target should be $23EC (BITBLT.0.1), not $2483

### What to investigate next
1. What instructions exist between `beq BITBLT.1` (line 86) and `BITBLT.1` (line 132 of BLT.txt)?
   Count bytes. Is there a sub-function or macro expansion that contains ZP instructions?
2. Check if the ZP correction is being applied to all ZP-mode instructions in the BLT code
3. Also: check pppssc.drv — likely has PUSHA/PULLA or similar ZP overcounting

---

## Key Source Files

- `src/syntax/scmasm_syntax.cpp` — ZP fix at ~line 1157-1200; local label scope at ~line 1109-1143
- `src/core/assembler.cpp` — branch resolution at lines 490–555
- `src/cpu/cpu_6502_branch_handler.cpp` — branch encoding/relaxation
- `/tmp/A2osX-335cd122/DRV/DHGR.DRV.S.BLT.txt` — source of the false branch
- `/tmp/A2osX-335cd122/DRV/PPPSSC.DRV.S.txt` — next to investigate
- `/tmp/A2osX-335cd122/INC/MACROS.I.txt` — macro definitions (PUSHA, PULLA, SYSCALL2, etc.)

---

## Build & Test Commands

```bash
# Build xasm++
cmake --build /Users/bryanw/Projects/Vintage/tools/xasm++/build

# Run unit tests
ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure 2>&1 | tail -5

# Rebuild A2osX after updating xasm++
find /tmp/A2osX-335cd122-build/stage -not -path "*/inc/*" -not -path "*/usr/src/*" -type f -delete
cmake --build /tmp/A2osX-335cd122-build 2>&1 | grep -E "error|warning|Built|dhgr|pppssc" | tail -20

# Extract stable dhgr.drv from STABLE.800.po for comparison
python3 << 'EOF'
import struct
BLOCK_SIZE = 512
def read_block(disk, n):
    disk.seek(n * BLOCK_SIZE); return disk.read(BLOCK_SIZE)

def extract_prodos_file(disk_path, dir_block, filename):
    with open(disk_path, 'rb') as disk:
        block = dir_block
        first = True
        el = 39
        while block:
            data = read_block(disk, block)
            if first:
                el = data[0x23]; epb = data[0x24]
                entry_start = 4 + el; first = False
            else:
                epb = 13; entry_start = 4
            for i in range(epb):
                off = entry_start + i * el
                if off + el > BLOCK_SIZE: break
                st = (data[off] >> 4) & 0xF
                if st == 0: continue
                nl = data[off] & 0xF
                name = data[off+1:off+1+nl].decode('ascii', errors='replace')
                if name.upper() == filename.upper():
                    kb = struct.unpack_from('<H', data, off+0x11)[0]
                    fl = struct.unpack_from('<H', data, off+0x15)[0]
                    result = b''
                    if st == 2:
                        idx = read_block(disk, kb)
                        for b in range(256):
                            lo=idx[b]; hi=idx[b+256]; blk=lo|(hi<<8)
                            result += read_block(disk, blk) if blk else b'\x00'*BLOCK_SIZE
                            if len(result) >= fl: break
                    return result[:fl]
            next_block = struct.unpack_from('<H', data, 2)[0]
            block = next_block

disk = '/Users/bryanw/Projects/Vintage/Apple/A2osX/.Floppies/STABLE.800.po'
# DRV dir is at block 8
stable = extract_prodos_file(disk, 8, 'DHGR.DRV')
with open('/tmp/stable_dhgr.drv', 'wb') as f: f.write(stable)
print(f'Stable DHGR.DRV: {len(stable)} bytes')
EOF

# Compare sizes
python3 -c "
s=open('/tmp/stable_dhgr.drv','rb').read()
b=open('/tmp/A2osX-335cd122-build/stage/drv/dhgr.drv','rb').read()
d=len(b)-len(s)
print(f'delta={d:+d}')
if d==0 and b[16:]==s[16:]: print('IDENTICAL')
else:
    for i in range(16,min(len(s),len(b))):
        if s[i]!=b[i]: print(f'first diff code+0x{i-16:04X}: stable={s[i]:02X} built={b[i]:02X}'); break
"
```

---

## Acceptance Criteria

```bash
# 1. All unit tests pass
ctest --test-dir /Users/bryanw/Projects/Vintage/tools/xasm++/build --output-on-failure
# Expected: 100% tests passed, no failures

# 2. drv/dhgr.drv byte-identical (delta=0, code section matches stable)
# 3. drv/pppssc.drv byte-identical (delta=0, code section matches stable)
# 4. bin/* deltas reduced — prioritize dhgr and pppssc, bin files are stretch goals
```

---

## ZP Fix Reference (already implemented, for context)

In `src/syntax/scmasm_syntax.cpp` around line 1157:
```cpp
size_t est = cpu_->GetInstructionSize(opcode_upper, instr_operand);
// If est==3 and operand resolves to ZP address ($00-$FF), correct to 2
if (est == 3 && opcode_upper != "JSR" && opcode_upper != "JMP") {
  std::string eval_expr = instr_operand;
  auto comma = eval_expr.rfind(',');
  if (comma != std::string::npos)
    eval_expr = Trim(eval_expr.substr(0, comma));
  try {
    uint32_t val = EvaluateExpression(eval_expr, symbols);
    if (val <= 0xFF)
      est = 2;
  } catch (...) {
    // forward reference — keep 3-byte estimate
  }
}
current_address_ += static_cast<uint32_t>(est);
```
This fixed mkboard.drv. The same approach may help dhgr.drv if there are ZP instructions
between the BEQ at line 86 and BITBLT.1 at line 132.
