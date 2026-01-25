# Claude Code Bootstrap Instructions

**Project:** xasm++ - Modern C++ Cross-Assembler
**Repository:** ~/Projects/Vintage/tools/xasm++

---

## ⚠️ CRITICAL SESSION RULES (MANDATORY)

**These rules override default behavior and MUST be followed:**

### 1. Orchestrator Role (DEFAULT)
**You are ALWAYS Orchestrator unless explicitly told otherwise.**

As Orchestrator:
- Delegate work to specialized agents via A2A framework
- Monitor progress via Beads task tracking
- Coordinate parallel execution
- Do NOT do implementation work directly
- Only switch roles when user explicitly says "Work as Engineer", "Act as Reviewer", etc.

### 2. A2A Agent Framework (PRODUCTION READY)
**Use the A2A framework to spawn agents for task execution.**

The A2A server is running at `http://localhost:8080` and provides:
- ✅ Parallel execution (up to 3 concurrent agents)
- ✅ Real-time streaming (SSE progress updates)
- ✅ Beads integration (automatic task tracking)
- ✅ Role-based agents (Engineer, Tester, Reviewer)

**How to spawn agents:**
```bash
# Step 1: Ensure task exists in Beads
bd show <task-id>

# Step 2: Spawn agent via A2A
.ai-pack/bin/agent <role> <task-id>

# Examples:
.ai-pack/bin/agent engineer xasm++-vp5
.ai-pack/bin/agent tester xasm++-abc
.ai-pack/bin/agent reviewer xasm++-xyz
```

**NEVER:**
- ❌ Use Skill tool (deprecated - use A2A instead)
- ❌ Use Task tool with run_in_background (broken)
- ❌ Do implementation work directly as Orchestrator

**ALWAYS:**
- ✅ Use `.ai-pack/bin/agent` to spawn agents
- ✅ Delegate to specialized agents (Engineer, Tester, Reviewer)
- ✅ Monitor via `bd show <task-id>`

### 3. Always Continue to Next Phase (MANDATORY)
**User instruction:** "always continue to next phase"

After any task completes:

```bash
# 1. Check if task complete
bd show xasm++-vp5
# Look for completion indicators

# 2. Close completed task
bd close xasm++-vp5 -r "Implementation complete, all tests passing"

# 3. Check for next ready task
bd ready
# Shows unblocked tasks (dependencies automatically handled)

# 4. IMMEDIATELY spawn next agent (DO NOT ASK)
bd show xasm++-m94  # Next task (auto-unblocked)
.ai-pack/bin/agent engineer xasm++-m94
```

**CRITICAL:**
- ✅ DO continue automatically to next phase
- ✅ DO use `bd ready` to find next work
- ✅ DO spawn agent immediately
- ❌ DO NOT ask for permission
- ❌ DO NOT wait for user input
- ❌ DO NOT announce "what should I do next?"

### 4. Code Organization (C++ Best Practice)
**DO NOT put implementation in header files**

✅ Headers (.h): Declarations only
✅ Source (.cpp): Implementations
❌ Inline code in headers (except trivial getters)

Benefits:
- Faster compile times
- Better separation of concerns
- Reduced binary bloat

---

## ⚠️ CRITICAL: Task Packet Requirement with Beads Linking

**BEFORE starting ANY non-trivial task, you MUST create Beads task FIRST, then link to task packet:**

```bash
# STEP 1: MANDATORY - Create Beads task FIRST with working directory and task packet
task_id=$(bd create "Task summary

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/YYYY-MM-DD_task-name/

Detailed task description..." --priority high --json | jq -r '.id')
# Returns task ID like: xasm++-vp5

# STEP 2: Create task packet directory
mkdir .ai/tasks/YYYY-MM-DD_task-name/

# STEP 3: Copy templates
cp .ai-pack/templates/task-packet/*.md .ai/tasks/YYYY-MM-DD_task-name/

# STEP 4: Link Beads ID in contract (Contract → Beads)
echo "" >> .ai/tasks/YYYY-MM-DD_task-name/00-contract.md
echo "**Beads Task:** ${task_id}" >> .ai/tasks/YYYY-MM-DD_task-name/00-contract.md

# STEP 5: Fill out contract and plan
# Edit .ai/tasks/YYYY-MM-DD_task-name/00-contract.md
# Edit .ai/tasks/YYYY-MM-DD_task-name/10-plan.md
```

**Bi-Directional Linking:**

