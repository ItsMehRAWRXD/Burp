# Selfbot (non-AI automation)

A small, dependency-free Python CLI that runs JSON-defined tasks and can call itself for subtasks. It also supports a file-based task queue.

## Usage

- Run a single task file:

```
python3 /workspace/selfbot/selfbot.py run --task-file /workspace/selfbot/tasks/example.json --var who=World --var timestamp=$(date -Is)
```

- Watch a queue directory and execute dropped tasks:

```
python3 /workspace/selfbot/selfbot.py watch --queue-dir /workspace/selfbot/queue --verbose
```

Then drop JSON task files into the queue directory. Results are moved to `_done` or `_failed`.

## Task schema

Minimal shape:

```json
{
  "name": "task-name",
  "steps": [
    { "action": "print", "params": {"message": "Hello"} }
  ]
}
```

Supported actions: `print`, `set`, `write_file`, `read_file`, `run`, `http_get`, `sleep`, `copy`, `call_self`.

Use `{var}` placeholders in strings to reference variables set via `--var/--varjson`, `set`, or outputs from actions.

## Rent A Coder workflow (C++)

Actions: `cpp_scaffold`, `cpp_add_class`, `cpp_write_main`, `cpp_compile`, `cpp_run_binary`, `zip_project`.

Example job:

```
python3 /workspace/selfbot/selfbot.py run \
  --task-file /workspace/selfbot/tasks/rentacoder.json \
  --var project_dir=/workspace/rac/MyProject \
  --var app_name=MyApp \
  --var class_name=Greeter \
  --verbose | cat
```

Result:
- Project at `/workspace/rac/MyProject`
- Binary at `/workspace/rac/MyProject/build/MyApp` (also copied to `bin/`)
- Zip at `/workspace/rac/MyProject.zip`