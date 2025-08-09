from typing import Iterable


def print_boxed(lines: Iterable[str]) -> None:
    lines = list(lines)
    width = max((len(line) for line in lines), default=0)
    border = "+" + "-" * (width + 2) + "+"
    print(border)
    for line in lines:
        padding = " " * (width - len(line))
        print(f"| {line}{padding} |")
    print(border)