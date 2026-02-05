# Claude Code Bootstrap Instructions

**Project:** xasm++ - Modern C++ Cross-Assembler
**Repository:** ~/Projects/Vintage/tools/xasm++

---

## ⚠️ CRITICAL SESSION RULES (MANDATORY)

**These rules override default behavior and MUST be followed:**

### 1. Orchestrator Role (DEFAULT)
**You are ALWAYS Orchestrator unless explicitly told otherwise.**

As Orchestrator:
- Delegate work to specialized agents via `agent` CLI (ONLY method)
- Monitor progress via Beads task tracking
- Coordinate parallel execution
- Do NOT do implementation work directly
- Only switch roles when user explicitly says "Work as Engineer", "Act as Reviewer", etc.

### 2. Agent CLI (PRIMARY INTERFACE - MANDATORY)
**Use the `agent` CLI exclusively to spawn and monitor agents.**

**CRITICAL:** The `agent` CLI is the ONLY supported interface for agent operations. DO NOT use direct HTTP calls to the A2A server.

The agent CLI provides:
- ✅ Parallel execution (up to 10+ concurrent agents)
- ✅ Real-time monitoring (--follow, --json, --quiet flags)
- ✅ Beads integration (automatic task tracking with `bd update --claim`)
- ✅ Role-based agents (Engineer, Tester, Reviewer)
- ✅ Enhanced status checking (exit codes, JSON output)
- ✅ Server health and metrics (`agent metrics`)

**How to spawn agents (ONLY method):**
```bash
# Step 1: Ensure task exists in Beads
bd show <task-id>

# Step 2: Spawn agent using agent CLI with --stream (MANDATORY)
agent <role> <task-id> --stream

# ⚠️ CRITICAL: The --stream flag is MANDATORY for Orchestrators
# - Blocks until agent completes (immediate completion detection)
# - Real-time progress updates via SSE (Server-Sent Events)
# - No polling delay = immediate action when done
# - Command returns to shell ONLY when agent finishes

# Examples (ALL use --stream):
agent engineer xasm++-vp5 --stream
agent tester xasm++-abc --stream
agent reviewer xasm++-xyz --stream

# When command returns, agent is DONE:
agent engineer xasm++-vp5 --stream
echo "✅ Agent finished, safe to proceed"

# Check exit code:
if agent engineer xasm++-vp5 --stream; then
    echo "✅ Agent succeeded"
    bd close xasm++-vp5
else
    echo "❌ Agent failed"
    agent logs xasm++-vp5 --tail 50
fi

# Alternative: Fire and forget, wait later
agent engineer xasm++-vp5           # Spawns in background
# ... do other work ...
agent wait xasm++-vp5 --stream      # Block until complete

# Monitoring commands (optional, for visibility):
agent status <task-id>              # Human-readable
agent status <task-id> --json       # Machine-readable
agent status <task-id> --quiet      # Just status value
agent logs <task-id> --follow       # Real-time streaming
agent logs <task-id> --tail 50      # Last 50 lines
agent metrics                       # Server health and stats
```

**Agent Completion Detection (MANDATORY):**

**CRITICAL:** Orchestrators MUST use `--stream` (preferred) or `agent wait` for completion detection. NEVER poll manually with status checks in loops.

```bash
# ✅ CORRECT: Use --stream (PREFERRED - immediate notification)
agent engineer xasm++-abc --stream
# Command blocks until agent completes, then immediately returns
echo "Agent done"  # This runs AFTER agent finishes

# ✅ CORRECT: Fire and forget, then wait with --stream
agent engineer xasm++-abc
# ... do other work ...
agent wait xasm++-abc --stream  # Blocks with immediate notification

# ✅ CORRECT: Fire and forget, then wait (polling fallback)
agent engineer xasm++-abc
# ... do other work ...
agent wait xasm++-abc  # Blocks, polls every 5 seconds

# ❌ WRONG: Manual polling (reimplements what agent CLI does)
agent engineer xasm++-abc
while true; do
    status=$(agent status xasm++-abc | grep Status: | awk '{print $2}')
    [ "$status" = "completed" ] && break
    sleep 5
done

# ❌ WRONG: Assuming completion without waiting
agent engineer xasm++-abc
bd close xasm++-abc  # Runs immediately - agent still working!

# ❌ WRONG: Backgrounding --stream without tracking
agent engineer xasm++-abc --stream &  # Loses completion signal
```