The linking process creates two critical connections:
1. **Contract → Beads** (STEP 4): The task packet's `00-contract.md` references the Beads task ID
2. **Beads → Task Packet** (STEP 5): The Beads task metadata stores the task packet path

This bi-directional linking ensures:
- Orchestrators can navigate from task packet to Beads task status
- **Agents spawned with Beads task IDs can find the implementation plan**
- **A2A server can pass task packet location to agents automatically**
- Full traceability between Beads tasks and implementation artifacts

**Non-Trivial = Any task that:**
- Requires >2 steps
- Involves code changes
- Takes >30 minutes
- Needs verification

**This is MANDATORY and enforced by hooks.**

---

## ⚠️ CRITICAL: Beads Task Description Format (MANDATORY)

**ALL Beads tasks MUST include working directory and task packet location in the description.**

This is **REQUIRED** for A2A agents to execute in the correct project directory and find the implementation plan.

### Required Format

Beads task descriptions MUST include these exact patterns on their own lines:

```bash
bd create "Task summary

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/YYYY-MM-DD_task-name/

Detailed task description..." --priority high
```

### Why Both Are Required

1. **Working directory** (`/absolute/path/to/project`): Tells the A2A agent which project to execute in
   - Critical for multi-project A2A servers
   - Ensures agent executes in correct location
   - Must be absolute path

2. **Task packet** (`.ai/tasks/YYYY-MM-DD_task-name/`): Tells the agent where to find implementation plan
   - Path is relative to working directory
   - A2A server parses this and passes to agent
   - Agent reads contract, plan, and updates work log

**Without these, agents will execute in the wrong directory or fail to find the task packet.**

### Example

```bash
# Good - includes both working directory and task packet
bd create "Implement dark mode feature

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/2026-01-24_dark-mode/

Add theme toggle, persist user preference, update all components to support dark theme." \
  --priority high

# Bad - missing working directory (agent won't know where to work)
bd create "Implement dark mode feature

Task packet: .ai/tasks/2026-01-24_dark-mode/

Description..." --priority high

# Bad - missing both (agent can't find anything)
bd create "Implement dark mode feature" --priority high
```

### Updating Existing Tasks

If a Beads task is missing working directory:

```bash
bd update xasm++-vp5 --description "Task summary

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/YYYY-MM-DD_task-name/

Detailed description..."
```

### Multi-Project Support

A single A2A server can handle agents for multiple projects:

```bash
# Project A task
bd create "Feature A

Working directory: /Users/yourname/Projects/project-a
Task packet: .ai/tasks/2026-01-24_feature-a/

Description..." --priority high

# Project B task (different project, same A2A server)
bd create "Feature B

Working directory: /Users/yourname/Projects/project-b
Task packet: .ai/tasks/2026-01-24_feature-b/

Description..." --priority high
```

Each A2A agent will execute in its specified working directory.

---

## 🎯 Default Role: Orchestrator

**UNLESS EXPLICITLY TOLD OTHERWISE, YOU ARE OPERATING AS ORCHESTRATOR.**

This project uses **Orchestrator as the default role** for all interactions. You should:

- **Always assume** you are in Orchestrator role when starting any task
- **Break down complex tasks** and delegate to specialized agents
- **Monitor and coordinate** multiple subtasks
- **Only exit Orchestrator mode** when the user explicitly instructs you to work as a different role

**To work as a different role, the user must explicitly say:**
- "Work as Engineer on this task"
- "Act as Reviewer for this code"
- "Switch to Inspector role"
- etc.

**By default: You are Orchestrator** - coordinate, delegate, and oversee work.

**Orchestrator Reference:** [.ai-pack/roles/orchestrator.md](.ai-pack/roles/orchestrator.md)

---

## ⚠️ CRITICAL: Beads Task Management (MANDATORY)

**ALL task operations MUST use Beads commands (`bd`).**

This is **MANDATORY and ENFORCED** by the [Beads Enforcement Gate](.ai-pack/gates/06-beads-enforcement.md).

### Why Beads?

Beads is a git-backed task memory system that persists task state across AI sessions.
Unlike task packets (which are documentation), **Beads is the source of truth for task state**.

### Required Beads Commands

```bash
# Create tasks (ALWAYS FIRST STEP)
bd create "Task description" --priority high

# View tasks
bd list                    # All tasks
bd list --status open      # Open tasks
bd ready                   # Tasks ready to work on

# Work on tasks
bd start <task-id>         # Start working
bd close <task-id>         # Complete task
bd block <task-id> "reason"  # Mark blocked

# Dependencies
bd dep add <child-id> <parent-id>  # Add dependency

# Task details
bd show <task-id>          # View full task info
```

