# Selfbot (Non-AI rule-based automation)

A small, self-contained command line bot that converts a textual goal into a deterministic plan and executes it using a fixed set of safe actions (filesystem, process, download).

## Quick start

- Run a dry plan:

```
python -m selfbot.cli --dry-run create file /workspace/selfbot_output/demo.txt with hello world
```

- Execute the plan:

```
python -m selfbot.cli create file /workspace/selfbot_output/demo.txt with hello world
```

- List a directory:

```
python -m selfbot.cli list dir /workspace/selfbot_output
```

- Run a command:

```
python -m selfbot.cli run echo done
```

- Download a file:

```
python -m selfbot.cli "download https://example.com to /workspace/selfbot_output/example.html"
```

Memory is stored at `./selfbot_memory.json` by default. Use `--memory` to change path.