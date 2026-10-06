#!/usr/bin/env python3
"""Bounded release-contract regression. Payloads are nonexecutable fixtures."""
from __future__ import annotations
import hashlib
import importlib.util
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent
SPEC = importlib.util.spec_from_file_location(
    "release_integrity", ROOT / "scripts" / "verify-release-assets.py"
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
VERSION = "0.8.0-rc.9"

class ReleaseIntegrityTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="browser release integrity ")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.release = self.root / "release"
        self.release.mkdir()
        self.setup = f"Remote-Commander-Browser-Setup-v{VERSION}.exe"
        self.windows = f"remote-commander-browser-v{VERSION}-windows-x86_64.zip"
        self.linux = f"remote-commander-browser-v{VERSION}-linux-x86_64.tar.gz"
        self.pairs = {
            f"SHA256SUMS-setup-v{VERSION}.txt": self.setup,
            f"SHA256SUMS-windows-v{VERSION}.txt": self.windows,
            f"{self.linux}.sha256": self.linux,
        }
        for manifest, asset in self.pairs.items():
            data = b"NONEXECUTABLE_RELEASE_FIXTURE:" + asset.encode("ascii")
            (self.release / asset).write_bytes(data)
            (self.release / manifest).write_text(
                hashlib.sha256(data).hexdigest() + "  " + asset + "\n",
                encoding="ascii",
            )
        self.manifest = self.release / f"{self.linux}.sha256"

    def rejected(self):
        with self.assertRaises(MODULE.IntegrityError):
            MODULE.verify(VERSION, self.release)

    def test_exact_payloads_and_producer_hashes(self):
        self.assertEqual(len(MODULE.verify(VERSION, self.release)), 3)

    def test_tampered_payload_rejected(self):
        (self.release / self.setup).write_bytes(b"TAMPERED")
        self.rejected()

    def test_missing_payload_rejected(self):
        (self.release / self.windows).unlink()
        self.rejected()

    def test_extra_file_rejected(self):
        (self.release / "unexpected.txt").write_bytes(b"EXTRA")
        self.rejected()

    def test_absolute_producer_path_rejected(self):
        text = self.manifest.read_text(encoding="ascii")
        self.manifest.write_text(text.replace("  ", "  /home/runner/work/project/dist/"), encoding="ascii")
        self.rejected()

    def test_parent_path_rejected(self):
        text = self.manifest.read_text(encoding="ascii")
        self.manifest.write_text(text.replace("  ", "  ../"), encoding="ascii")
        self.rejected()

    def test_duplicate_checksum_record_rejected(self):
        text = self.manifest.read_text(encoding="ascii")
        self.manifest.write_text(text + text, encoding="ascii")
        self.rejected()

    def test_wrong_basename_rejected(self):
        text = self.manifest.read_text(encoding="ascii")
        self.manifest.write_text(text.replace(self.linux, "other.tar.gz"), encoding="ascii")
        self.rejected()

    def test_directory_as_payload_rejected(self):
        member = self.release / self.setup
        member.unlink()
        member.mkdir()
        self.rejected()

    def test_invalid_version_rejected(self):
        for value in ("../0.8", "v0.8.0", "0.8.0/../../x", "0.8.0;echo unsafe", ""):
            with self.subTest(value=value), self.assertRaises(MODULE.IntegrityError):
                MODULE.verify(value, self.release)

    def test_cli_returns_nonzero_for_bad_hash(self):
        (self.release / self.setup).write_bytes(b"TAMPERED")
        import sys
        result = subprocess.run(
            [sys.executable, str(ROOT / "scripts" / "verify-release-assets.py"), VERSION, str(self.release)],
            capture_output=True, text=True, timeout=15,
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("RELEASE_PRODUCER_SHA256_FAIL", result.stderr)

    @unittest.skipUnless(os.name == "posix", "native POSIX symlink semantics")
    def test_symlink_member_rejected(self):
        member = self.release / self.setup
        outside = self.root / "outside"
        outside.write_bytes(member.read_bytes())
        member.unlink()
        member.symlink_to(outside)
        self.rejected()

    def test_publication_policy_contract(self):
        workflow = (ROOT / ".github" / "workflows" / "release.yml").read_text(encoding="utf-8")
        self.assertIn("python3 scripts/verify-release-assets.py", workflow)
        self.assertLess(
            workflow.index("python3 scripts/verify-release-assets.py"),
            workflow.index(" > SHA256SUMS.txt"),
        )
        self.assertIn("release_flags=(--latest=false)", workflow)
        self.assertIn("release_flags+=(--prerelease)", workflow)
        self.assertIn('RELEASE_ALREADY_EXISTS_RECONCILE', workflow)
        self.assertIn('test "$immutable" = true', workflow)
        self.assertIn('needs: [windows, linux, release-contract]', workflow)
        stage = workflow.split("Package and stage exact Windows release assets", 1)[1].split(
            "      - uses:", 1
        )[0]
        self.assertNotIn("if($LASTEXITCODE -ne 0)", stage)

    @unittest.skipUnless(os.name == "posix", "native Linux package execution")
    def test_actual_packager_checksum_is_portable(self):
        fixture = self.root / "project with spaces"
        for directory in ("scripts", "build/bin", ".deps/cef"):
            (fixture / directory).mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / "scripts" / "package-release.sh", fixture / "scripts" / "package-release.sh")
        (fixture / "VERSION").write_text(VERSION + "\n", encoding="ascii")
        (fixture / "build/bin/chatgpt-cef-v2").write_text("#!/bin/sh\nexit 0\n", encoding="ascii")
        (fixture / "build/bin/chatgpt-cef-v2").chmod(0o755)
        for name in ("LICENSE.txt", "CREDITS.html"):
            (fixture / ".deps/cef" / name).write_text("FIXTURE ONLY\n", encoding="ascii")
        for name in ("NOTICE.md", "THIRD_PARTY_NOTICES.md", "RELEASE_NOTES.md"):
            (fixture / name).write_text("FIXTURE ONLY\n", encoding="ascii")
        result = subprocess.run(
            ["bash", str(fixture / "scripts/package-release.sh")],
            cwd=self.root, capture_output=True, text=True, timeout=30,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        sums = fixture / "dist" / f"remote-commander-browser-v{VERSION}-linux-x86_64.tar.gz.sha256"
        line = sums.read_text(encoding="ascii").strip()
        self.assertEqual(line.split("  ", 1)[1], f"remote-commander-browser-v{VERSION}-linux-x86_64.tar.gz")
        check = subprocess.run(
            ["sha256sum", "--check", sums.name], cwd=sums.parent,
            capture_output=True, text=True, timeout=10,
        )
        self.assertEqual(check.returncode, 0, check.stderr)

if __name__ == "__main__":
    unittest.main(verbosity=2)
