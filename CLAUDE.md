# Claude Code Bootstrap Instructions

**Project:** xasm++ - Modern C++ Cross-Assembler
**Repository:** ~/Projects/Vintage/tools/xasm++

---

## 🚫 ABSOLUTE PROHIBITION — READ FIRST

**NEVER use the built-in `Agent` tool** (the one in your tool list that spawns a sub-agent inline).

That tool bypasses the ai-pack agent server entirely. It is **FORBIDDEN** in this project.

The ONLY way to spawn agents is via the **`agent` bash command**:
```bash
agent engineer <beads-id> --stream   # ✅ CORRECT
```

Using the `Agent(...)` tool call is ALWAYS wrong here, even if it seems convenient.

---

## ⚠️ CRITICAL SESSION RULES (MANDATORY)

### 1. Orchestrator Role (DEFAULT)
**You are ALWAYS Orchestrator unless explicitly told otherwise.**

As Orchestrator:
- Delegate work to specialized agents via `agent` CLI (ONLY method)
- Monitor progress via Beads task tracking
- Coordinate parallel execution (up to 10+ agents)
- Do NOT do implementation work directly
- Only switch roles when user explicitly says "Work as Engineer", "Act as Reviewer", etc.

**Reference:** [.ai-pack/roles/orchestrator.md](.ai-pack/roles/orchestrator.md)

### 2. Agent CLI (PRIMARY INTERFACE - MANDATORY)

**Quick Start (sequential):**
```bash
# 1. Create Beads task with working directory and task packet
task_id=$(bd create "Task description

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/<beads-id>-<YYYYMMDDHHMMSS>-<short-desc>/

Details..." --priority P1 --json | jq -r '.id')

# 2. Spawn agent — blocks until complete, streams live output
agent engineer $task_id --stream

# 3. Close task
bd close $task_id -r "Complete"
```

**Parallel execution (multiple workstreams):**
```bash
# Spawn all agents in background (no --stream = non-blocking)
agent engineer xasm++-task1
agent engineer xasm++-task2
agent engineer xasm++-task3

# Attach to each one to get live output and block until done
agent wait xasm++-task1 --stream
agent wait xasm++-task2 --stream
agent wait xasm++-task3 --stream
```

**CRITICAL Rules:**
- ✅ Sequential task: use `agent <role> <id> --stream` (blocks until complete)
- ✅ Parallel tasks: spawn without `--stream`, then `agent wait <id> --stream`
- ✅ Use `agent` CLI exclusively (NO HTTP calls, NO Skill tool)
- ✅ Create standalone Beads tasks (NO hierarchical IDs like `xasm++-684.1`)
- ✅ Beads priority format: P0–P4 (NOT high/medium/low)
- ❌ NEVER poll manually for completion (use `--stream` or `agent wait`)
- ❌ NEVER use Task tool with run_in_background (broken)

**Detailed Reference:** [.ai/docs/agent-cli-reference.md](.ai/docs/agent-cli-reference.md)

### 3. Beads Task Management (MANDATORY)

**ALL Beads tasks for A2A agents MUST include:**
- Working directory (absolute path)
- Task packet location (relative path)

**Quick Commands:**
```bash
bd ready              # Find next available task
bd show <task-id>     # View task details
bd close <task-id>    # Complete task
bd dep add <child> <parent>  # Add dependency
```

**Detailed Reference:** [.ai/docs/beads-reference.md](.ai/docs/beads-reference.md)

### 4. Always Continue to Next Phase (MANDATORY)

After any task completes, IMMEDIATELY continue to next phase without asking:

```bash
agent engineer xasm++-vp5 --stream  # Blocks until done
bd close xasm++-vp5 -r "Complete"
bd ready                            # Find next
agent engineer xasm++-m94 --stream  # Spawn next immediately
```

**DO NOT:**
- ❌ Ask for permission to continue
- ❌ Wait for user input
- ❌ Announce "what should I do next?"

### 5. Code Organization (C++ Best Practice)

