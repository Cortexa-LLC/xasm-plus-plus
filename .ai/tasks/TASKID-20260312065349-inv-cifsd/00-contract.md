# Task Contract: Investigate sbin/cifsd 26 Diffs

**Task ID:** TASKID-20260312065349-inv-cifsd  
**Created:** 2026-03-12  
**Status:** ✅ COMPLETE  
**Assigned To:** Engineer

---

## Objective

Determine whether the 26 byte differences in sbin/cifsd between xasm++ build (commit 335cd122) and reference build (commit ce94435) are due to:
1. A2osX source changes between commits (SYSCALL relocation), OR
2. An xasm++ assembler bug

---

## Context

From Run 15 validation report:
- **File:** sbin/cifsd
- **Size:** 4920 bytes (same in both builds)
- **Differences:** 26 bytes differ
- **First difference:** @0x00FF: 0x00→0x53
- **Pattern:** Appears to be 0x00→0x53 address changes

---

## Acceptance Criteria

- [ ] Root cause identified (source change vs assembler bug)
- [ ] Evidence documented with specific byte addresses and values
- [ ] If source change: Identify what changed in A2osX between commits
- [ ] If assembler bug: Identify the xasm++ defect
- [ ] Recommendation provided for next steps
- [ ] Knowledge graph updated with findings

---

## Resources

- Validation report: `.ai/tasks/xasm-89o-20260312063117-run15-full-comparison/30-validation-report.md`
- Previous investigation: `/tmp/INVESTIGATION-cifsd-26-diffs.md`
- Knowledge graph: "cifsd SYSCALL investigation" topic
- A2osX source commits: ce94435 (reference) vs 335cd122 (current)

---

## Related Work

- Similar investigations completed for bin/du (+2B source change verified)
- SYSCALL architecture changes between A2osX versions documented
- Pattern recognition from other diff investigations
