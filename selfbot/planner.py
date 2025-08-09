from dataclasses import dataclass
from typing import Dict, List, Any


@dataclass
class Step:
    action: str
    params: Dict[str, Any]


class Planner:
    """Converts simple textual goals into deterministic plans using rules."""

    @staticmethod
    def plan(goal: str) -> List[Step]:
        g = goal.strip().lower()

        # Rule set: specific patterns first
        if g.startswith("create file "):
            # Format: create file <path> with <content>
            return Planner._plan_create_file(g)

        if g.startswith("append to "):
            # Format: append to <path> with <content>
            return Planner._plan_append_file(g)

        if g.startswith("list dir ") or g.startswith("list directory "):
            path = goal.split(" ", 2)[2].strip()
            return [Step("list_dir", {"path": path})]

        if g.startswith("run "):
            # e.g., run echo hello
            command = goal[len("run "):].strip().split()
            return [Step("run", {"command": command})]

        if g.startswith("download ") and " to " in g:
            # download <url> to <path>
            return Planner._plan_download(g, goal)

        if g.startswith("copy ") and " to " in g:
            # copy <src> to <dst>
            parts = goal.split(" to ", 2)
            return [Step("copy", {"src": parts[0][len("copy "):].strip(), "dst": parts[1].strip(), "overwrite": True})]

        if g.startswith("move ") and " to " in g:
            parts = goal.split(" to ", 2)
            return [Step("move", {"src": parts[0][len("move "):].strip(), "dst": parts[1].strip(), "overwrite": True})]

        # Default fallback plan: just echo the goal into a log
        return [
            Step("write_file", {"path": "./selfbot_output/last_goal.txt", "content": goal, "overwrite": True}),
            Step("run", {"command": ["sh", "-lc", "echo 'Goal captured'" ]}),
        ]

    @staticmethod
    def _plan_create_file(g: str) -> List[Step]:
        # "create file <path> with <content>"
        if " with " in g:
            before, after = g.split(" with ", 1)
            # Recover original (case-sensitive) from goal is not needed for file path
            path = before[len("create file "):].strip()
            content = after
            return [Step("write_file", {"path": path, "content": content, "overwrite": True})]
        # "create file <path>"
        path = g[len("create file "):].strip()
        return [Step("write_file", {"path": path, "content": "", "overwrite": False})]

    @staticmethod
    def _plan_append_file(g: str) -> List[Step]:
        # "append to <path> with <content>"
        if " with " in g:
            before, after = g.split(" with ", 1)
            path = before[len("append to "):].strip()
            content = after
            return [Step("append_file", {"path": path, "content": content})]
        # No content provided
        path = g[len("append to "):].strip()
        return [Step("append_file", {"path": path, "content": ""})]

    @staticmethod
    def _plan_download(g: str, original: str) -> List[Step]:
        # We preserve original casing for URL and path extraction
        low = g
        to_idx = low.index(" to ")
        url = original[len("download "): len("download ") + to_idx - len("download ")].strip()
        path = original[to_idx + len(" to "):].strip()
        return [Step("download", {"url": url, "dest_path": path, "overwrite": True})]