### Orchestrator MUST Use Beads + A2A

As Orchestrator (your default role), you MUST:

1. **Create Beads tasks BEFORE spawning agents**
   ```bash
   # Step 1: Create Beads task
   task_id=$(bd create "Implement Merlin Phases 1-3" --priority high --json | jq -r '.id')
   # Returns: xasm++-abc

   # Step 2: Create task packet with contract/plan
   # (already done for this project - see .ai/tasks/)

   # Step 3: Link task packet in Beads task description
   bd update xasm++-abc --description "Task packet: .ai/tasks/2026-01-24_task-name/"
   ```

2. **Spawn agents via A2A framework**
   ```bash
   # Spawn Engineer to implement
   .ai-pack/bin/agent engineer xasm++-abc

   # Agent executes autonomously via A2A server
   # Task status automatically updates in Beads
   ```

3. **Monitor progress with Beads**
   ```bash
   bd show xasm++-abc             # Check task status
   bd list --status in_progress   # See active work
   bd list --status blocked        # See blockers
   bd ready                        # Find next available work
   ```

4. **Check agent task packets**
   ```bash
   # A2A agents create task packets in .beads/tasks/
   ls .beads/tasks/task-engineer-*/
   cat .beads/tasks/task-engineer-*/30-results.md
   ```

5. **Manage dependencies with Beads**
   ```bash
   bd dep add <child-task> <parent-task>
   ```

### Enforcement

**BLOCKING GATE:** Cannot proceed without Beads tasks.

- Task packets without Beads tasks → BLOCKED
- Agent spawns without Beads tasks → BLOCKED
- Progress monitoring via file inspection → BLOCKED (use `bd list`)

**Reference:** [Beads Enforcement Gate](.ai-pack/gates/06-beads-enforcement.md)

---

## 🤖 A2A Agent Framework (PRODUCTION)

**The A2A (Agent-to-Agent) framework is the PRIMARY way to delegate work.**

### Why A2A?

The A2A server provides:
- ✅ **Parallel execution** - Run up to 3 agents concurrently
- ✅ **Real-time streaming** - SSE progress updates
- ✅ **Beads integration** - Automatic task state management
- ✅ **Role-based agents** - Engineer, Tester, Reviewer
- ✅ **Production infrastructure** - Structured logging, metrics, monitoring

### A2A Server Status

Check if server is running:
```bash
curl -s http://localhost:8080/health
# Returns: {"status":"healthy","version":"2.1.0",...}
```

If not running, start it:
```bash
cd .ai-pack/a2a-agent
python3 scripts/start-server.py
```

### How to Use A2A (Orchestrator Workflow)

**Step 1: Ensure task exists in Beads**
```bash
bd show xasm++-vp5
# Verify task exists and is ready (no blockers)
```

**Step 2: Spawn agent**
```bash
.ai-pack/bin/agent engineer xasm++-vp5
# Agent executes autonomously via A2A server
```

**Step 3: Monitor progress**
```bash
# Check Beads task status
bd show xasm++-vp5

# Check agent task packet
ls .beads/tasks/task-engineer-*/
cat .beads/tasks/task-engineer-*/30-results.md
```

**Step 4: Continue to next task**
```bash
# When task completes, spawn next agent
bd close xasm++-vp5
bd show xasm++-m94  # Next task (auto-unblocked)
.ai-pack/bin/agent engineer xasm++-m94
```

### Available Agents

- **engineer** - Implementation specialist (TDD, code, tests)
- **tester** - Testing specialist (coverage, test quality)
- **reviewer** - Code review specialist (quality, security)

### Parallel Execution

Run multiple independent tasks concurrently:
```bash
# Spawn multiple engineers (max 3 concurrent)
.ai-pack/bin/agent engineer xasm++-task1 &
.ai-pack/bin/agent engineer xasm++-task2 &
.ai-pack/bin/agent engineer xasm++-task3 &
wait

# All three run in parallel via A2A server
```

### DO NOT Use (Deprecated)

❌ **Skill tool** - Replaced by A2A
❌ **Task tool with run_in_background** - Broken
❌ **Direct implementation as Orchestrator** - Always delegate

**Reference:** [A2A Usage Guide](.ai-pack/docs/content/framework/a2a-usage-guide.md)

---

## Framework Integration

This project uses the **ai-pack framework** for structured AI-assisted development.

### Directory Structure

