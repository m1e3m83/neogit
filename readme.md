# NeoGit

NeoGit is a lightweight, educational reimplementation of Git written in **C**.  
It provides a simplified version control system (VCS) that mimics Git’s functionality such as commits, branches, merges, stashes, and tags.

This project was completed as part of the Fundermentals of Programming course in Sharif University of Technology, Feb 2024.
---

## Features

- **Repository Initialization** – Create and manage NeoGit repositories.
- **Configuration** – Set global and local user information (name, email).
- **Commit System** – Track file changes with commit history.
- **Branching & Merging** – Create branches and merge changes.
- **Stashing** – Save and restore work-in-progress changes.
- **Tagging** – Mark commits with version-style tags.
- **Aliases & Shortcuts** – Define custom command shortcuts.
- **Colored Console Output** – Enhanced readability in Windows terminal.

---

## Requirements

- **Operating System:** Windows  
- **Compiler:** GCC (MinGW) or MSVC  
- **Dependencies:** Standard C library, Windows API (`windows.h`)

---

## Build Instructions

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/neogit.git
   cd neogit
   ```

2. Compile with GCC (MinGW):
   ```bash
   gcc neogit.c -o neogit
   ```

3. Run:
   ```bash
   ./neogit <command>
   ```

You can put the neogit executable directory in your PATH enviroment variable for easier access and use.

---

## Usage

Initialize a repository:
```bash
neogit init
```

Configure user information:
```bash
neogit config --global user.name "John Doe"
neogit config --global user.email "john@example.com"
```

Make a commit:
```bash
neogit commit -m "Initial commit"
```

Create and switch branches:
```bash
neogit branch feature-x
neogit checkout feature-x
```

Stash changes:
```bash
neogit stash save "WIP: working on feature"
neogit stash pop
```

Tag a commit:
```bash
neogit tag v1.0
```

---


## Notes

- Currently works **only on Windows** due to `windows.h` dependencies.  
- Not a drop-in replacement for Git — meant for learning and experimentation.  
- File operations and conflict handling are simplified compared to Git.  

---

## License
This project is licensed under the [MIT License](https://opensource.org/licenses/MIT).
