#!/usr/bin/env python3

from __future__ import annotations

import os
import xml.etree.ElementTree as ET
from pathlib import Path


def truncate(text: str, limit: int = 220) -> str:
    text = " ".join(text.split())
    if len(text) <= limit:
        return text
    return text[: limit - 3] + "..."


def main() -> None:
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary_path:
        return

    report_dir = Path(os.environ["COMPILER_REPORT_DIR"])
    junit_path = report_dir / "ctest-junit.xml"
    log_path = report_dir / "ctest.log"

    if not junit_path.exists():
        return

    root = ET.parse(junit_path).getroot()
    suites = [root] if root.tag == "testsuite" else list(root.findall("testsuite"))

    tests = 0
    failures = 0
    skipped = 0
    duration = 0.0
    failed_cases: list[tuple[str, str]] = []

    for suite in suites:
        tests += int(suite.attrib.get("tests", 0))
        failures += int(suite.attrib.get("failures", 0)) + int(suite.attrib.get("errors", 0))
        skipped += int(suite.attrib.get("skipped", 0))
        duration += float(suite.attrib.get("time", 0.0) or 0.0)

        for case in suite.findall("testcase"):
            failure = case.find("failure")
            error = case.find("error")
            if failure is None and error is None:
                continue
            detail_node = failure if failure is not None else error
            name = case.attrib.get("name", "<unnamed test>")
            detail = detail_node.attrib.get("message", "") or detail_node.text or "See ctest log for details."
            failed_cases.append((name, truncate(detail)))

    passed = tests - failures - skipped

    with open(summary_path, "a", encoding="utf-8") as summary:
        summary.write("## Unit test report\n\n")
        summary.write(f"- Total: `{tests}`\n")
        summary.write(f"- Passed: `{passed}`\n")
        summary.write(f"- Failed: `{failures}`\n")
        summary.write(f"- Skipped: `{skipped}`\n")
        summary.write(f"- Duration: `{duration:.2f}s`\n")
        summary.write(f"- JUnit report: `{junit_path}`\n")
        summary.write(f"- Test log: `{log_path}`\n\n")

        if failed_cases:
            summary.write("| Failing test | Details |\n")
            summary.write("| --- | --- |\n")
            for name, detail in failed_cases[:20]:
                escaped_detail = detail.replace("|", "\\|")
                summary.write(f"| `{name}` | {escaped_detail} |\n")


if __name__ == "__main__":
    main()
