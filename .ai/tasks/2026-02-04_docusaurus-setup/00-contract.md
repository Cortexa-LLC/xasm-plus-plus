# Task Contract: Docusaurus Documentation Site Setup

**Beads Task:** xasm++-6x4

## Objective

Set up a Docusaurus documentation website for xasm++ following the pattern from `~/Projects/Vibe/ai-pack/docs/`, with GitHub Pages deployment and CI/CD integration.

## Background

**Current State:**
- Documentation exists in `docs/` directory (architecture, ADRs, etc.)
- No documentation website or publishing mechanism
- The ai-pack framework has a complete Docusaurus setup to use as reference

**Goal:**
Create a professional documentation site at `https://USERNAME.github.io/xasm++/` with:
- Modern, searchable interface
- Automatic deployment on commits
- Architecture documentation
- API documentation
- Getting started guides

## Reference Implementation

**Copy pattern from:** `~/Projects/Vibe/ai-pack/docs/`

Key files to examine:
- `~/Projects/Vibe/ai-pack/docs/website/docusaurus.config.ts` - Main configuration
- `~/Projects/Vibe/ai-pack/docs/website/sidebars.ts` - Sidebar structure
- `~/Projects/Vibe/ai-pack/docs/content/` - Documentation content
- `~/Projects/Vibe/ai-pack/.github/workflows/` - GitHub Pages workflow (if exists)

## Requirements

### 1. Directory Structure

Create the following structure:

```
docs/
├── website/                  # Docusaurus site
│   ├── docusaurus.config.ts
│   ├── sidebars.ts
│   ├── package.json
│   ├── tsconfig.json
│   ├── src/
│   │   ├── pages/
│   │   └── css/
│   └── static/
│       └── img/
│           ├── logo.svg     # Placeholder
│           ├── favicon.ico  # Placeholder
│           └── banner.png   # Placeholder
├── content/                 # Documentation markdown
│   ├── intro.md
│   ├── getting-started/
│   ├── architecture/
│   ├── adr/
│   └── api/
└── README.md
```

### 2. Configuration

Adapt from ai-pack but customize for xasm++:
```typescript
{
  title: 'xasm++',
  tagline: 'Modern C++ Cross-Assembler for 6502, 65C02, 65816, and 6809',
  url: 'https://USERNAME.github.io',
  baseUrl: '/xasm++/',
  organizationName: 'USERNAME',
  projectName: 'xasm++',
  themes: ['@docusaurus/theme-mermaid'],
  markdown: { mermaid: true }
}
```

### 3. Content Migration

Convert existing docs:
- `docs/architecture/` → `docs/content/architecture/`
- `docs/adr/` → `docs/content/adr/`
- Add frontmatter to all markdown files
- Create intro and getting-started guides

### 4. GitHub Actions Workflow

**File:** `.github/workflows/docs.yml`

Create workflow for automatic GitHub Pages deployment (reference ai-pack if it has one, otherwise create standard GitHub Pages deployment workflow).

### 5. Placeholder Assets

Create simple SVG placeholders:
- `logo.svg` - Simple "xasm++" text logo
- `favicon.ico` - Generated from logo
- `banner.png` - Simple banner

User will replace these later with real assets.

### 6. Local Development

Add npm scripts:
```json
{
  "scripts": {
    "start": "docusaurus start",
    "build": "docusaurus build",
    "serve": "docusaurus serve"
  }
}
```

## Acceptance Criteria

### Functional
- [ ] Docusaurus builds locally without errors
- [ ] GitHub Actions workflow created
- [ ] Site deploys to GitHub Pages
- [ ] Navigation works (sidebar, search)
- [ ] Code blocks render properly
- [ ] Mobile responsive

### Content
- [ ] Introduction page
- [ ] Getting started guide
- [ ] Architecture docs migrated
- [ ] ADRs migrated
- [ ] API documentation structure
- [ ] All links working

### Technical
- [ ] TypeScript configuration valid
- [ ] Dependencies locked
- [ ] Build optimized
- [ ] SEO metadata configured
- [ ] Placeholder assets in place

## Testing

```bash
cd docs/website
npm install
npm run build    # Must succeed
npm run serve    # Verify site works
```

Manual checks:
- Navigate all sections
- Test search
- Check mobile view
- Verify code blocks
- Test all links

## Dependencies

```json
{
  "@docusaurus/core": "^3.0.0",
  "@docusaurus/preset-classic": "^3.0.0",
  "@docusaurus/theme-mermaid": "^3.0.0",
  "react": "^18.0.0",
  "react-dom": "^18.0.0"
}
```

## Technical Notes

### GitHub Pages Setup

After first workflow run:
1. Repository Settings → Pages
2. Source: Deploy from a branch
3. Branch: `gh-pages` / `root`
4. Save

### ai-pack Reference

Study these files:
```bash
~/Projects/Vibe/ai-pack/docs/website/docusaurus.config.ts
~/Projects/Vibe/ai-pack/docs/website/sidebars.ts
~/Projects/Vibe/ai-pack/docs/website/package.json
~/Projects/Vibe/ai-pack/docs/content/
```

Copy structure and adapt for xasm++.

### Asset Replacement

User will later replace:
- `docs/website/static/img/logo.svg`
- `docs/website/static/img/favicon.ico`
- `docs/website/static/img/banner.png`
- Colors in `docs/website/src/css/custom.css`

## Estimated Effort

30-45 minutes total:
- Docusaurus setup: 10 min
- Content migration: 15 min
- GitHub Actions: 10 min
- Testing: 10 min

## References

- ai-pack docs: `~/Projects/Vibe/ai-pack/docs/`
- Docusaurus: https://docusaurus.io/docs
- GitHub Pages: https://docs.github.com/en/pages

## Deliverables

1. Complete Docusaurus setup in `docs/website/`
2. Migrated content in `docs/content/`
3. GitHub Actions workflow
4. Placeholder assets
5. Working site (local + deployed)
6. Documentation README