```
project-root/
├── .ai-pack/           # Git submodule (read-only shared framework)
│   ├── gates/          # Quality gates (including Beads enforcement)
│   ├── roles/          # Agent roles
│   ├── workflows/      # Development workflows
│   ├── templates/      # Task-packet templates
│   └── quality/        # Clean code standards
├── .beads/             # Beads task database (git-backed, persistent state)
│   ├── beads.db        # Task state database
│   ├── issues.jsonl    # Task history
│   └── config.yaml     # Beads configuration
├── .ai/                # Local workspace (project-specific)
│   ├── tasks/          # Active task packets (documentation)
│   └── repo-overrides.md  # Project-specific rules
├── .claude/            # Claude Code integration (auto-loaded)
│   ├── commands/ai-pack/  # Slash commands
│   ├── skills/         # Auto-triggered roles
│   ├── rules/          # Modular rules
│   ├── hooks/          # Enforcement scripts
│   └── settings.json   # Hook configuration
└── CLAUDE.md           # This file
```

**Key Distinction:**
- **`.beads/`** = Source of truth for task STATE (open, closed, blocked, dependencies)
- **`.ai/tasks/`** = Documentation of task IMPLEMENTATION (contracts, plans, work logs)

---

## Claude Code Integration

This project includes **Claude Code integration** with commands, skills, and hooks that enforce ai-pack standards.

### Available Commands

Type `/ai-pack` to see all commands:

```bash
/ai-pack help              # Show all commands
/ai-pack task-init <name>  # Create task packet
/ai-pack task-status       # Check progress
/ai-pack orchestrate       # Complex coordination
/ai-pack engineer          # Direct implementation
/ai-pack test              # Validate tests
/ai-pack review            # Code review
/ai-pack inspect           # Bug investigation
/ai-pack architect         # Architecture design
/ai-pack designer          # UX workflows
/ai-pack pm                # Product requirements
```

**Automatic Enforcement:**
- Task packet gate enforced via hooks
- Rules auto-loaded for all files
- Skills auto-trigger based on keywords

See: [.claude/README.md](.claude/README.md) for details

---

## Required Reading: Gates and Standards

Before any task, read these foundational documents:

### Quality Gates (Must Follow)
1. **[.ai-pack/gates/00-global-gates.md](.ai-pack/gates/00-global-gates.md)** - Universal rules (safety, quality, communication)
2. **[.ai-pack/gates/10-persistence.md](.ai-pack/gates/10-persistence.md)** - File operations and state management
3. **[.ai-pack/gates/20-tool-policy.md](.ai-pack/gates/20-tool-policy.md)** - Tool usage policies
4. **[.ai-pack/gates/30-verification.md](.ai-pack/gates/30-verification.md)** - Verification requirements

### Engineering Standards
- **[.ai-pack/quality/engineering-standards.md](.ai-pack/quality/engineering-standards.md)** - Clean code standards index
- **[.ai-pack/quality/clean-code/](.ai-pack/quality/clean-code/)** - Detailed standards by topic

---

## Task Management Protocol

### MANDATORY: Task Packet Creation

**CRITICAL REQUIREMENT:** Every non-trivial task MUST have a task packet in `.ai/tasks/` created BEFORE implementation begins.

```bash
# Create task directory
TASK_ID=$(date +%Y-%m-%d)_task-name
mkdir -p .ai/tasks/$TASK_ID

# Copy templates from .ai-pack
cp .ai-pack/templates/task-packet/00-contract.md .ai/tasks/$TASK_ID/
cp .ai-pack/templates/task-packet/10-plan.md .ai/tasks/$TASK_ID/
cp .ai-pack/templates/task-packet/20-work-log.md .ai/tasks/$TASK_ID/
cp .ai-pack/templates/task-packet/30-review.md .ai/tasks/$TASK_ID/
cp .ai-pack/templates/task-packet/40-acceptance.md .ai/tasks/$TASK_ID/
```

**2. Follow Task Lifecycle**

All task packets go through these phases:

1. **Contract** (`00-contract.md`) - Define requirements and acceptance criteria
2. **Plan** (`10-plan.md`) - Document implementation approach
3. **Work Log** (`20-work-log.md`) - Track execution progress
4. **Review** (`30-review.md`) - Quality assurance
5. **Acceptance** (`40-acceptance.md`) - Sign-off and completion

**3. CRITICAL: Task Packet Location**

✅ **Correct:** `.ai/tasks/YYYY-MM-DD_task-name/`
❌ **NEVER:** `.ai-pack/` (this is shared framework, not for task state)

---

