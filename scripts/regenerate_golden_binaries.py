#!/usr/bin/env python3
"""
Regenerate golden reference binaries for binary compatibility tests.

This script generates reference binaries using vasm-ext assemblers and stores
them as golden files for comparison against xasm++ output. Golden files are
checked into git and used by the test suite.

Usage:
    python scripts/regenerate_golden_binaries.py               # All combinations
    python scripts/regenerate_golden_binaries.py --syntax merlin  # Only Merlin
    python scripts/regenerate_golden_binaries.py --cpu 65c02   # Only 65C02
    python scripts/regenerate_golden_binaries.py --force       # Rebuild vasm-ext

Requirements:
    - vasm-ext source at ~/Projects/Vintage/tools/vasm-ext
    - make (Unix) or cmake (Windows) for vasm-ext build
    - Python 3.7+
    - No external pip packages (stdlib only)

Cross-platform:
    - Works on Windows, macOS, Linux
    - Uses pathlib for cross-platform paths
    - Detects OS and uses appropriate build tools
"""

import argparse
import os
import platform
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Tuple


class VasmBuilder:
    """Build vasm-ext assemblers if needed."""
    
    def __init__(self, vasm_root: Path, force_rebuild: bool = False):
        self.vasm_root = vasm_root
        self.force_rebuild = force_rebuild
        self.system = platform.system()
        
    def get_vasm_binary(self, syntax: str) -> Path:
        """Get path to vasm binary for given syntax."""
        binary_name = f"vasm6502_{syntax}"
        if self.system == "Windows":
            binary_name += ".exe"
        return self.vasm_root / binary_name
    
    def needs_build(self, syntax: str) -> bool:
        """Check if vasm binary needs to be built."""
        if self.force_rebuild:
            return True
        binary = self.get_vasm_binary(syntax)
        return not binary.exists()
    
    def build(self, syntax: str) -> bool:
        """Build vasm binary for given syntax."""
        if not self.vasm_root.exists():
            print(f"ERROR: vasm-ext not found at {self.vasm_root}")
            return False
        
        print(f"Building vasm6502_{syntax}...")
        
        try:
            if self.system == "Windows":
                # Windows: Use cmake if available, otherwise fail
                result = subprocess.run(
                    ["cmake", "--version"],
                    capture_output=True,
                    check=False
                )
                if result.returncode != 0:
                    print("ERROR: cmake required for Windows builds")
                    return False
                
                # CMake build process (would need CMakeLists.txt in vasm-ext)
                print("WARNING: Windows build process needs implementation")
                return False
            else:
                # Unix: Use make
                result = subprocess.run(
                    ["make", f"CPU=6502", f"SYNTAX={syntax}"],
                    cwd=self.vasm_root,
                    capture_output=True,
                    text=True,
                    check=False
                )
                
                if result.returncode != 0:
                    print(f"ERROR: Build failed:\n{result.stderr}")
                    return False
                
                print(f"  SUCCESS: {self.get_vasm_binary(syntax)}")
                return True
                
        except FileNotFoundError as e:
            print(f"ERROR: Build tool not found: {e}")
            return False
        except Exception as e:
            print(f"ERROR: Build failed: {e}")
            return False


