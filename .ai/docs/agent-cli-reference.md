# Agent CLI Reference

Complete reference for using the `agent` CLI to spawn and monitor A2A agents.

## Quick Start

```bash
# 1. Create Beads task
task_id=$(bd create "Task description" --priority high --json | jq -r '.id')

# 2. Spawn agent with --stream (blocks until complete)
agent engineer $task_id --stream

# 3. Close task when done
bd close $task_id -r "Complete"
```

## Detailed Usage

### Spawning Agents

**With --stream (MANDATORY for Orchestrators):**
```bash
agent engineer xasm++-vp5 --stream
# - Blocks until agent completes
# - Real-time progress via SSE
# - Returns immediately when agent finishes
# - Exit code: 0 = success, 1 = failure
```

**Fire and forget:**
```bash
agent engineer xasm++-vp5           # Spawns in background
# ... do other work ...
agent wait xasm++-vp5 --stream      # Block until complete
```

### Completion Detection Patterns

**✅ CORRECT: Use --stream (PREFERRED)**
```bash
agent engineer xasm++-abc --stream
# Command blocks until agent completes, then immediately returns
echo "Agent done"  # This runs AFTER agent finishes
```

**✅ CORRECT: Fire and forget, then wait with --stream**
```bash
agent engineer xasm++-abc
# ... do other work ...
agent wait xasm++-abc --stream  # Blocks with immediate notification
```

**❌ WRONG: Manual polling**
```bash
agent engineer xasm++-abc
while true; do
    status=$(agent status xasm++-abc | grep Status: | awk '{print $2}')
    [ "$status" = "completed" ] && break
    sleep 5
done
# DON'T DO THIS - use --stream instead
```

### Monitoring Commands

**Status checks:**
```bash
agent status xasm++-a1b2              # Human-readable
agent status xasm++-a1b2 --json       # Machine-readable JSON
agent status xasm++-a1b2 --quiet      # Just the status value
echo $?  # Exit code: 0=completed, 1=failed, 2=in_progress, 3=not found
```

**Log monitoring:**
```bash
agent logs xasm++-a1b2 --tail 50      # Last 50 lines
agent logs xasm++-a1b2 --follow       # Stream new lines (like tail -f)
agent logs --server --follow          # Monitor server logs in real-time
```

**List agents:**
```bash
agent list --running --json           # JSON output for scripting
agent list --completed                # Show completed agents
agent list --failed                   # Show failed agents
```

**Server health:**
```bash
agent metrics                         # Server health and stats
```

## Parallel Execution

### Pattern 1: Spawn all, then wait (RECOMMENDED)

```bash
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
```

### Pattern 2: Sequential (simpler but slower)

```bash
echo "🚀 Running agents sequentially..."
agent engineer xasm++-task1 --stream  # Blocks until done
echo "✓ Task 1 complete"

agent engineer xasm++-task2 --stream  # Blocks until done
echo "✓ Task 2 complete"

agent engineer xasm++-task3 --stream  # Blocks until done
echo "✓ Task 3 complete"
```

### Pattern 3: Multiple Workstreams

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

## Available Agents

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

## Agent CLI vs Task Tool

**Use agent CLI when:**
- Task is long-running (>10 minutes expected)
- Running multiple independent tasks in parallel
- Task should persist across sessions
- You want real-time progress monitoring

**Use Task tool when:**
- Task requires immediate results for next step
- Agent needs conversation context from current session
- Task is interactive (back-and-forth required)
- Task is very short (<5 minutes)

## Troubleshooting

**IF AGENT CLI FAILS:**
- 🛑 **STOP immediately** - Do NOT attempt workarounds
- 📢 **INFORM the user** with clear error message
- 📋 **PROVIDE diagnostic info**: error output, agent version, server status
- ⏸️ **WAIT for user** to fix the issue (server restart, CLI rebuild, etc.)
- ❌ **DO NOT** fall back to Task tool, HTTP calls, or other methods
