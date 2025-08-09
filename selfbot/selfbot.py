#!/usr/bin/env python3

import argparse
import json
import os
import sys
import subprocess
import time
import shutil
import tempfile
import urllib.request
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

# -------------------------------
# Utility and Exceptions
# -------------------------------

class BotError(Exception):
    pass


def eprint(*args: Any, **kwargs: Any) -> None:
    print(*args, file=sys.stderr, **kwargs)


# -------------------------------
# Task and Context Handling
# -------------------------------

class ExecutionContext:
    def __init__(self, variables: Optional[Dict[str, Any]] = None, verbose: bool = False) -> None:
        self.variables: Dict[str, Any] = variables.copy() if variables else {}
        self.verbose: bool = verbose

    def get(self, key: str, default: Any = None) -> Any:
        return self.variables.get(key, default)

    def set(self, key: str, value: Any) -> None:
        if self.verbose:
            eprint(f"[ctx] set {key}={value!r}")
        self.variables[key] = value

    def format_text(self, text: str) -> str:
        try:
            return text.format(**self.variables)
        except KeyError as exc:
            raise BotError(f"Missing variable for formatting: {exc}") from exc


# -------------------------------
# Actions Implementation
# -------------------------------

def action_print(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    message = str(params.get("message", ""))
    print(ctx.format_text(message))


def action_set(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    for key, value in params.items():
        if isinstance(value, str):
            value = ctx.format_text(value)
        ctx.set(key, value)


def action_write_file(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    path = ctx.format_text(str(params["path"]))
    content = params.get("content", "")
    if isinstance(content, str):
        content = ctx.format_text(content)
    mkdirs = bool(params.get("mkdirs", True))
    mode = params.get("mode", "w")
    encoding = params.get("encoding", "utf-8")
    p = Path(path)
    if mkdirs:
        p.parent.mkdir(parents=True, exist_ok=True)
    if ctx.verbose:
        eprint(f"[fs] write {p} (mode={mode})")
    with open(p, mode, encoding=encoding if "b" not in mode else None) as f:
        if "b" in mode and isinstance(content, str):
            raise BotError("Binary write mode requires bytes content")
        f.write(content)


def action_read_file(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    path = ctx.format_text(str(params["path"]))
    key = params.get("to", "file_content")
    encoding = params.get("encoding", "utf-8")
    p = Path(path)
    if not p.exists():
        raise BotError(f"File not found: {p}")
    if ctx.verbose:
        eprint(f"[fs] read {p}")
    with open(p, "r", encoding=encoding) as f:
        content = f.read()
    ctx.set(key, content)


def action_run(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    cmd_raw = params["cmd"]
    timeout = float(params.get("timeout", 120))
    capture_key = params.get("to", None)
    cwd = params.get("cwd")
    env_updates = params.get("env", {}) or {}

    # Allow cmd to be list or string; if string, run via shell=False split naive
    if isinstance(cmd_raw, str):
        # naive split; avoid shell injection by not using shell=True
        cmd = cmd_raw.split()
    elif isinstance(cmd_raw, list):
        cmd = [ctx.format_text(str(part)) for part in cmd_raw]
    else:
        raise BotError("cmd must be a string or list")

    cmd = [ctx.format_text(str(c)) for c in cmd]
    run_cwd = ctx.format_text(cwd) if cwd else None

    env = os.environ.copy()
    for k, v in env_updates.items():
        env[str(k)] = ctx.format_text(str(v))

    if ctx.verbose:
        eprint(f"[run] {' '.join(cmd)} (cwd={run_cwd})")
    try:
        result = subprocess.run(cmd, cwd=run_cwd, env=env, capture_output=True, timeout=timeout, text=True, check=False)
    except FileNotFoundError as exc:
        raise BotError(f"Command not found: {cmd[0]}") from exc
    except subprocess.TimeoutExpired as exc:
        raise BotError(f"Command timed out after {timeout}s: {' '.join(cmd)}") from exc

    if capture_key:
        ctx.set(capture_key, result.stdout)

    if result.returncode != 0:
        raise BotError(f"Command failed ({result.returncode}): {' '.join(cmd)}\nSTDOUT:\n{result.stdout}\nSTDERR:\n{result.stderr}")


def action_http_get(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    url = ctx.format_text(str(params["url"]))
    to = params.get("to", "http_response")
    timeout = float(params.get("timeout", 30))
    if ctx.verbose:
        eprint(f"[http] GET {url}")
    req = urllib.request.Request(url, headers={"User-Agent": "selfbot/1.0"})
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        charset = resp.headers.get_content_charset() or "utf-8"
        body = resp.read().decode(charset, errors="replace")
    ctx.set(to, body)


def action_sleep(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    seconds = float(params.get("seconds", 1))
    if ctx.verbose:
        eprint(f"[sleep] {seconds}s")
    time.sleep(seconds)


def action_copy(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    src = Path(ctx.format_text(str(params["src"])) )
    dst = Path(ctx.format_text(str(params["dst"])) )
    if src.is_dir():
        if dst.exists():
            shutil.rmtree(dst)
        shutil.copytree(src, dst)
    else:
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
    if ctx.verbose:
        eprint(f"[fs] copy {src} -> {dst}")


def action_call_self(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    """Spawn this program to run a subtask file or inline steps."""
    subtask_file = params.get("task_file")
    inline_steps = params.get("steps")
    inherit_vars = bool(params.get("inherit_vars", True))
    extra_vars: Dict[str, Any] = params.get("vars", {}) or {}

    if inline_steps and subtask_file:
        raise BotError("Provide either 'steps' or 'task_file', not both")

    # Prepare temp file if inline steps provided
    if inline_steps:
        tmp_dir = tempfile.mkdtemp(prefix="selfbot_")
        subtask_file = os.path.join(tmp_dir, "inline_task.json")
        json_payload = {"name": "inline", "steps": inline_steps}
        with open(subtask_file, "w", encoding="utf-8") as f:
            json.dump(json_payload, f, indent=2)
    elif not subtask_file:
        raise BotError("call_self requires 'steps' or 'task_file'")

    python_exe = sys.executable

    vars_args: List[str] = []
    combined_vars = {}
    if inherit_vars:
        combined_vars.update(ctx.variables)
    combined_vars.update({str(k): v for (k, v) in extra_vars.items()})

    for k, v in combined_vars.items():
        # Serialize primitives and strings; for others, dump json
        if isinstance(v, (str, int, float, bool)) or v is None:
            vars_args.append(f"--var={k}={v}")
        else:
            vars_args.append(f"--varjson={k}={json.dumps(v)}")

    cmd = [python_exe, os.path.abspath(__file__), "run", "--task-file", subtask_file] + vars_args

    if ctx.verbose:
        eprint(f"[self] spawn: {' '.join(cmd)}")

    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        raise BotError(f"Subtask failed: {result.stderr}\nOutput:\n{result.stdout}")


def action_zip_project(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    project_dir = Path(ctx.format_text(str(params["project_dir"])) )
    zip_path = Path(ctx.format_text(str(params["zip_path"])) )
    zip_path.parent.mkdir(parents=True, exist_ok=True)
    base_name = str(zip_path.with_suffix(""))
    if ctx.verbose:
        eprint(f"[zip] {project_dir} -> {zip_path}")
    shutil.make_archive(base_name, "zip", root_dir=str(project_dir), base_dir=".")
    to = params.get("to")
    if to:
        ctx.set(to, str(zip_path))


def _validate_identifier(name: str) -> None:
    import re
    if not re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", name):
        raise BotError(f"Invalid C++ identifier: {name}")


def action_cpp_scaffold(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    project_dir = Path(ctx.format_text(str(params["project_dir"])) )
    name = ctx.format_text(str(params.get("name", "app")))
    (project_dir / "src").mkdir(parents=True, exist_ok=True)
    (project_dir / "include").mkdir(parents=True, exist_ok=True)
    (project_dir / "build").mkdir(parents=True, exist_ok=True)
    (project_dir / "bin").mkdir(parents=True, exist_ok=True)

    gitignore = """
/build/
/bin/
/*.zip
*.o
*.obj
*.exe
*.out
.DS_Store
    """.strip() + "\n"
    readme = f"# {name}\n\nGenerated by selfbot.\n"

    with open(project_dir / ".gitignore", "w", encoding="utf-8") as f:
        f.write(gitignore)
    with open(project_dir / "README.md", "w", encoding="utf-8") as f:
        f.write(readme)

    if ctx.verbose:
        eprint(f"[cpp] scaffold at {project_dir}")


def action_cpp_add_class(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    project_dir = Path(ctx.format_text(str(params["project_dir"])) )
    class_name = ctx.format_text(str(params["class_name"]))
    namespace = params.get("namespace")
    if namespace:
        namespace = ctx.format_text(str(namespace))
    _validate_identifier(class_name)

    include_dir = project_dir / "include"
    src_dir = project_dir / "src"
    include_dir.mkdir(parents=True, exist_ok=True)
    src_dir.mkdir(parents=True, exist_ok=True)

    header_path = include_dir / f"{class_name}.hpp"
    source_path = src_dir / f"{class_name}.cpp"

    ns_open = f"namespace {namespace} {{\n" if namespace else ""
    ns_close = "}\n" if namespace else ""
    qualified = f"{namespace}::{class_name}" if namespace else class_name

    header = f"""#pragma once
#include <string>

{ns_open}class {class_name} {{
public:
    {class_name}();
    ~{class_name}();
    std::string greet(const std::string& name) const;
}};
{ns_close}"""

    source = f"""#include "{class_name}.hpp"

{ns_open}{qualified}::{class_name}() = default;
{qualified}::~{class_name}() = default;

std::string {qualified}::greet(const std::string& name) const {{
    return std::string("Hello, ") + name + "!";
}}
{ns_close}"""

    with open(header_path, "w", encoding="utf-8") as f:
        f.write(header)
    with open(source_path, "w", encoding="utf-8") as f:
        f.write(source)

    if ctx.verbose:
        eprint(f"[cpp] class {class_name} added (namespace={namespace or 'none'})")


def action_cpp_write_main(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    project_dir = Path(ctx.format_text(str(params["project_dir"])) )
    main_path = project_dir / "src" / "main.cpp"
    use_class = params.get("use_class")
    namespace = params.get("namespace")
    if use_class:
        use_class = ctx.format_text(str(use_class))
    if namespace:
        namespace = ctx.format_text(str(namespace))

    if "code" in params:
        code = ctx.format_text(str(params["code"]))
    else:
        if use_class:
            qualified = f"{namespace}::{use_class}" if namespace else use_class
            include_line = f"#include \"{use_class}.hpp\"\n"
            body = (
                "int main() {\n"
                f"    {qualified} greeter;\n"
                "    std::cout << greeter.greet(\"World\") << std::endl;\n"
                "    return 0;\n"
                "}\n"
            )
            code = "#include <iostream>\n" + include_line + "\n" + body
        else:
            code = (
                "#include <iostream>\n\n"
                "int main() {\n"
                "    std::cout << \"Hello from main!\" << std::endl;\n"
                "    return 0;\n"
                "}\n"
            )

    main_path.parent.mkdir(parents=True, exist_ok=True)
    with open(main_path, "w", encoding="utf-8") as f:
        f.write(code)
    if ctx.verbose:
        eprint(f"[cpp] wrote main at {main_path}")


def action_cpp_compile(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    project_dir = Path(ctx.format_text(str(params["project_dir"])) )
    output_name = ctx.format_text(str(params.get("output", "app")))
    std = ctx.format_text(str(params.get("std", "c++17")))
    extra_flags = params.get("flags", [])
    if isinstance(extra_flags, str):
        extra_flags = extra_flags.split()

    include_dir = project_dir / "include"
    src_dir = project_dir / "src"
    build_dir = project_dir / "build"
    bin_dir = project_dir / "bin"
    build_dir.mkdir(parents=True, exist_ok=True)
    bin_dir.mkdir(parents=True, exist_ok=True)

    cpp_files = sorted(str(p) for p in src_dir.glob("*.cpp"))
    if not cpp_files:
        raise BotError(f"No .cpp files found in {src_dir}")

    output_path = build_dir / output_name
    cmd = [
        "g++", f"-std={std}", "-O2", "-Wall", "-Wextra",
        "-I", str(include_dir),
        *extra_flags,
        *cpp_files,
        "-o", str(output_path),
    ]

    action_run(ctx, {"cmd": cmd, "cwd": str(project_dir)})

    # Also copy to bin for convenience
    shutil.copy2(output_path, bin_dir / output_name)

    to = params.get("to")
    if to:
        ctx.set(to, str(output_path))
    if ctx.verbose:
        eprint(f"[cpp] compiled -> {output_path}")


def action_cpp_run_binary(ctx: ExecutionContext, params: Dict[str, Any]) -> None:
    project_dir = Path(ctx.format_text(str(params["project_dir"])) )
    binary_name = ctx.format_text(str(params.get("binary_name", "app")))
    args = params.get("args", [])
    if isinstance(args, str):
        args = args.split()

    binary_path = project_dir / "build" / binary_name
    if not binary_path.exists():
        raise BotError(f"Binary not found: {binary_path}")

    cmd = [str(binary_path), *[ctx.format_text(str(a)) for a in args]]
    action_run(ctx, {"cmd": cmd, "cwd": str(project_dir), "to": params.get("to")})


# -------------------------------
# Control Flow Helpers
# -------------------------------

def evaluate_condition(ctx: ExecutionContext, condition: Dict[str, Any]) -> bool:
    """Simple condition evaluator: supports equals, not_equals, exists, contains."""
    if "equals" in condition:
        left = condition["equals"][0]
        right = condition["equals"][1]
        left_v = ctx.variables.get(left, left)
        right_v = ctx.variables.get(right, right)
        return left_v == right_v
    if "not_equals" in condition:
        left = condition["not_equals"][0]
        right = condition["not_equals"][1]
        left_v = ctx.variables.get(left, left)
        right_v = ctx.variables.get(right, right)
        return left_v != right_v
    if "exists" in condition:
        key = condition["exists"]
        return key in ctx.variables
    if "contains" in condition:
        container = condition["contains"][0]
        item = condition["contains"][1]
        container_v = ctx.variables.get(container, container)
        item_v = ctx.variables.get(item, item)
        try:
            return item_v in container_v
        except Exception:
            return False
    raise BotError(f"Unsupported condition: {condition}")


def execute_steps(ctx: ExecutionContext, steps: List[Dict[str, Any]]) -> None:
    actions = {
        "print": action_print,
        "set": action_set,
        "write_file": action_write_file,
        "read_file": action_read_file,
        "run": action_run,
        "http_get": action_http_get,
        "sleep": action_sleep,
        "copy": action_copy,
        "call_self": action_call_self,
        # C++ project helpers
        "zip_project": action_zip_project,
        "cpp_scaffold": action_cpp_scaffold,
        "cpp_add_class": action_cpp_add_class,
        "cpp_write_main": action_cpp_write_main,
        "cpp_compile": action_cpp_compile,
        "cpp_run_binary": action_cpp_run_binary,
    }

    for idx, step in enumerate(steps):
        if not isinstance(step, dict):
            raise BotError(f"Invalid step at index {idx}: must be object")
        if "action" in step:
            action_name = step["action"]
            params = step.get("params", {})
            if ctx.verbose:
                eprint(f"[step {idx+1}/{len(steps)}] {action_name}")
            handler = actions.get(action_name)
            if not handler:
                raise BotError(f"Unknown action: {action_name}")
            handler(ctx, params)
        elif "if" in step:
            condition = step["if"]
            then_steps = step.get("then", [])
            else_steps = step.get("else", [])
            if evaluate_condition(ctx, condition):
                if ctx.verbose:
                    eprint(f"[if] condition true -> then")
                execute_steps(ctx, then_steps)
            else:
                if ctx.verbose:
                    eprint(f"[if] condition false -> else")
                execute_steps(ctx, else_steps)
        elif "for_each" in step:
            iterable_ref = step["for_each"]
            as_var = step.get("as", "item")
            body_steps = step.get("do", [])
            iterable = ctx.variables.get(iterable_ref, step.get("list", []))
            if not isinstance(iterable, list):
                raise BotError("for_each expects a list in context or 'list'")
            for item in iterable:
                ctx.set(as_var, item)
                execute_steps(ctx, body_steps)
        else:
            raise BotError(f"Invalid step at index {idx}: missing 'action', 'if', or 'for_each'")


# -------------------------------
# Queue Processing (Self-sustained loop)
# -------------------------------

def process_task_file(ctx: ExecutionContext, task_file: Path) -> None:
    with open(task_file, "r", encoding="utf-8") as f:
        task = json.load(f)
    name = task.get("name", task_file.stem)
    steps = task.get("steps", [])
    if ctx.verbose:
        eprint(f"[task] start {name} from {task_file}")
    execute_steps(ctx, steps)
    if ctx.verbose:
        eprint(f"[task] done {name}")


def run_queue(ctx: ExecutionContext, queue_dir: Path, poll_interval: float = 1.0) -> None:
    eprint(f"[queue] watching {queue_dir} (poll={poll_interval}s)")
    queue_dir.mkdir(parents=True, exist_ok=True)
    processing_dir = queue_dir / "_processing"
    done_dir = queue_dir / "_done"
    failed_dir = queue_dir / "_failed"
    for d in (processing_dir, done_dir, failed_dir):
        d.mkdir(parents=True, exist_ok=True)

    while True:
        for task_file in sorted(queue_dir.glob("*.json")):
            target = processing_dir / task_file.name
            try:
                task_file.rename(target)
            except Exception:
                # May be picked up concurrently; skip
                continue
            try:
                process_task_file(ctx, target)
            except Exception as exc:
                eprint(f"[queue] task {target.name} failed: {exc}")
                shutil.move(str(target), str(failed_dir / task_file.name))
            else:
                shutil.move(str(target), str(done_dir / task_file.name))
        time.sleep(poll_interval)


# -------------------------------
# CLI
# -------------------------------

def parse_kv(argument: str) -> Tuple[str, Any]:
    # format KEY=VALUE; value kept as string
    if "=" not in argument:
        raise argparse.ArgumentTypeError("--var must be KEY=VALUE")
    key, value = argument.split("=", 1)
    return key, value


def parse_kv_json(argument: str) -> Tuple[str, Any]:
    if "=" not in argument:
        raise argparse.ArgumentTypeError("--varjson must be KEY=JSON_VALUE")
    key, value = argument.split("=", 1)
    try:
        return key, json.loads(value)
    except json.JSONDecodeError as exc:
        raise argparse.ArgumentTypeError(f"Invalid JSON for --varjson: {exc}") from exc


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Self-sustained non-AI automation bot")
    sub = parser.add_subparsers(dest="command", required=True)

    # run a single task file
    p_run = sub.add_parser("run", help="Run a JSON task file")
    p_run.add_argument("--task-file", required=True, help="Path to JSON task file")
    p_run.add_argument("--var", action="append", default=[], type=parse_kv, help="Set variable KEY=VALUE")
    p_run.add_argument("--varjson", action="append", default=[], type=parse_kv_json, help="Set variable KEY=JSON_VALUE")
    p_run.add_argument("--verbose", action="store_true", help="Verbose logs")

    # watch a queue directory
    p_watch = sub.add_parser("watch", help="Watch a queue directory for tasks")
    p_watch.add_argument("--queue-dir", default=str(Path(__file__).parent / "queue"), help="Directory containing task JSON files")
    p_watch.add_argument("--poll", type=float, default=1.0, help="Polling interval seconds")
    p_watch.add_argument("--var", action="append", default=[], type=parse_kv, help="Set initial variable KEY=VALUE")
    p_watch.add_argument("--varjson", action="append", default=[], type=parse_kv_json, help="Set initial variable KEY=JSON_VALUE")
    p_watch.add_argument("--verbose", action="store_true", help="Verbose logs")

    return parser


def main(argv: Optional[List[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    variables: Dict[str, Any] = {}
    for k, v in getattr(args, "var", []) or []:
        variables[k] = v
    for k, v in getattr(args, "varjson", []) or []:
        variables[k] = v

    ctx = ExecutionContext(variables=variables, verbose=bool(getattr(args, "verbose", False)))

    try:
        if args.command == "run":
            task_path = Path(args.task_file)
            process_task_file(ctx, task_path)
            return 0
        if args.command == "watch":
            run_queue(ctx, Path(args.queue_dir), poll_interval=float(args.poll))
            return 0
        raise BotError(f"Unknown command: {args.command}")
    except BotError as exc:
        eprint(f"Error: {exc}")
        return 2


if __name__ == "__main__":
    sys.exit(main())