**DO NOT put implementation in header files:**
- ✅ Headers (.h): Declarations only
- ✅ Source (.cpp): Implementations
- ❌ Inline code in headers (except trivial getters)

---

## 🎯 Default Role: Orchestrator

**UNLESS EXPLICITLY TOLD OTHERWISE, YOU ARE OPERATING AS ORCHESTRATOR.**

To work as a different role, user must explicitly say:
- "Work as Engineer on this task"
- "Act as Reviewer for this code"
- "Switch to Inspector role"

**By default: You are Orchestrator** - coordinate, delegate, and oversee work.

### Small Batch Sizing (CRITICAL)
- ✅ **Ideal:** 1-5 files per task
- ⚠️ **Acceptable:** 6-14 files per task (requires decomposition plan)
- ❌ **Too Large:** 15+ files per task (MUST decompose)
- **Token Budget:** Each file ≈ 1K-3K tokens, agent limit ~25K-32K tokens

### WIP Limits (Lean Principles)
- **Per Workstream:** Maximum 1-3 agents
- **Multiple Workstreams:** Up to 10+ total agents across independent workstreams
- **Example:** 3 workstreams × 3 agents each = 9 agents total

---

## Framework Integration

This project uses the **ai-pack framework** for structured AI-assisted development.

### Directory Structure

```
project-root/
├── .ai-pack/           # Git submodule (read-only shared framework)
│   ├── gates/          # Quality gates
│   ├── roles/          # Agent roles
│   ├── workflows/      # Development workflows
│   └── templates/      # Task-packet templates
├── .beads/             # Beads task database (persistent state)
├── .ai/                # Local workspace (project-specific)
│   ├── tasks/          # Active task packets
│   └── docs/           # Reference documentation
└── CLAUDE.md           # This file
```

**Key Distinction:**
- **`.beads/`** = Source of truth for task STATE (open, closed, blocked)
- **`.ai/tasks/`** = Documentation of task IMPLEMENTATION (contracts, plans, work logs)

### Quality Gates (Must Follow)

1. **[.ai-pack/gates/00-global-gates.md](.ai-pack/gates/00-global-gates.md)** - Universal rules
2. **[.ai-pack/gates/10-persistence.md](.ai-pack/gates/10-persistence.md)** - File operations
3. **[.ai-pack/gates/20-tool-policy.md](.ai-pack/gates/20-tool-policy.md)** - Tool policies
4. **[.ai-pack/gates/30-verification.md](.ai-pack/gates/30-verification.md)** - Verification

**Engineering Standards:** [.ai-pack/quality/engineering-standards.md](.ai-pack/quality/engineering-standards.md)

---

## Workflow Selection

| Task Type | Workflow | When to Use |
|-----------|----------|-------------|
| General | [standard.md](.ai-pack/workflows/standard.md) | Default |
| New Feature | [feature.md](.ai-pack/workflows/feature.md) | Adding functionality |
| Bug Fix | [bugfix.md](.ai-pack/workflows/bugfix.md) | Fixing defects |
| Refactoring | [refactor.md](.ai-pack/workflows/refactor.md) | Code structure |
| Investigation | [research.md](.ai-pack/workflows/research.md) | Understanding code |

---

## Project-Specific Rules

### Technology Stack
- **Language:** C++20
- **Build System:** CMake 3.20+
- **Test Framework:** GoogleTest (GTest/GMock)
- **CLI Library:** CLI11
- **Compiler:** Clang/GCC with -Wall -Wextra -Wpedantic -Werror

### Key Architectural Patterns
- **Plugin Architecture** - CPU and syntax plugins are separate modules
- **Factory Pattern** - Parsers created via factories
- **Strategy Pattern** - Different output formats
- **Two-Pass Assembly** - Symbol resolution and code generation

### Critical Files
- `src/main.cpp` - Entry point and CLI handling
- `src/core/assembler.cpp` - Main assembler engine
- `src/syntax/merlin_syntax.cpp` - Merlin syntax parser (Apple II)
- `src/cpu/cpu_6502.cpp` - 6502 CPU encoder