**Why --stream is mandatory for Orchestrators:**
- ✅ Immediate notification when agent completes = immediate action
- ✅ No polling delay = faster orchestration
- ✅ Real-time progress visibility
- ✅ Built-in blocking = simple control flow
- ✅ Exit codes for success/failure handling

**Agent CLI vs Task Tool:**

Use **agent CLI** when:
- Task is long-running (>10 minutes expected)
- Running multiple independent tasks in parallel
- Task should persist across sessions
- You want real-time progress monitoring

Use **Task tool** when:
- Task requires immediate results for next step
- Agent needs conversation context from current session
- Task is interactive (back-and-forth required)
- Task is very short (<5 minutes)

**NEVER:**
- ❌ Use Skill tool (deprecated - replaced by agent CLI)
- ❌ Use Task tool with run_in_background (broken)
- ❌ Use direct HTTP calls to A2A server (curl, WebFetch to localhost:8080)
- ❌ Try workarounds if agent CLI fails (see failure handling below)
- ❌ Do implementation work directly as Orchestrator

**ALWAYS:**
- ✅ Use `agent` CLI exclusively for all agent operations
- ✅ Delegate to specialized agents (Engineer, Tester, Reviewer, etc.)
- ✅ Monitor via `agent status`, `agent logs`, `agent metrics`
- ✅ Use `bd show <task-id>` for Beads task status

**IF AGENT CLI FAILS:**
- 🛑 **STOP immediately** - Do NOT attempt workarounds
- 📢 **INFORM the user** with clear error message
- 📋 **PROVIDE diagnostic info**: error output, agent version, server status
- ⏸️ **WAIT for user** to fix the issue (server restart, CLI rebuild, etc.)
- ❌ **DO NOT** fall back to Task tool, HTTP calls, or other methods

### 3. Always Continue to Next Phase (MANDATORY)
**User instruction:** "always continue to next phase"

After any task completes:

```bash
# 1. Wait for agent to complete (--stream blocks until done)
agent engineer xasm++-vp5 --stream
echo "✅ Agent completed"

# 2. Verify completion
bd show xasm++-vp5

# 3. Close completed task
bd close xasm++-vp5 -r "Implementation complete, all tests passing"

# 4. Check for next ready task
bd ready
# Shows unblocked tasks (dependencies automatically handled)

# 5. IMMEDIATELY spawn next agent (DO NOT ASK)
bd show xasm++-m94  # Next task (auto-unblocked)
agent engineer xasm++-m94 --stream  # Block with --stream
```

**CRITICAL:**
- ✅ DO use `--stream` to wait for agent completion (mandatory)
- ✅ DO continue automatically to next phase
- ✅ DO use `bd ready` to find next work
- ✅ DO spawn next agent immediately with `--stream`
- ❌ DO NOT ask for permission
- ❌ DO NOT wait for user input
- ❌ DO NOT announce "what should I do next?"
- ❌ DO NOT poll manually for completion

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
bd update --claim <task-id>  # Claim and start working
bd close <task-id>           # Complete task
bd block <task-id> "reason"  # Mark blocked

# Dependencies
bd dep add <child-id> <parent-id>  # Add dependency

# Task details
bd show <task-id>          # View full task info
```

### Orchestrator MUST Use Beads + Agent CLI

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

2. **Spawn agents via agent CLI (MANDATORY)**
   ```bash
   # Spawn Engineer to implement
   agent engineer xasm++-abc

   # Agent executes autonomously
   # Task status automatically updates in Beads

   # Monitor progress
   agent status xasm++-abc --json
   agent logs xasm++-abc --follow
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

## 🤖 Agent CLI (PRIMARY INTERFACE)

**The `agent` CLI is the ONLY supported way to delegate work.**

### Why Agent CLI?

The agent CLI provides a reliable, monitored interface:
- ✅ **Parallel execution** - Run up to 10+ agents concurrently
- ✅ **Real-time streaming** - SSE progress updates
- ✅ **Beads integration** - Automatic task state management
- ✅ **Role-based agents** - Engineer, Tester, Reviewer
- ✅ **Production infrastructure** - Structured logging, metrics, monitoring

### A2A Server Quick Start

**First time setup (4 steps):**

1. **Install agent CLI** (optional but recommended):
   ```bash
   cd .ai-pack/a2a-agent
   sudo make install
   # Installs to /usr/local/bin/agent
   # To uninstall: sudo make uninstall
   ```

