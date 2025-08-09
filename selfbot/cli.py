import argparse
import json
import os
from typing import List

from .planner import Planner
from .executor import Executor
from .memory import Memory
from .utils import print_boxed


def main(argv: List[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Rule-based self bot (non-AI)")
    parser.add_argument("goal", nargs="+", help="Goal to accomplish, e.g. create file /tmp/test.txt with hello")
    parser.add_argument("--memory", default=os.path.abspath("./selfbot_memory.json"), help="Path to memory JSON")
    parser.add_argument("--dry-run", action="store_true", help="Only show plan; do not execute")
    args = parser.parse_args(argv)

    goal = " ".join(args.goal)

    steps = Planner.plan(goal)

    print_boxed([
        "Goal",
        goal,
    ])

    print("Plan:")
    for idx, step in enumerate(steps, start=1):
        print(f"  {idx}. {step.action} {json.dumps(step.params)}")

    if args.dry_run:
        print("\nDry run: not executing.")
        return 0

    memory = Memory(args.memory)
    executor = Executor(memory)
    history = executor.execute(goal, steps)

    print("\nResults:")
    for h in history:
        status = "OK" if h.ok else "FAIL"
        print(f"  [{status}] step {h.step} {h.action}: {h.message}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())