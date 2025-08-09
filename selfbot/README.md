# Rent A Coder Template (via Selfbot)

This repository includes a minimal, dependency-free Python CLI that can execute a single JSON task template to scaffold, build, run, and zip a small C++ project.

## Run the template

```
python3 /workspace/selfbot/selfbot.py run \
  --task-file /workspace/selfbot/tasks/rentacoder.json \
  --var project_dir=/workspace/rac/MyProject \
  --var app_name=MyApp \
  --var class_name=Greeter \
  --verbose | cat
```

Results:
- Project at `/workspace/rac/MyProject`
- Binary at `/workspace/rac/MyProject/build/MyApp` (also copied to `bin/`)
- Zip at `/workspace/rac/MyProject.zip`

## Template reference

Task file: `/workspace/selfbot/tasks/rentacoder.json`

Variables:
- `project_dir`: Absolute path for the project directory
- `app_name`: Binary and project name
- `class_name`: Class to generate (e.g., `Greeter`)

Actions used (internals): `cpp_scaffold`, `cpp_add_class`, `cpp_write_main`, `cpp_compile`, `cpp_run_binary`, `zip_project`, `print`.