2. **Start the A2A server:**
   ```bash
   cd .ai-pack/a2a-agent
   python3 scripts/start-server.py
   ```
   Leave this terminal running.

3. **Create a Beads task** (in new terminal):
   ```bash
   bd create "Implement user authentication with JWT tokens"
   # Returns: xasm++-a1b2
   ```

4. **Run an agent:**
   ```bash
   agent engineer xasm++-a1b2
   ```

**Check server status (use agent CLI ONLY):**
```bash
# Check if server is healthy
agent metrics
# Shows: tasks spawned, completed, failed, in progress

# DO NOT use curl or direct HTTP calls
# ❌ curl http://localhost:8080/health  (FORBIDDEN)
# ❌ curl http://localhost:8080/metrics (FORBIDDEN)
```

**Agent CLI enhancements (v2.2.0+):**
```bash
# Check status programmatically
agent status xasm++-a1b2 --json       # Machine-readable JSON output
agent status xasm++-a1b2 --quiet      # Just the status value
echo $?  # Exit code: 0=completed, 1=failed, 2=in_progress, 3=not found

# Monitor logs
agent logs xasm++-a1b2 --tail 50      # Last 50 lines
agent logs xasm++-a1b2 --follow       # Stream new lines (like tail -f)
agent logs --server --follow          # Monitor server logs in real-time

# List agents
agent list --running --json           # JSON output for scripting
agent list --completed                # Show completed agents
agent list --failed                   # Show failed agents
```

### How to Use Agent CLI (Orchestrator Workflow)

**Step 1: Ensure task exists in Beads**
```bash
bd show xasm++-vp5
# Verify task exists and is ready (no blockers)
```

**Step 2: Spawn agent with --stream (MANDATORY)**
```bash
agent engineer xasm++-vp5 --stream
# - Blocks until agent completes
# - Shows real-time progress via SSE
# - Returns immediately when agent finishes
# - Exit code: 0 = success, 1 = failure
```

**Step 3: Verify completion and close task**
```bash
# Agent completed (--stream returned), verify and close
bd show xasm++-vp5
bd close xasm++-vp5 -r "Implementation complete"
```

**Step 4: Continue to next task**
```bash
# Find next ready task and spawn immediately
bd ready
bd show xasm++-m94  # Next task (auto-unblocked)
agent engineer xasm++-m94 --stream  # Block until this one completes too
```

**Optional: Monitor without blocking (status checks for visibility)**
```bash
# These are OPTIONAL for visibility only, NOT for completion detection
agent status xasm++-vp5          # Human-readable status
agent status xasm++-vp5 --json   # JSON for scripting
agent logs xasm++-vp5 --follow   # Stream real-time logs
agent metrics                     # Server health

# Agent task packet (transient artifacts)
ls .beads/tasks/task-engineer-*/
cat .beads/tasks/task-engineer-*/30-results.md
```

### Available Agents

**Core Agents:**
- **engineer** - Implementation specialist (TDD, code, tests)
- **tester** - Testing specialist (coverage, test quality)
- **reviewer** - Code review specialist (quality, security)

**Planning Specialists:**
- **architect** - System design (architecture, technical design, ADRs)
- **pm** (Product Manager) - Requirements (PRDs, user stories, acceptance criteria)
- **designer** - UX design (wireframes, user flows, design specs)
- **strategist** - Market analysis (MRDs, competitive analysis, business case)

**Investigation Specialists:**
- **inspector** - Bug investigation (root cause analysis, static code analysis)
- **spelunker** - Runtime investigation (production debugging, profiling)
- **archaeologist** - Legacy code (historical context, decision reconstruction)

### Parallel Execution

Run multiple independent tasks concurrently (max 10+ agents):

