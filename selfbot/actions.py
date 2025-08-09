import os
import json
import shutil
import subprocess
from dataclasses import dataclass
from typing import List, Dict, Any, Optional


@dataclass
class ActionResult:
    ok: bool
    message: str
    data: Optional[Dict[str, Any]] = None


class Actions:
    """Safe, deterministic actions the bot can perform without AI."""

    @staticmethod
    def read_file(path: str, max_bytes: int = 200_000) -> ActionResult:
        try:
            with open(path, "rb") as f:
                content = f.read(max_bytes)
            return ActionResult(True, f"Read {len(content)} bytes from {path}", {"content": content.decode(errors="replace")})
        except Exception as e:
            return ActionResult(False, f"read_file failed: {e}")

    @staticmethod
    def write_file(path: str, content: str, overwrite: bool = False) -> ActionResult:
        try:
            if os.path.exists(path) and not overwrite:
                return ActionResult(False, f"write_file refused: {path} exists (set overwrite=True)")
            os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
            with open(path, "w", encoding="utf-8") as f:
                f.write(content)
            return ActionResult(True, f"Wrote {len(content)} bytes to {path}")
        except Exception as e:
            return ActionResult(False, f"write_file failed: {e}")

    @staticmethod
    def append_file(path: str, content: str) -> ActionResult:
        try:
            os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
            with open(path, "a", encoding="utf-8") as f:
                f.write(content)
            return ActionResult(True, f"Appended {len(content)} bytes to {path}")
        except Exception as e:
            return ActionResult(False, f"append_file failed: {e}")

    @staticmethod
    def list_dir(path: str) -> ActionResult:
        try:
            entries = os.listdir(path)
            return ActionResult(True, f"Listed {len(entries)} entries in {path}", {"entries": entries})
        except Exception as e:
            return ActionResult(False, f"list_dir failed: {e}")

    @staticmethod
    def copy(src: str, dst: str, overwrite: bool = False) -> ActionResult:
        try:
            if os.path.exists(dst) and not overwrite:
                return ActionResult(False, f"copy refused: {dst} exists (set overwrite=True)")
            os.makedirs(os.path.dirname(dst) or ".", exist_ok=True)
            shutil.copy2(src, dst)
            return ActionResult(True, f"Copied {src} -> {dst}")
        except Exception as e:
            return ActionResult(False, f"copy failed: {e}")

    @staticmethod
    def move(src: str, dst: str, overwrite: bool = False) -> ActionResult:
        try:
            if os.path.exists(dst) and not overwrite:
                return ActionResult(False, f"move refused: {dst} exists (set overwrite=True)")
            os.makedirs(os.path.dirname(dst) or ".", exist_ok=True)
            shutil.move(src, dst)
            return ActionResult(True, f"Moved {src} -> {dst}")
        except Exception as e:
            return ActionResult(False, f"move failed: {e}")

    @staticmethod
    def delete(path: str) -> ActionResult:
        try:
            if os.path.isdir(path):
                shutil.rmtree(path)
            else:
                os.remove(path)
            return ActionResult(True, f"Deleted {path}")
        except Exception as e:
            return ActionResult(False, f"delete failed: {e}")

    @staticmethod
    def run(command: List[str], cwd: Optional[str] = None, timeout: int = 60) -> ActionResult:
        try:
            proc = subprocess.run(
                command,
                cwd=cwd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=timeout,
                text=True,
            )
            ok = proc.returncode == 0
            return ActionResult(
                ok,
                f"run exited {proc.returncode}",
                {"stdout": proc.stdout[-8000:], "stderr": proc.stderr[-8000:], "returncode": proc.returncode},
            )
        except Exception as e:
            return ActionResult(False, f"run failed: {e}")

    @staticmethod
    def download(url: str, dest_path: str, overwrite: bool = False) -> ActionResult:
        try:
            import urllib.request

            if os.path.exists(dest_path) and not overwrite:
                return ActionResult(False, f"download refused: {dest_path} exists (set overwrite=True)")
            os.makedirs(os.path.dirname(dest_path) or ".", exist_ok=True)
            urllib.request.urlretrieve(url, dest_path)
            return ActionResult(True, f"Downloaded {url} -> {dest_path}")
        except Exception as e:
            return ActionResult(False, f"download failed: {e}")


SAFE_ACTIONS = {
    "read_file": Actions.read_file,
    "write_file": Actions.write_file,
    "append_file": Actions.append_file,
    "list_dir": Actions.list_dir,
    "copy": Actions.copy,
    "move": Actions.move,
    "delete": Actions.delete,
    "run": Actions.run,
    "download": Actions.download,
}