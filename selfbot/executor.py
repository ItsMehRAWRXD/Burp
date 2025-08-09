from typing import List
from .planner import Step
from .actions import SAFE_ACTIONS
from .memory import Memory, MemoryEntry


class Executor:
    def __init__(self, memory: Memory):
        self.memory = memory

    def execute(self, goal: str, steps: List[Step]) -> List[MemoryEntry]:
        run_idx = self.memory.start_run(goal)
        history: List[MemoryEntry] = []
        for i, step in enumerate(steps, start=1):
            action_fn = SAFE_ACTIONS.get(step.action)
            if not action_fn:
                entry = MemoryEntry(i, step.action, step.params, False, f"Unknown action: {step.action}")
                self.memory.append(run_idx, entry)
                history.append(entry)
                continue
            try:
                result = action_fn(**step.params)
                entry = MemoryEntry(i, step.action, step.params, result.ok, result.message)
            except Exception as e:
                entry = MemoryEntry(i, step.action, step.params, False, f"Exception: {e}")
            self.memory.append(run_idx, entry)
            history.append(entry)
        return history