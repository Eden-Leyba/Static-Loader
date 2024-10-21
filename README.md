# Static-Loader

This project implements a static loader for Linux that runs static executable files, which do not rely on dynamic libraries. The loader directly interfaces with the system call interface, bypassing standard libraries like `glibc`.
## Features
- **Execute static binaries:** Supports running static executable files that are compiled without dynamic library dependencies.
- **System call interface:** Loads and runs binaries using raw system calls, without relying on standard libraries (e.g., no `libc`).

## Project Structure
loader.c: The core logic of the loader, responsible for loading and executing static executables.