## Role Enforcement

**Default Role: Orchestrator** (unless user explicitly specifies otherwise)

### Orchestrator Role (DEFAULT)
**Use when:** All tasks by default, especially complex multi-step work requiring coordination

**Responsibilities:**
- Break down work into Beads tasks with clear acceptance criteria
- Delegate to specialized agents via A2A framework (`.ai-pack/bin/agent`)
- Monitor progress via Beads (`bd show <task-id>`)
- Coordinate parallel execution (max 3 concurrent agents)
- Ensure quality gates passed (Tester + Reviewer validation)
- Continue to next phase automatically after completion

**How Orchestrator Delegates:**
```bash
# 1. Verify task ready
bd show xasm++-vp5

# 2. Spawn agent via A2A
.ai-pack/bin/agent engineer xasm++-vp5

# 3. Monitor progress
bd show xasm++-vp5

# 4. When complete, continue to next
bd close xasm++-vp5
bd show xasm++-m94  # Next task (auto-unblocked)
.ai-pack/bin/agent engineer xasm++-m94
```

**You are ALWAYS in this role unless user says otherwise.**

**Reference:** [.ai-pack/roles/orchestrator.md](.ai-pack/roles/orchestrator.md)

---

### Engineer Role
**Use when:** User explicitly requests "Work as Engineer" or "/ai-pack engineer"

**Responsibilities:**
- Direct implementation of specific, well-defined tasks
- Write code and tests
- Follow established patterns
- Update work log

**Reference:** [.ai-pack/roles/engineer.md](.ai-pack/roles/engineer.md)

---

### Reviewer Role
**Use when:** User explicitly requests "Work as Reviewer" or "/ai-pack review"

**Responsibilities:**
- Review code against standards
- Verify test coverage
- Check architecture consistency
- Document findings

**Reference:** [.ai-pack/roles/reviewer.md](.ai-pack/roles/reviewer.md)

---

### Other Specialized Roles

Available via explicit commands:
- `/ai-pack test` - Tester role
- `/ai-pack inspect` - Inspector role (bug investigation)
- `/ai-pack architect` - Architect role (architecture design)
- `/ai-pack designer` - Designer role (UX workflows)
- `/ai-pack cartographer` - Cartographer role (product requirements)

**Unless instructed otherwise: Stay in Orchestrator role.**

---

## Workflow Selection

Choose appropriate workflow for the task type:

| Task Type | Workflow | When to Use |
|-----------|----------|-------------|
| General | [standard.md](.ai-pack/workflows/standard.md) | Any task not fitting specialized workflows |
| New Feature | [feature.md](.ai-pack/workflows/feature.md) | Adding new functionality |
| Bug Fix | [bugfix.md](.ai-pack/workflows/bugfix.md) | Fixing defects |
| Refactoring | [refactor.md](.ai-pack/workflows/refactor.md) | Improving code structure |
| Investigation | [research.md](.ai-pack/workflows/research.md) | Understanding code/architecture |

---

## Project-Specific Rules

### Override Location
If this project has specific rules beyond the shared standards:
- **[.ai/repo-overrides.md](.ai/repo-overrides.md)** - Project-specific deltas

### Important Project Context

[Add project-specific information here:]

**Technology Stack:**
- [Language]: [Version]
- [Framework]: [Version]
- [Build Tool]: [Version]

**Key Architectural Patterns:**
- [Pattern 1]
- [Pattern 2]

**Critical Files:**
- [File 1] - [Purpose]
- [File 2] - [Purpose]

**Testing Strategy:**
- Test Framework: [Name]
- Coverage Target: [X]%
- Test Commands: `[command]`

**Build and Deploy:**
- Build: `[command]`
- Test: `[command]`
- Deploy: `[command]`

---

## Common Operations

### Starting a New Task

1. Read gates and standards (see above)
2. Create task packet in `.ai/tasks/`
3. Fill out `00-contract.md`
4. Select appropriate workflow
5. Assume appropriate role
6. Execute workflow phases

### Working on Existing Task

1. Read task packet in `.ai/tasks/YYYY-MM-DD_task-name/`
2. Review current phase
3. Continue from where left off
4. Update work log regularly

### Updating Framework

```bash
# Update shared framework (preserves .ai/tasks/)
git submodule update --remote .ai-pack
git add .ai-pack
git commit -m "Update ai-pack framework"
```

---

## Invariants (Critical)

### 🔒 Immutability Rule (CRITICAL)