class GoldenBinaryGenerator:
    """Generate golden reference binaries using vasm-ext."""
    
    def __init__(self, project_root: Path, vasm_root: Path):
        self.project_root = project_root
        self.vasm_root = vasm_root
        self.test_sources_dir = project_root / "tests" / "integration" / "binary_compat" / "test_sources"
        self.golden_dir = project_root / "tests" / "integration" / "binary_compat" / "golden"
        self.stats = {
            "generated": 0,
            "failed": 0,
            "skipped": 0
        }
    
    def get_vasm_flags(self, cpu: str) -> List[str]:
        """Get vasm command-line flags for CPU variant."""
        flags = ["-Fbin"]  # Binary output format
        
        if cpu == "65c02":
            flags.append("-m65c02")
        elif cpu == "65816":
            flags.append("-m65816")
        # 6502 needs no special flag
        
        return flags
    
    def find_test_sources(self, syntax: Optional[str], cpu: Optional[str]) -> List[Tuple[str, str, Path]]:
        """Find all test source files matching filters."""
        sources = []
        
        syntaxes = [syntax] if syntax else ["merlin", "scmasm"]
        cpus = [cpu] if cpu else ["6502", "65c02", "65816"]
        
        for syn in syntaxes:
            for cpu_variant in cpus:
                source_dir = self.test_sources_dir / syn / cpu_variant
                if not source_dir.exists():
                    continue
                
                for asm_file in sorted(source_dir.glob("*.asm")):
                    sources.append((syn, cpu_variant, asm_file))
        
        return sources
    
    def generate_golden(self, syntax: str, cpu: str, source_file: Path, vasm_binary: Path) -> bool:
        """Generate golden binary for a single test source."""
        # Create output path
        relative_name = source_file.stem  # Filename without .asm extension
        golden_output = self.golden_dir / syntax / cpu / f"{relative_name}.bin"
        golden_output.parent.mkdir(parents=True, exist_ok=True)
        
        # Assemble with vasm
        vasm_flags = self.get_vasm_flags(cpu)
        cmd = [str(vasm_binary)] + vasm_flags + ["-o", str(golden_output), str(source_file)]
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=False
            )
            
            if result.returncode != 0:
                print(f"  FAILED: {source_file.name}")
                print(f"    {result.stderr.strip()}")
                self.stats["failed"] += 1
                return False
            
            # Check output file was created
            if not golden_output.exists():
                print(f"  FAILED: {source_file.name} (no output)")
                self.stats["failed"] += 1
                return False
            
            size = golden_output.stat().st_size
            print(f"  ✓ {source_file.name} → {relative_name}.bin ({size} bytes)")
            self.stats["generated"] += 1
            return True
            
        except Exception as e:
            print(f"  ERROR: {source_file.name}: {e}")
            self.stats["failed"] += 1
            return False
    
    def generate_all(self, syntax: Optional[str], cpu: Optional[str]) -> bool:
        """Generate all golden binaries matching filters."""
        sources = self.find_test_sources(syntax, cpu)
        
        if not sources:
            print("No test sources found matching filters")
            return False
        
        print(f"Found {len(sources)} test source(s)")
        print()
        
        # Group by syntax for organized output
        by_syntax = {}
        for syn, cpu_variant, path in sources:
            key = (syn, cpu_variant)
            if key not in by_syntax:
                by_syntax[key] = []
            by_syntax[key].append(path)
        
        # Generate for each syntax/CPU combination
        for (syn, cpu_variant), files in sorted(by_syntax.items()):
            vasm_binary = self.vasm_root / f"vasm6502_{syn}"
            
            if not vasm_binary.exists():
                print(f"WARNING: {vasm_binary.name} not found, skipping {syn}/{cpu_variant}")
                self.stats["skipped"] += len(files)
                continue
            
            print(f"=== {syn} + {cpu_variant.upper()} ===")
            for source_file in files:
                self.generate_golden(syn, cpu_variant, source_file, vasm_binary)
            print()
        
        return True


def main():
    parser = argparse.ArgumentParser(
        description="Regenerate golden reference binaries for xasm++ binary compatibility tests"
    )
    parser.add_argument(
        "--syntax",
        choices=["merlin", "scmasm"],
        help="Only regenerate for specific syntax"
    )
    parser.add_argument(
        "--cpu",
        choices=["6502", "65c02", "65816"],
        help="Only regenerate for specific CPU"
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Force rebuild vasm-ext even if binaries exist"
    )
    parser.add_argument(
        "--vasm-root",
        type=Path,
        default=Path.home() / "Projects" / "Vintage" / "tools" / "vasm-ext",
        help="Path to vasm-ext directory (default: ~/Projects/Vintage/tools/vasm-ext)"
    )
    
    args = parser.parse_args()
    
    # Find project root
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True,
            text=True,
            check=True
        )
        project_root = Path(result.stdout.strip())
    except subprocess.CalledProcessError:
        print("ERROR: Must run from within git repository")
        return 1
    except FileNotFoundError:
        print("ERROR: git command not found")
        return 1
    
    print("Golden Binary Regeneration Tool")
    print("=" * 40)
    print(f"Project root: {project_root}")
    print(f"vasm-ext:     {args.vasm_root}")
    print(f"Platform:     {platform.system()} ({platform.machine()})")
    print()
    
    # Build vasm if needed
    builder = VasmBuilder(args.vasm_root, args.force)
    syntaxes = [args.syntax] if args.syntax else ["merlin", "scmasm"]
    
    for syntax in syntaxes:
        if builder.needs_build(syntax):
            print(f"Building vasm6502_{syntax}...")
            if not builder.build(syntax):
                print(f"ERROR: Failed to build vasm6502_{syntax}")
                return 1
    
    print()
    
    # Generate golden binaries
    generator = GoldenBinaryGenerator(project_root, args.vasm_root)
    if not generator.generate_all(args.syntax, args.cpu):
        return 1
    
    # Print summary
    print("=" * 40)
    print(f"Generated: {generator.stats['generated']}")
    print(f"Failed:    {generator.stats['failed']}")
    print(f"Skipped:   {generator.stats['skipped']}")
    print()
    
    if generator.stats["failed"] > 0:
        print("⚠️  Some golden binaries failed to generate")
        return 1
    
    if generator.stats["generated"] > 0:
        print("✅ Golden binaries generated successfully!")
        print()
        print("Next steps:")
        print("1. Review the generated binaries:")
        print(f"   cd {project_root / 'tests/integration/binary_compat/golden'}")
        print("2. Verify they match expected output")
        print("3. Commit the golden files to git:")
        print("   git add tests/integration/binary_compat/golden/")
        print("   git commit -m 'Update golden reference binaries'")
    else:
        print("No golden binaries were generated")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
