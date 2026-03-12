# Task Contract: Fix GitHub Workflows

**Beads Task:** xasm++-obnu
**Priority:** P1
**Created:** 2026-02-14

## Requirements

Fix all failing GitHub workflows to work with new build structure.

## Context

Recent build reorganization moved:
- Binaries: `build/src/xasm++` → `build/bin/xasm++`
- Test executables: scattered → `build/Testing/`
- Libraries: `build/` → `build/lib/`

GitHub workflows need updates to reflect new paths.

## Acceptance Criteria

- [ ] Build workflow passes
- [ ] Test workflow passes
- [ ] All workflows updated for new build structure
- [ ] Workflows use correct binary paths (build/bin/, build/Testing/)
- [ ] No failing workflows in GitHub Actions

## Stakeholders

- Owner: Bryan Woodruff
- Priority: P1 (blocking releases)
