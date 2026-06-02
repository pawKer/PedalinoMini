#!/usr/bin/env python3
"""Tests for unofficial custom build identity surfaces."""

from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BUILD_NAME = "PedalinoMini 6 T-Display S3 Custom (Unofficial)"


class CustomBuildIdentityTest(unittest.TestCase):
    def read_text(self, relative_path: str) -> str:
        return (ROOT / relative_path).read_text(encoding="utf-8")

    def test_readme_starts_with_unofficial_disclaimer(self) -> None:
        readme = self.read_text("README.md")

        self.assertTrue(readme.startswith("# Unofficial Custom Build\n\n"))
        self.assertIn(BUILD_NAME, readme.splitlines()[2])
        self.assertIn("not official upstream PedalinoMini releases", readme[:400])

    def test_installer_displays_and_generates_custom_build_name(self) -> None:
        installer = self.read_text("index.html")

        self.assertIn("<title>Install {}</title>".format(BUILD_NAME), installer)
        self.assertIn("<h1>{} Installer</h1>".format(BUILD_NAME), installer)
        self.assertIn("This installer flashes unofficial custom fork builds.", installer)
        self.assertIn("const CUSTOM_BUILD_NAME = \"{}\";".format(BUILD_NAME), installer)
        self.assertIn("name: CUSTOM_BUILD_NAME", installer)

    def test_all_esp_web_tools_manifests_use_custom_build_name(self) -> None:
        manifest_paths = sorted((ROOT / "manifest").glob("*.json"))
        manifest_paths.extend(sorted((ROOT / "firmware").glob("*/*.json")))
        self.assertGreater(len(manifest_paths), 0)

        for path in manifest_paths:
            with self.subTest(path=path.relative_to(ROOT)):
                manifest = json.loads(path.read_text(encoding="utf-8"))
                self.assertEqual(BUILD_NAME, manifest["name"])

    def test_runtime_connectivity_identity_is_not_renamed(self) -> None:
        runtime_identity_files = [
            "src/Pedalino.h",
            "src/PedalinoMini.cpp",
            "src/ImprovSerial.cpp",
            "src/ImprovBLE.cpp",
            "src/WifiConnect.h",
        ]

        for relative_path in runtime_identity_files:
            with self.subTest(path=relative_path):
                self.assertNotIn(BUILD_NAME, self.read_text(relative_path))


if __name__ == "__main__":
    unittest.main()
