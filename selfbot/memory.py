import json
import os
from dataclasses import dataclass, asdict
from typing import Any, Dict, List


@dataclass
class MemoryEntry:
    step: int
    action: str
    params: Dict[str, Any]
    ok: bool
    message: str


class Memory:
    def __init__(self, storage_path: str):
        self.storage_path = storage_path
        os.makedirs(os.path.dirname(storage_path) or ".", exist_ok=True)
        self._data = {
            "runs": []
        }
        if os.path.exists(storage_path):
            try:
                with open(storage_path, "r", encoding="utf-8") as f:
                    self._data = json.load(f)
            except Exception:
                pass

    def start_run(self, goal: str) -> int:
        run = {"goal": goal, "history": []}
        self._data["runs"].append(run)
        self._save()
        return len(self._data["runs"]) - 1

    def append(self, run_index: int, entry: MemoryEntry) -> None:
        self._data["runs"][run_index]["history"].append(asdict(entry))
        self._save()

    def _save(self) -> None:
        tmp = self.storage_path + ".tmp"
        with open(tmp, "w", encoding="utf-8") as f:
            json.dump(self._data, f, indent=2)
        os.replace(tmp, self.storage_path)

    def get_runs(self) -> List[Dict[str, Any]]:
        return self._data.get("runs", [])