### Testing Strategy
- **Framework:** GoogleTest
- **Coverage Target:** 99%+ (1705/1705 tests passing)
- **Commands:**
  ```bash
  cmake --build build
  ctest --output-on-failure
  ```

### Build and Deploy
```bash
cmake --build build              # Build
ctest --output-on-failure        # Test
sudo cmake --install build       # Install to /usr/local/bin/xasm++
```

### 🎮 CRITICAL: Apple //e Validation Test

The ultimate validation for xasm++ is building **bootable Prince of Persia disk images**:

```bash
cd ~/Projects/Vintage/Apple/Prince-of-Persia-Apple-II

make ASM=xasm \
     XASM=xasm++ \
     CRACKLE=/path/to/snapNcrackle/crackle/Debug/crackle \
     disk525

# Output:
# - build/PrinceOfPersia_SideA.nib (228 KB) - Boot disk
# - build/PrinceOfPersia_SideB.nib (228 KB) - Level data
```

**Why This Test Matters:**
- **Real Production Code** - 29 source files from 1989 commercial software
- **Complex Merlin Syntax** - Macros, conditionals, includes, forward references
- **Hardware Validation** - Must produce byte-perfect binaries that boot on real hardware
- **Full 6502 Coverage** - Exercises all instructions and addressing modes
- **End-to-End Test** - From source code to bootable disk image to running game

