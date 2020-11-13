# Git PR Comparison Tool
This tool creates a (html) document showing a git diff. Initially it was planned to use an abstract syntax tree described by Schulz C. et. al ([https://doi.org/10.1109/VISSOFT.2018.00017](https://doi.org/10.1109/VISSOFT.2018.00017)) to give a better diff of the tree and use this tree structure for further comparative visualizations, but thus far it only mimics `git diff` combined with color encoding. Inspired by [https://github.com/theZiz/aha](https://github.com/theZiz/aha).

## Usage
*Note:* The plotting capabilities requires the **gnuplot** executable to be installed and added to path.

If working directory is in repository, simply specify the commits to compare and it will generate a `diff.html` file in that repository:
```
GitPRComp 602a626 HEAD
```
You can specify which repository to use with the `-r` option
```
GitPRComp -r /path/to/repo 602a626 HEAD
```
You can also get some general help with the `-h` option.
```
GitPRComp -h
```

## Bulding
The project is laid out using CMake and git submodules and the CMakeLists is set up to download submodules if they weren't cloned together with the repository.

