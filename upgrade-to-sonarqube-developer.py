#!/usr/bin/env python3
"""
Upgrade SonarQube to a new Developer Edition version.

CRITICAL RULES:
  1. NEVER use 'docker-compose down -v' -- this destroys the database and resets the server ID
  2. Always use 'docker-compose down' (no -v) to preserve volumes
  3. Follow the LTA upgrade path -- you cannot skip major versions:
       v10.x -> 2026.1-developer (LTA) -> developer (latest)

The server ID is stored in the PostgreSQL volume (postgresql_data).
As long as volumes are preserved, the server ID and license survive upgrades.
"""

import re
import subprocess
import sys
import time
from pathlib import Path

COMPOSE_BASE = ".ai-pack/docker-compose.sonarqube.yml"
COMPOSE_DEV = "docker-compose.sonarqube-dev.yml"


def run(cmd: list[str], check: bool = True) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, check=check)


def current_image() -> str:
    text = Path(COMPOSE_DEV).read_text()
    m = re.search(r"image:\s*(\S+)", text)
    return m.group(1) if m else "(unknown)"


def set_image(tag: str) -> None:
    path = Path(COMPOSE_DEV)
    text = path.read_text()
    text = re.sub(r"image:\s*sonarqube:\S+", f"image: sonarqube:{tag}", text)
    path.write_text(text)


def main() -> None:
    print("Upgrading SonarQube Developer Edition...")
    print()
    print("CRITICAL: This script uses 'down' (NOT 'down -v') to preserve:")
    print("  - Server ID (required for license binding)")
    print("  - All projects, settings, and history")
    print()
    print("Upgrade path: you cannot skip LTA versions.")
    print("  v10.x  -> sonarqube:2026.1-developer  (required intermediate step)")
    print("  2026.1 -> sonarqube:developer          (latest)")
    print()
    print(f"Current image: {current_image()}")
    print()

    tag = input("Target image tag (e.g. 2026.1-developer or developer): ").strip()
    if not tag:
        print("No tag provided. Cancelled.")
        sys.exit(1)

    print(f"\nPulling sonarqube:{tag}...")
    run(["docker", "pull", f"sonarqube:{tag}"])

    print("\nStopping SonarQube (preserving volumes)...")
    # NOTE: No -v flag -- volumes are intentionally preserved to keep server ID
    run(["docker-compose", "-f", COMPOSE_BASE, "-f", COMPOSE_DEV, "down"])

    set_image(tag)
    print(f"Updated {COMPOSE_DEV} to sonarqube:{tag}")

    print("\nStarting SonarQube...")
    run(["docker-compose", "-f", COMPOSE_BASE, "-f", COMPOSE_DEV, "up", "-d"])

    print("\nWaiting 60 seconds for startup and schema migration...")
    time.sleep(60)

    print()
    print("SonarQube is starting. Check logs if needed:")
    print("  docker logs sonarqube | grep -E 'SonarQube is up|ERROR'")
    print()
    print("Next steps:")
    print("  1. Open: http://localhost:9000")
    print("  2. If prompted to run DB migration wizard, follow the on-screen steps")
    print("  3. Go to: Administration -> Configuration -> License Manager")
    print("  4. Verify license is still active (server ID should be unchanged)")
    print("  5. Check: Administration -> Marketplace -> Installed -> C/C++ (CFamily)")
    print()
    print("Run analysis:")
    print("  source sonar.env && sonar-scanner")


if __name__ == "__main__":
    main()