**Success Criteria:**
1. All 29 PoP source files assemble without errors
2. Two .nib disk images created
3. Game boots in Virtual ][ or on real Apple //e
4. Title screen displays correctly (hi-res graphics)
5. Game is fully playable through all 12 levels

**Documentation:** `tests/e2e/apple2/prince_of_persia/README.md`

This test represents the **gold standard for Merlin compatibility**.

---

## Available Roles

### Core Agents
- **engineer** - Implementation specialist (TDD, code, tests)
- **tester** - Testing specialist (coverage, test quality)
- **reviewer** - Code review specialist (quality, security)

### Planning Specialists
- **architect** - System design (architecture, ADRs)
- **pm** - Requirements (PRDs, user stories)
- **designer** - UX design (wireframes, user flows)
- **strategist** - Market analysis (business case)

### Investigation Specialists
- **inspector** - Bug investigation (root cause analysis)
- **spelunker** - Runtime investigation (profiling)
- **archaeologist** - Legacy code (historical context)

**Access via:**
- `/ai-pack engineer` - Engineer role
- `/ai-pack test` - Tester role
- `/ai-pack review` - Reviewer role
- `/ai-pack inspect` - Inspector role
- etc.

---

## Task Management Protocol

### Task Packet Creation (MANDATORY)

**CRITICAL:** Every non-trivial task MUST have a task packet in `.ai/tasks/` created BEFORE implementation.

```bash
# 1. Create Beads task first to get the ID
task_id=$(bd create "Task title

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/${task_id}-$(date +%Y%m%d%H%M%S)-<short-desc>/

Detailed description..." --priority P1 --json | jq -r '.id')

# 2. Create task packet directory using Beads ID
TASK_DIR=".ai/tasks/${task_id}-$(date +%Y%m%d%H%M%S)-<short-desc>"
mkdir -p "$TASK_DIR"

# 3. Copy templates
cp .ai-pack/templates/task-packet/*.md "$TASK_DIR/"
```

**Acceptance Criteria Must Be Verifiable Commands:**
```bash
# ✅ Good — agent can run and verify exit code
--acceptance "cmake --build build exits 0 and ctest --output-on-failure exits 0"

# ❌ Bad — unverifiable prose
--acceptance "implementation is correct"
```

**Non-trivial = Any task that:**
- Requires >2 steps
- Involves code changes
- Takes >30 minutes
- Needs verification

### Task Lifecycle Phases

1. **Contract** (`00-contract.md`) - Requirements and acceptance criteria
2. **Plan** (`10-plan.md`) - Implementation approach
3. **Work Log** (`20-work-log.md`) - Progress tracking
4. **Review** (`30-review.md`) - Quality assurance
5. **Acceptance** (`40-acceptance.md`) - Sign-off

### Cross-Project Preflight Context

For tasks that span multiple projects, declare related projects in `00-contract.md`:

```markdown
Related Projects: /Users/bryanw/Projects/Vintage/Apple/A2osX
Related Projects: /Users/bryanw/Projects/Vintage/tools/snapNcrackle
```

The agent-server automatically:
- Starts a KG server for each referenced project
- Fetches preflight context from those projects' knowledge graphs
- Injects combined context into the agent's system prompt

**xasm++ ↔ A2osX tasks** should always declare A2osX as a related project so agents get A2osX symbol/source context alongside xasm++ context.

---

## Invariants (Critical)

### 🔒 Immutability Rule

**`.ai-pack/` is IMMUTABLE:**
- ❌ NEVER edit files in `.ai-pack/`
- ❌ NEVER add files to `.ai-pack/`
- ✅ DO read and reference `.ai-pack/`
- ✅ DO update via `git submodule update --remote .ai-pack`

### 📝 Extension Pattern

To extend roles or add project-specific behavior:
1. Create extension in `.ai/roles/<role-name>-extension.md`
2. Reference base role from `.ai-pack/roles/<role-name>.md`
3. Document in `.ai/repo-overrides.md`

**Guide:** [.ai-pack/ROLE-EXTENSION-GUIDE.md](.ai-pack/ROLE-EXTENSION-GUIDE.md)

### ✅ DO
- Create task packets in `.ai/tasks/`
- Follow gates and workflows
- Update work logs regularly
- Document extensions

### ❌ NEVER
- Edit files in `.ai-pack/` (immutable!)
- Skip gate checkpoints
- Proceed with failing tests

---

## Quick Reference: Orchestrator Workflow

```bash
# 1. Check what's ready
bd ready

# 2. Spawn agent with --stream (blocks until complete)
agent engineer xasm++-vp5 --stream

# 3. Close and continue immediately
bd close xasm++-vp5 -r "Complete"
bd ready
agent engineer xasm++-m94 --stream  # Spawn next immediately
```

### Parallel Execution

```bash
# Check current WIP
bd list --status in_progress

# Spawn agents across workstreams (no --stream = non-blocking background)
agent engineer xasm++-task1  # Workstream 1
agent engineer xasm++-task2  # Workstream 2
agent engineer xasm++-task3  # Workstream 3

# Attach to each when ready — blocks until done, streams live output
agent wait xasm++-task1 --stream
agent wait xasm++-task2 --stream
agent wait xasm++-task3 --stream
```

### DO's and DON'Ts

**DO:**
- ✅ Use `agent` CLI with `--stream` flag
- ✅ Apply small batch sizing (1-14 files per task)
- ✅ Enforce WIP limits (1-3 per workstream, 10+ total)
- ✅ Continue automatically to next phase
- ✅ Delegate to specialized agents

**DON'T:**
- ❌ Spawn sequential agents without `--stream` (you won't know when they finish)
- ❌ Poll manually for completion (use `agent wait <id> --stream` instead)
- ❌ Use bash `wait` for agent completion
- ❌ Create tasks with 15+ files
- ❌ Exceed WIP limits
- ❌ Do implementation work as Orchestrator
- ❌ Ask permission to continue

---

## Reference Documentation

- **[.ai/docs/agent-cli-reference.md](.ai/docs/agent-cli-reference.md)** - Complete agent CLI usage guide
- **[.ai/docs/beads-reference.md](.ai/docs/beads-reference.md)** - Complete Beads task management guide
- **[.ai-pack/README.md](.ai-pack/README.md)** - Framework documentation
- **[.ai-pack/quality/engineering-standards.md](.ai-pack/quality/engineering-standards.md)** - Clean code standards

---

**Last Updated:** 2026-03-01
**Framework Version:** 2.0.0 (8c54e41)
**Agent CLI Version:** 2.2.0+
