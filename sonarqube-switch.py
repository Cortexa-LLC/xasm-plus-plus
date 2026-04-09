#!/usr/bin/env python3
"""
Switch between SonarQube Community and Developer editions.

Each edition uses a separate Docker project name, giving it isolated volumes:
  Developer Edition:  project 'ai-pack'       -> volumes ai-pack_*
  Community Edition:  project 'sonarqube-ce'  -> volumes sonarqube-ce_*

Volumes are NEVER removed on switch -- both editions' data coexist on disk.
Switch back at any time without data loss.

CRITICAL: Never add -v to docker-compose down calls in this script.
"""

import subprocess
import sys
import time

COMPOSE_BASE = ".ai-pack/docker-compose.sonarqube.yml"

EDITIONS = {
    "dev": {
        "label": "Developer Edition",
        "project": "ai-pack",
        "compose_override": "docker-compose.sonarqube-dev.yml",
    },
    "ce": {
        "label": "Community Edition",
        "project": "sonarqube-ce",
        "compose_override": "docker-compose.sonarqube-ce.yml",
    },
}


def compose_cmd(edition: dict, *args: str) -> list[str]:
    return [
        "docker-compose",
        "-p", edition["project"],
        "-f", COMPOSE_BASE,
        "-f", edition["compose_override"],
        *args,
    ]


def is_running(edition: dict) -> bool:
    result = subprocess.run(
        compose_cmd(edition, "ps", "-q"),
        capture_output=True, text=True
    )
    return bool(result.stdout.strip())


def stop_edition(edition: dict) -> None:
    if is_running(edition):
        print(f"Stopping {edition['label']} (preserving volumes)...")
        # No -v -- volumes are intentionally preserved
        subprocess.run(compose_cmd(edition, "down"), check=True)
    else:
        print(f"{edition['label']} is not running, skipping stop.")


def start_edition(edition: dict) -> None:
    print(f"Starting {edition['label']}...")
    subprocess.run(compose_cmd(edition, "up", "-d"), check=True)
    print("Waiting 60 seconds for startup...")
    time.sleep(60)
    print(f"\n{edition['label']} is up at http://localhost:9000")


def main() -> None:
    targets = {"dev": "Developer Edition", "ce": "Community Edition"}

    if len(sys.argv) != 2 or sys.argv[1] not in targets:
        print("Usage: sonarqube-switch.py <ce|dev>")
        print()
        for key, label in targets.items():
            e = EDITIONS[key]
            running = "RUNNING" if is_running(e) else "stopped"
            print(f"  {key}  {label} [{running}]  volumes: {e['project']}_*")
        sys.exit(1)

    target_key = sys.argv[1]
    other_key = "dev" if target_key == "ce" else "ce"

    target = EDITIONS[target_key]
    other = EDITIONS[other_key]

    print(f"Switching to {target['label']}...")
    print()

    stop_edition(other)
    start_edition(target)

    print()
    print("To switch back:")
    print(f"  python3 sonarqube-switch.py {other_key}")


if __name__ == "__main__":
    main()
