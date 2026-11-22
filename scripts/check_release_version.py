#!/usr/bin/env python3
"""Reject a release tag unless all packaged versions match it."""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main(tag):
    if not re.fullmatch(r"v(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)", tag):
        raise SystemExit(f"Invalid release tag: {tag!r}; expected vMAJOR.MINOR.PATCH")
    version = tag[1:]
    checks = {
        "python/pyproject.toml": r'^version = "([^"]+)"',
        "python/src/arborikit/__init__.py": r'^__version__ = "([^"]+)"',
        "java/pom.xml": r'<artifactId>arborikit</artifactId>\s*<version>([^<]+)</version>',
        "c/CMakeLists.txt": r'\bVERSION\s+(\d+\.\d+\.\d+)',
        "cpp/CMakeLists.txt": r'\bVERSION\s+(\d+\.\d+\.\d+)',
        "c/include/arborikit/version.h": r'#define ARBORIKIT_VERSION_STRING "([^"]+)"',
        "c/conanfile.py": r'^    version = "([^"]+)"',
        "cpp/conanfile.py": r'^    version = "([^"]+)"',
        "c/vcpkg.json": r'"version":\s*"([^"]+)"',
        "cpp/vcpkg.json": r'"version":\s*"([^"]+)"',
    }
    errors = []
    for filename, pattern in checks.items():
        match = re.search(pattern, (ROOT / filename).read_text(), re.M)
        if not match or match.group(1) != version:
            errors.append(f"{filename}: {match.group(1) if match else 'version not found'}")
    header = (ROOT / "c/include/arborikit/version.h").read_text()
    for name, expected in zip(("MAJOR", "MINOR", "PATCH"), version.split(".")):
        match = re.search(rf'^#define ARBORIKIT_VERSION_{name} (\d+)$', header, re.M)
        if not match or match.group(1) != expected:
            errors.append(f"c/include/arborikit/version.h: {name} mismatch")
    if errors:
        raise SystemExit(f"Versions do not match {tag}:\n" + "\n".join(errors))
    print(f"All package versions match {tag}.")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: check_release_version.py vMAJOR.MINOR.PATCH")
    main(sys.argv[1])