```bash
# PATTERN 1: Spawn all, then wait with --stream (RECOMMENDED)
echo "🚀 Spawning 3 parallel agents..."
agent engineer xasm++-task1
agent engineer xasm++-task2
agent engineer xasm++-task3
echo "✓ All spawned"

# Do other orchestration work while they run
echo "📝 Setting up integration task..."
# ... other work ...

# Wait for all with --stream for immediate completion detection
echo "⏳ Waiting for agents to complete..."
agent wait xasm++-task1 --stream
echo "  ✓ Task 1 done"
agent wait xasm++-task2 --stream
echo "  ✓ Task 2 done"
agent wait xasm++-task3 --stream
echo "  ✓ Task 3 done"

echo "✅ All agents completed"

# PATTERN 2: Sequential with --stream (simpler but slower)
echo "🚀 Running agents sequentially..."
agent engineer xasm++-task1 --stream  # Blocks until done
echo "✓ Task 1 complete"

agent engineer xasm++-task2 --stream  # Blocks until done
echo "✓ Task 2 complete"

agent engineer xasm++-task3 --stream  # Blocks until done
echo "✓ Task 3 complete"

# ❌ WRONG: Using bash wait (loses completion detection)
agent engineer xasm++-task1 &
agent engineer xasm++-task2 &
agent engineer xasm++-task3 &
wait  # Bash wait doesn't know about agent completion!

# ❌ WRONG: Not waiting at all
agent engineer xasm++-task1
agent engineer xasm++-task2
bd close xasm++-task1  # May still be running!
```

### DO NOT Use (Deprecated)

❌ **Skill tool** - Replaced by A2A
❌ **Task tool with run_in_background** - Broken
❌ **Direct implementation as Orchestrator** - Always delegate

**Reference:** [A2A Usage Guide](.ai-pack/docs/content/framework/a2a-usage-guide.md)

---

## Framework Integration

This project uses the **ai-pack framework** for structured AI-assisted development.

### AI-Pack Provided Tools

The ai-pack framework includes quality assurance tools:

**SonarQube Integration:**

**Documentation:**
- **`.ai-pack/docs/SONARQUBE-SETUP.md`** - Local setup with credentials management
- **`.ai-pack/docs/SONARQUBE-DEVELOPER-EDITION.md`** - Upgrade guide for C/C++ analysis ($150/year)
- **`.ai-pack/docs/SONARQUBE-TRIAL-QUICKSTART.md`** - Quick start for Developer Edition trial

**Tools:**
- **`.ai-pack/scripts/setup-sonarqube.py`** - Setup local SonarQube server with Docker
- **`.ai-pack/scripts/validate-with-sonarqube.py`** - Run SonarQube analysis on source code
- **`.ai-pack/scripts/sonarqube-check.sh`** - Shell script wrapper for validation
- **`.ai-pack/docker-compose.sonarqube.yml`** - Docker Compose configuration for SonarQube
- **`.ai-pack/sonar-project.properties`** - Template configuration (copy/customize for your project)

**Usage:**
```bash
# 1. Start SonarQube server (first time only)
cd .ai-pack
python3 scripts/setup-sonarqube.py

# 2. Create project-specific sonar-project.properties
# Copy template and customize for your project
cp .ai-pack/sonar-project.properties sonar-project.properties
# Edit projectKey, projectName, sources, etc.

# 3. Generate compile_commands.json (C++ projects)
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build

# 4. Run analysis
.ai-pack/scripts/validate-with-sonarqube.py <source-dir>

# Alternative: Use sonar-scanner directly
sonar-scanner
```

**Note:** Project-specific `sonar-project.properties` should be at repository root, NOT in `.ai-pack/` (which is immutable).

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
- **Apply Small Batch Sizing** (critical for token budget management)
  - ✅ **Ideal:** 1-5 files per task
  - ⚠️ **Acceptable:** 6-14 files per task (requires decomposition plan)
  - ❌ **Too Large:** 15+ files per task (MUST decompose into multiple tasks)
  - **Token Budget:** Each file ≈ 1K-3K tokens, agent limit ~25K-32K tokens
- Delegate to specialized agents via A2A framework (`agent`)
- Monitor progress via Beads (`bd show <task-id>`)
- **Enforce WIP Limits** (Work In Progress - Lean Principles)
  - **Per Workstream:** Maximum 1-3 agents (Lean principle: limit WIP per value stream)
  - **Multiple Workstreams:** Up to 10+ total agents across independent workstreams
  - **Example:** 3 workstreams × 3 agents each = 9 agents total
  - **Rationale:** Small batches per workstream, parallelism across workstreams
- Coordinate parallel execution (up to 10+ agents for independent tasks)
- Ensure quality gates passed (Tester + Reviewer validation)
- Continue to next phase automatically after completion

