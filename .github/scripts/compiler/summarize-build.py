#!/usr/bin/env python3

from __future__ import annotations

import os
import re
from pathlib import Path


def read_text(path: Path) -> str:
    if not path.exists():
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def build_repo_prefix() -> str | None:
    repo = os.environ.get("REPORT_REPO")
    sha = os.environ.get("REPORT_SHA")
    server = os.environ.get("GITHUB_SERVER_URL", "https://github.com")
    if not repo or not sha:
        return None
    return f"{server}/{repo}/blob/{sha}"


def make_link(file_path: str, line_no: str | None, repo_prefix: str | None) -> str:
    if not repo_prefix:
        return file_path if not line_no else f"{file_path}:{line_no}"
    suffix = f"#L{line_no}" if line_no else ""
    label = file_path if not line_no else f"{file_path}:{line_no}"
    return f"[{label}]({repo_prefix}/{file_path}{suffix})"


def normalize_path(file_path: str) -> str:
    workspace = os.environ.get("GITHUB_WORKSPACE") or str(Path.cwd().resolve())
    normalized = file_path.strip()
    if workspace and normalized.startswith(workspace.rstrip("/") + "/"):
        return normalized[len(workspace.rstrip("/") + "/") :]
    return normalized


def collect_findings(log_text: str) -> list[tuple[str, str | None, str]]:
    findings: list[tuple[str, str | None, str]] = []
    pattern = re.compile(
        r"(?P<file>(?:[A-Za-z]:)?[^:\n]+?\.(?:c|cc|cpp|cxx|h|hh|hpp)):(?P<line>\d+)(?::\d+)?:\s*(?:fatal\s+)?error:\s*(?P<msg>.+)"
    )

    for match in pattern.finditer(log_text):
        file_path = normalize_path(match.group("file"))
        if "/_deps/" in file_path or "/googletest/" in file_path:
            continue
        findings.append((file_path, match.group("line"), match.group("msg").strip()))
        if len(findings) == 10:
            return findings

    if findings:
        return findings

    for line in log_text.splitlines():
        if "error:" not in line.lower():
            continue
        findings.append((line.strip(), None, ""))
        if len(findings) == 10:
            break

    return findings


def main() -> None:
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary_path:
        return

    report_dir = Path(os.environ["COMPILER_REPORT_DIR"])
    configure_log = report_dir / "configure.log"
    build_log = report_dir / "build.log"
    phase_file = report_dir / "build-phase.txt"

    phase = read_text(phase_file).strip() or "unknown"
    configure_text = read_text(configure_log)
    build_text = read_text(build_log)

    log_text = configure_text if phase == "configure" else build_text
    findings = collect_findings(log_text)
    repo_prefix = build_repo_prefix()

    with open(summary_path, "a", encoding="utf-8") as summary:
        summary.write("## Build failed\n\n")
        summary.write(f"- Failing phase: `{phase}`\n")
        summary.write(f"- Configure log: `{configure_log}`\n")
        summary.write(f"- Build log: `{build_log}`\n\n")

        if findings:
            summary.write("| Location | Details |\n")
            summary.write("| --- | --- |\n")
            for file_path, line_no, message in findings:
                location = make_link(file_path, line_no, repo_prefix)
                details = message.replace("|", "\\|") if message else "See job logs for details."
                summary.write(f"| {location} | {details} |\n")
        else:
            summary.write("No file-level diagnostics were parsed from the logs. See the job log excerpt below.\n\n")

        excerpt = "\n".join(log_text.splitlines()[-25:]).strip()
        if excerpt:
            summary.write("\n<details>\n")
            summary.write("<summary>Recent build log output</summary>\n\n")
            summary.write("```text\n")
            summary.write(excerpt[:12000])
            summary.write("\n```\n")
            summary.write("</details>\n")


if __name__ == "__main__":
    main()
