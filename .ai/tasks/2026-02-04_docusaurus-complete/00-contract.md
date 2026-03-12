# Task Contract: Complete Docusaurus Setup

**Beads Task:** xasm++-mnq
**Related:** xasm++-6x4 (previous agent - hit token limit at turn 88)

## Objective

Complete and verify the Docusaurus documentation site setup that was 95% completed by the previous agent before it hit the token limit.

## Background

Previous agent (task xasm++-6x4) completed most of the Docusaurus setup before failing at turn 88 due to token limit (211,317 tokens).

**What was completed:**
- ✅ Full Docusaurus installation in `docs/website/`
- ✅ Documentation migrated to `docs/content/`
- ✅ GitHub Actions workflow created
- ✅ Site built successfully (build/ directory exists)
- ✅ README created

**What needs verification/completion:**
- Placeholder assets (logo, favicon, banner)
- Final testing
- Task closure

## Requirements

### 1. Verify Existing Setup

Check that everything is in place:
```bash
# Verify structure
ls -la docs/website/
ls -la docs/content/
ls -la .github/workflows/deploy-docs.yml

# Check config
cat docs/website/docusaurus.config.ts
cat docs/website/package.json
```

### 2. Add/Verify Placeholder Assets

**Location:** `docs/website/static/img/`

Required placeholders:
- `logo.svg` - Simple SVG with "xasm++" text
- `favicon.ico` - Icon version of logo
- `banner.png` - Banner image (optional)

If missing, create simple SVG placeholders.

### 3. Test Build

```bash
cd docs/website
npm run build    # Must succeed
npm run serve    # Verify site works locally
```

Check:
- Site builds without errors
- All pages accessible
- Navigation works
- Code blocks render
- No broken links

### 4. Verify GitHub Actions Workflow

Check `.github/workflows/deploy-docs.yml`:
- Correct triggers (push to main, docs/** changes)
- Correct Node.js version
- Builds in docs/website/
- Deploys to GitHub Pages

### 5. Documentation

Verify `docs/README.md` has:
- Clear instructions for local development
- Build commands
- Deployment information

### 6. Close Tasks

```bash
# Close this task
bd close xasm++-mnq -r "Completed Docusaurus setup verification"

# Close original task
bd close xasm++-6x4 -r "Completed by xasm++-mnq after token limit"
```

## Acceptance Criteria

- [ ] All directories and files verified present
- [ ] Placeholder assets exist
- [ ] `npm run build` succeeds
- [ ] Site serves locally and works
- [ ] GitHub Actions workflow is valid
- [ ] README has clear instructions
- [ ] Both Beads tasks closed
- [ ] Ready to commit

## Testing

**Build test:**
```bash
cd docs/website
npm run build
# Should complete without errors
```

**Local serve test:**
```bash
cd docs/website
npm run serve
# Visit http://localhost:3000/xasm++/
# Verify all sections accessible
```

**Workflow validation:**
```bash
# Check workflow syntax
cat .github/workflows/deploy-docs.yml
# Verify it references correct paths
```

## Estimated Effort

5-10 minutes:
- Verification: 3 min
- Assets (if needed): 2 min
- Testing: 3 min
- Task closure: 2 min

## Deliverables

1. Verified complete Docusaurus setup
2. Placeholder assets (if missing)
3. Successful build test
4. Both tasks closed
5. Summary of what's ready

## Notes

- **Keep it simple** - Most work is done, just verify and complete
- **Don't recreate** - Verify existing work first
- **Minimal assets** - Simple SVG placeholders are fine
- **Quick verification** - This should be fast

## Success Criteria

Documentation site is complete and ready to:
1. Build locally
2. Deploy to GitHub Pages
3. Accept real assets later
4. Be committed to repository
