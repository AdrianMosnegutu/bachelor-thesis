#!/usr/bin/env python3

from __future__ import annotations

import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path


ERROR_PATTERN = re.compile(r"^(?P<file>.+?\.(?:tex|sty|cls|bib)):(?P<line>\d+): (?P<message>.+)$")
CITATION_PATTERN = re.compile(
    r"(?:LaTeX|Package [^ ]+) Warning: Citation [`'](?P<key>[^']+)'(?: on page \d+)? undefined on input line (?P<line>\d+)\."
)
REFERENCE_PATTERN = re.compile(
    r"(?:LaTeX|Package [^ ]+) Warning: Reference [`'](?P<key>[^']+)'(?: on page \d+)? undefined on input line (?P<line>\d+)\."
)
MULTIPLY_DEFINED_LABEL_PATTERN = re.compile(
    r"(?:LaTeX|Package [^ ]+) Warning: Label [`'](?P<key>[^']+)' multiply defined\."
)


@dataclass(frozen=True)
class Finding:
    file_path: str
    line_no: int
    description: str
    severity: str


def normalize_repo_path(paper_dir: Path, candidate: Path | str) -> str:
    candidate_path = Path(candidate)
    if candidate_path.is_absolute():
        try:
            return candidate_path.relative_to(Path.cwd()).as_posix()
        except ValueError:
            return candidate_path.as_posix()

    normalized = candidate_path.as_posix().lstrip("./")
    if normalized.startswith(f"{paper_dir.as_posix()}/"):
        return normalized
    return f"{paper_dir.as_posix()}/{normalized}"


def load_source_files(paper_dir: Path) -> list[tuple[Path, list[str]]]:
    return [(path, path.read_text(encoding="utf-8").splitlines()) for path in sorted(paper_dir.rglob("*.tex"))]


def find_command_occurrences(
    sources: list[tuple[Path, list[str]]],
    line_hint: int | None,
    pattern: re.Pattern[str],
) -> list[tuple[Path, int]]:
    matches: list[tuple[Path, int]] = []

    if line_hint is not None:
        for file_path, lines in sources:
            if 1 <= line_hint <= len(lines) and pattern.search(lines[line_hint - 1]):
                return [(file_path, line_hint)]

    for file_path, lines in sources:
        for idx, line in enumerate(lines, start=1):
            if pattern.search(line):
                matches.append((file_path, idx))

    return matches


def build_findings(
    paper_dir: Path,
    root_file: Path,
    error_text: str,
    warning_text: str,
) -> list[Finding]:
    findings: list[Finding] = []
    sources = load_source_files(paper_dir)

    for raw_line in error_text.splitlines():
        error_match = ERROR_PATTERN.match(raw_line.strip())
        if not error_match:
            continue
        findings.append(
            Finding(
                file_path=normalize_repo_path(paper_dir, error_match.group("file")),
                line_no=int(error_match.group("line")),
                description=error_match.group("message").strip(),
                severity="error",
            )
        )

    for match in CITATION_PATTERN.finditer(warning_text):
        key = re.escape(match.group("key"))
        line_hint = int(match.group("line"))
        command_pattern = re.compile(rf"""\\[A-Za-z]*cite[A-Za-z*]*\s*(?:\[[^\]]*\]\s*){{0,2}}\{{[^}}]*\b{key}\b[^}}]*\}}""")
        locations = find_command_occurrences(sources, line_hint, command_pattern)
        if not locations:
            locations = [(paper_dir / root_file, line_hint)]
        for file_path, line_no in locations:
            findings.append(
                Finding(
                    file_path=normalize_repo_path(paper_dir, file_path),
                    line_no=line_no,
                    description=f"Undefined citation key `{match.group('key')}`.",
                    severity="error",
                )
            )

    for match in REFERENCE_PATTERN.finditer(warning_text):
        key = re.escape(match.group("key"))
        line_hint = int(match.group("line"))
        command_pattern = re.compile(rf"""\\(?:[A-Za-z]*ref|autoref|cref|Cref|eqref)[A-Za-z*]*\s*(?:\[[^\]]*\]\s*)?\{{[^}}]*\b{key}\b[^}}]*\}}""")
        locations = find_command_occurrences(sources, line_hint, command_pattern)
        if not locations:
            locations = [(paper_dir / root_file, line_hint)]
        for file_path, line_no in locations:
            findings.append(
                Finding(
                    file_path=normalize_repo_path(paper_dir, file_path),
                    line_no=line_no,
                    description=f"Undefined reference key `{match.group('key')}`.",
                    severity="error",
                )
            )

    for match in MULTIPLY_DEFINED_LABEL_PATTERN.finditer(warning_text):
        key = re.escape(match.group("key"))
        command_pattern = re.compile(rf"""\\label\{{{key}\}}""")
        locations = find_command_occurrences(sources, None, command_pattern)
        if not locations:
            locations = [(paper_dir / root_file, 1)]
        for file_path, line_no in locations:
            findings.append(
                Finding(
                    file_path=normalize_repo_path(paper_dir, file_path),
                    line_no=line_no,
                    description=f"Multiply defined label `{match.group('key')}`.",
                    severity="error",
                )
            )

    deduped: list[Finding] = []
    seen: set[tuple[str, int, str, str]] = set()
    for finding in findings:
        key = (finding.file_path, finding.line_no, finding.description, finding.severity)
        if key in seen:
            continue
        seen.add(key)
        deduped.append(finding)

    return deduped


def main() -> int:
    paper_dir = Path(os.environ["PAPER_DIR"])
    root_file = Path(os.environ["PAPER_ROOT_FILE"])
    report_repo = os.environ["REPORT_REPO"]
    report_sha = os.environ["REPORT_SHA"]
    summary_path = Path(os.environ["GITHUB_STEP_SUMMARY"])

    build_dir = paper_dir / "out" / "build" / "ci"
    output_log = build_dir / "latexmk-output.log"
    exit_code_file = build_dir / "latexmk-exit-code.txt"
    tex_log = paper_dir / f"{root_file.stem}.log"

    build_rc = int(exit_code_file.read_text(encoding="utf-8").strip()) if exit_code_file.exists() else 1

    report_parts: list[str] = []
    if output_log.exists():
        report_parts.append(output_log.read_text(encoding="utf-8", errors="replace"))
    if tex_log.exists():
        report_parts.append(tex_log.read_text(encoding="utf-8", errors="replace"))
    error_text = "\n".join(report_parts)
    warning_text = tex_log.read_text(encoding="utf-8", errors="replace") if tex_log.exists() else ""

    findings = build_findings(paper_dir, root_file, error_text, warning_text)

    if build_rc != 0 and not findings:
        findings.append(
            Finding(
                file_path=normalize_repo_path(paper_dir, paper_dir / root_file),
                line_no=1,
                description="LaTeX build failed. Inspect the workflow log for the full compiler output.",
                severity="error",
            )
        )

    if not findings:
        return 0

    repo_url = f"{os.environ.get('GITHUB_SERVER_URL', 'https://github.com')}/{report_repo}/blob/{report_sha}"
    with summary_path.open("a", encoding="utf-8") as summary:
        summary.write(f"## Build issues found: {len(findings)}\n\n")
        summary.write("| File | Description | Severity |\n")
        summary.write("| --- | --- | --- |\n")
        for finding in findings:
            link = f"{repo_url}/{finding.file_path}#L{finding.line_no}"
            description = finding.description.replace("|", "\\|")
            summary.write(
                f"| [{finding.file_path}:{finding.line_no}]({link}) | {description} | {finding.severity} |\n"
            )

    return 1


if __name__ == "__main__":
    sys.exit(main())