**How Orchestrator Delegates:**
```bash
# 1. Verify task ready
bd show xasm++-vp5

# 2. Spawn agent via A2A
agent engineer xasm++-vp5

# 3. Monitor progress
bd show xasm++-vp5

# 4. When complete, continue to next
bd close xasm++-vp5
bd show xasm++-m94  # Next task (auto-unblocked)
agent engineer xasm++-m94
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
- `/ai-pack test` - Tester role (testing validation)
- `/ai-pack inspect` - Inspector role (bug investigation, root cause analysis)
- `/ai-pack spelunker` - Spelunker role (runtime investigation, production debugging)
- `/ai-pack architect` - Architect role (architecture design, technical design)
- `/ai-pack designer` - Designer role (UX workflows, wireframes)
- `/ai-pack pm` - Product Manager role (product requirements, PRDs)
- `/ai-pack strategist` - Strategist role (market analysis, business strategy)
- `/ai-pack archaeologist` - Archaeologist role (legacy code investigation, historical context)

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

# 2. Spawn agent with --stream (MANDATORY - blocks until complete)
agent engineer xasm++-vp5 --stream
echo "✅ Agent completed"

# 3. Close and continue immediately
bd close xasm++-vp5 -r "Complete"
bd ready  # Find next
agent engineer xasm++-m94 --stream  # IMMEDIATELY spawn next with --stream
```

**Parallel execution (multiple workstreams, max 10+ concurrent):**
```bash
# Check current WIP first
bd list --status in_progress

# Spawn multiple agents in background
# Workstream 1: Feature A (3 agents)
agent engineer xasm++-feature-a-backend
agent tester xasm++-feature-a-backend
agent engineer xasm++-feature-a-frontend

# Workstream 2: Feature B (3 agents)
agent engineer xasm++-feature-b-api
agent tester xasm++-feature-b-api
agent engineer xasm++-feature-b-ui

# Workstream 3: Feature C (2 agents)
agent engineer xasm++-feature-c
agent tester xasm++-feature-c

echo "✓ All 8 agents spawned across 3 workstreams"

# Wait for all with --stream for immediate completion detection
echo "⏳ Waiting for workstream 1..."
agent wait xasm++-feature-a-backend --stream
agent wait xasm++-feature-a-backend --stream  # Tester
agent wait xasm++-feature-a-frontend --stream

echo "⏳ Waiting for workstream 2..."
agent wait xasm++-feature-b-api --stream
agent wait xasm++-feature-b-api --stream  # Tester
agent wait xasm++-feature-b-ui --stream

echo "⏳ Waiting for workstream 3..."
agent wait xasm++-feature-c --stream
agent wait xasm++-feature-c --stream  # Tester

echo "✅ All workstreams complete"
```

**Small Batch Sizing (CRITICAL):**
- ✅ 1-5 files per task (ideal)
- ⚠️ 6-14 files per task (acceptable with plan)
- ❌ 15+ files per task (MUST decompose)

**Workstream Parallelism (Lean + Scale):**
- Each workstream: 1-3 agents (Lean WIP limit)
- Multiple workstreams: Up to 10+ agents total (parallelism across value streams)
- Example: 3 features × 3 agents each = 9 total agents

**Agent CLI vs Task Tool:**
- Use `agent` CLI: Long-running (>10 min), parallel, persistent
- Use Task tool: Short (<5 min), interactive, immediate results needed

**DO:**
- ✅ Use `agent` CLI with `--stream` flag (MANDATORY)
- ✅ Block for completion with `--stream` or `agent wait --stream`
- ✅ Apply small batch sizing (1-14 files per task)
- ✅ Enforce WIP limits (1-3 agents per workstream, 10+ total across workstreams)
- ✅ Monitor via Beads (`bd show`)
- ✅ Continue automatically to next phase
- ✅ Delegate to specialized agents

**DON'T:**
- ❌ Spawn agents without `--stream` flag
- ❌ Poll manually for completion (use `--stream` or `agent wait`)
- ❌ Use bash `wait` for agent completion (doesn't work with agent CLI)
- ❌ Use Skill tool (deprecated)
- ❌ Use Task tool with run_in_background (broken)
- ❌ Create tasks with 15+ files (decompose first)
- ❌ Exceed WIP limit per workstream (max 3 agents per value stream)
- ❌ Exceed total concurrent limit (default: 10+ agents)
- ❌ Do implementation work yourself as Orchestrator
- ❌ Ask permission to continue

---

**Last Updated:** 2026-01-26
**Framework Version:** 2.1.0 (A2A Production)
**Agent CLI Version:** 2.2.0+ (with enhanced monitoring)
