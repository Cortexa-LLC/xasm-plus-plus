# Task Contract: Investigate Path Reference Errors

**Task ID:** 2026-02-18_path-references
**Beads Task:** xasm++-syxi
**Created:** 2026-02-18
**Assigned Role:** Spelunker
**Workflow:** Research
**Priority:** P2

---

## Context

After fixing all major A2osX errors (785/788), only 3 path reference errors remain.

**Error Pattern:**
```
error: Invalid argument for USR/SRC/A2OSX.S: Unsupported instruction: USR/SRC/A2OSX.S
```

**Occurrences:** 3 identical errors (0.4% of original 788)

---

## Investigation Tasks

```
□ Find exact lines where USR/SRC/A2OSX.S appears
□ Determine context (editor command? comment? directive?)
□ Check if similar to SAVE/ASM commands
□ Recommend fix strategy
□ Estimate complexity
```

---

## Deliverables

1. Root cause analysis
2. Code location(s)
3. Fix recommendation
4. Priority/complexity estimate

---

## Test Command

```bash
cd ~/Projects/Vintage/Apple/A2osX
XASM=~/Projects/Vintage/tools/xasm++/build/bin/xasm++ make clean && make 2>&1 | grep "USR/SRC/A2OSX.S"
```

---

## Expected Outcome

Final fix to achieve **788 → 0 errors (100% A2osX compatibility)**
