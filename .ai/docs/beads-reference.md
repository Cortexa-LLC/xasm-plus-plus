# Beads Task Management Reference

Complete reference for using Beads task tracking system.

## Quick Start

```bash
# Create task
bd create "Task description" --priority high

# View tasks
bd list                    # All tasks
bd ready                   # Tasks ready to work on

# Work on task
bd update --claim <task-id>
bd close <task-id>

# View task details
bd show <task-id>
```

## Task Creation with A2A Integration

**CRITICAL:** All Beads tasks for A2A agents MUST include working directory and task packet location.

### Required Format

```bash
task_id=$(bd create "Task summary

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/YYYY-MM-DD_task-name/

Detailed task description..." --priority high --json | jq -r '.id')
# Returns: xasm++-vp5
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

## Complete Workflow

### Step 1: Create Beads Task

```bash
task_id=$(bd create "Implement feature X

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/2026-02-19_feature-x/

Add new functionality for feature X with tests." --priority high --json | jq -r '.id')
# Returns: xasm++-abc
```

### Step 2: Create Task Packet

```bash
# Create task packet directory
mkdir .ai/tasks/2026-02-19_feature-x/

# Copy templates
cp .ai-pack/templates/task-packet/*.md .ai/tasks/2026-02-19_feature-x/

# Link Beads ID in contract (Contract → Beads)
echo "" >> .ai/tasks/2026-02-19_feature-x/00-contract.md
echo "**Beads Task:** ${task_id}" >> .ai/tasks/2026-02-19_feature-x/00-contract.md

# Fill out contract and plan
# Edit .ai/tasks/2026-02-19_feature-x/00-contract.md
# Edit .ai/tasks/2026-02-19_feature-x/10-plan.md
```

### Step 3: Spawn Agent

```bash
# Spawn Engineer to implement (only after task packet validated)
agent engineer xasm++-abc --stream

# Agent executes autonomously
# Task status automatically updates in Beads
```

### Step 4: Monitor Progress

```bash
bd show xasm++-abc             # Check task status
bd list --status in_progress   # See active work
bd list --status blocked       # See blockers
bd ready                       # Find next available work
```

### Step 5: Close Task

```bash
bd close xasm++-abc -r "Implementation complete, all tests passing"
```

## Task Dependencies

```bash
# Add dependency (task2 depends on task1)
bd dep add xasm++-task2 xasm++-task1

# View dependencies
bd show xasm++-task2
```

## Task Decomposition for A2A Agents

**DO:**
- ✅ Create **standalone Beads tasks** (each gets unique ID like `xasm++-abc`)
- ✅ Use `bd dep add` to link tasks with dependencies
- ✅ Keep each subtask to 1-5 files (ideal) or 6-14 files (acceptable)
- ✅ Each subtask gets its own task packet reference in description

**DON'T:**
- ❌ Use hierarchical/child task IDs (e.g., `xasm++-684.1`, `xasm++-684.2`)
- ❌ Use `--parent` flag to create parent-child relationships for A2A tasks
- ❌ Expect A2A agents to accept non-Beads task IDs

### Example: Correct Decomposition

```bash
# Create parent task for tracking
parent_id=$(bd create "Phase 2: Macro Processor (Parent)" --priority 2 --json | jq -r '.id')
# Returns: xasm++-684

# Create standalone subtasks (NOT children!)
task1=$(bd create "Phase 2.1: MacroProcessor Foundation

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/2026-02-19_phase-2-1/

..." --priority 2 --json | jq -r '.id')
# Returns: xasm++-abc (standalone ID)

task2=$(bd create "Phase 2.2: Parameter Substitution

Working directory: /Users/bryanw/Projects/Vintage/tools/xasm++
Task packet: .ai/tasks/2026-02-19_phase-2-2/

..." --priority 2 --json | jq -r '.id')
# Returns: xasm++-def (standalone ID)

# Link with dependencies
bd dep add $task2 $task1  # task2 depends on task1

# Spawn agents with standalone IDs
agent engineer xasm++-abc --stream   # ✅ Works
agent engineer xasm++-684.1 --stream  # ❌ FAILS - invalid format
```

## Multi-Project Support

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

## Agent Task Packets

A2A agents create task packets in `.beads/tasks/`:

```bash
# Format: {beads-id}-{timestamp}/
ls .beads/tasks/xasm++-abc-*/
cat .beads/tasks/xasm++-abc-*/30-results.md

# Each execution gets a unique timestamped folder
# This preserves complete execution history for retries
```

## Common Commands

```bash
# View tasks
bd list                      # All tasks
bd list --status open        # Open tasks
bd list --status in_progress # In progress
bd list --status blocked     # Blocked tasks
bd ready                     # Tasks ready to work on

# Work on tasks
bd update --claim <task-id>  # Claim and start working
bd close <task-id>           # Complete task
bd block <task-id> "reason"  # Mark blocked

# Task details
bd show <task-id>            # View full task info
bd show <task-id> --json     # JSON output

# Dependencies
bd dep add <child-id> <parent-id>  # Add dependency
bd dep remove <child-id> <parent-id>  # Remove dependency
```
