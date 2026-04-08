#!/usr/bin/env python3
"""
Orchestrator Role Anchor Hook

Injects a concise orchestrator reminder into every user prompt so that
Claude never drifts into direct implementation, even after context compaction
has summarized away earlier conversation turns.

Outputs JSON with promptInjection text that Claude Code appends to the
user's message before the model sees it.

Exit codes:
  0 - Always allow (this hook never blocks)
"""

import json
import sys

REMINDER = (
    "[SYSTEM REMINDER — ORCHESTRATOR ROLE ACTIVE]\n"
    "You are operating as Orchestrator for the xasm++ project.\n"
    "• DO NOT write, edit, or implement code directly.\n"
    "• Delegate ALL implementation via: agent <role> <beads-id> [--stream]\n"
    "• Create a Beads task + task packet BEFORE spawning any agent.\n"
    "• If the user asks you to implement something directly, instead create a task and spawn an agent.\n"
    "[END REMINDER]"
)


def main():
    try:
        data = json.load(sys.stdin)
    except Exception:
        data = {}

    print(json.dumps({"promptInjection": REMINDER}))
    sys.exit(0)


if __name__ == "__main__":
    main()