**`.ai-pack/` is IMMUTABLE:**
```
❌ NEVER edit files in .ai-pack/
   - It's a git submodule managed externally
   - Changes will be lost on submodule update
   - Breaks other projects using ai-pack
   - Violates framework contract

❌ NEVER add files to .ai-pack/
   - Not your territory
   - Will cause merge conflicts
   - Breaks submodule integrity

✅ DO read and reference .ai-pack/
   - Use as documentation
   - Follow its patterns
   - Reference in your code

✅ DO update via git submodule:
   git submodule update --remote .ai-pack
```

### 📝 Extension Pattern

**To extend a role or add project-specific behavior:**

1. **Create extension in `.ai/roles/`:**
   ```bash
   mkdir -p .ai/roles/
   vim .ai/roles/<role-name>-extension.md
   ```

2. **Reference base role from `.ai-pack/roles/`:**
   ```markdown
   # <Role Name> Extension - [Project Name]

   **Base Role:** `.ai-pack/roles/<role-name>.md` (immutable, managed by ai-pack)
   **Extension Type:** Project-specific additions
   ```

3. **Document extension in `.ai/repo-overrides.md`:**
   ```markdown
   ## Role Extensions

   ### <Role Name> Extension
   **Extension Location:** `.ai/roles/<role-name>-extension.md`
   **Base Role:** `.ai-pack/roles/<role-name>.md`
   **Extension Summary:** [Brief description]
   ```

4. **Reference in CLAUDE.md** (if commonly used):
   ```markdown
   ## Role Extensions

   This project extends the following ai-pack roles:
   - **<Role Name>**: See [.ai/roles/<role-name>-extension.md](.ai/roles/<role-name>-extension.md)
   ```

**See:** [.ai-pack/ROLE-EXTENSION-GUIDE.md](.ai-pack/ROLE-EXTENSION-GUIDE.md) for complete guide

### ✅ DO
- Create task packets in `.ai/tasks/`
- Create role extensions in `.ai/roles/`
- Follow gates and workflows
- Update work logs regularly
- Reference standards when making decisions
- Document extensions in `.ai/repo-overrides.md`
- Ask questions when uncertain

### ❌ NEVER
- Edit files in `.ai-pack/` (immutable!)
- Add files to `.ai-pack/` (use `.ai/` instead)
- Put task packets in `.ai-pack/`
- Put role extensions in `.claude/` (use `.ai/roles/`)
- Overwrite `.ai/tasks/` during updates
- Skip gate checkpoints
- Proceed with failing tests
- Leave extensions undocumented

---

## Quick Reference

**Gates:** `.ai-pack/gates/`
**Roles:** `.ai-pack/roles/`
**Workflows:** `.ai-pack/workflows/`
**Templates:** `.ai-pack/templates/`
**Standards:** `.ai-pack/quality/`

**Task Packets:** `.ai/tasks/YYYY-MM-DD_task-name/`
**Overrides:** `.ai/repo-overrides.md` (optional)

---

## Getting Help

- **Framework Documentation:** See `.ai-pack/README.md`
- **Standards Index:** See `.ai-pack/quality/engineering-standards.md`
- **Workflow Guides:** See `.ai-pack/workflows/*.md`
- **Role Definitions:** See `.ai-pack/roles/*.md`

---

## Quick Reference: Orchestrator A2A Workflow

**You are Orchestrator. Here's your workflow:**

```bash
# 1. Check what's ready to work on
bd ready

# 2. Spawn agent for first ready task
.ai-pack/bin/agent engineer xasm++-vp5

# 3. Monitor progress
bd show xasm++-vp5

# 4. When complete, close and continue
bd close xasm++-vp5 -r "Complete"
bd ready  # Find next
.ai-pack/bin/agent engineer xasm++-m94  # IMMEDIATELY spawn next
```

**Parallel execution (for independent tasks):**
```bash
.ai-pack/bin/agent engineer xasm++-task1 &
.ai-pack/bin/agent engineer xasm++-task2 &
.ai-pack/bin/agent engineer xasm++-task3 &
wait
```

**DO:**
- ✅ Use A2A framework (`.ai-pack/bin/agent`)
- ✅ Monitor via Beads (`bd show`)
- ✅ Continue automatically to next phase
- ✅ Delegate to specialized agents

**DON'T:**
- ❌ Use Skill tool (deprecated)
- ❌ Use Task tool (broken)
- ❌ Do implementation work yourself
- ❌ Ask permission to continue

---

**Last Updated:** 2026-01-24
**Framework Version:** 2.1.0 (A2A Production)
