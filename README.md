## Seal-Ntt-Lang

Seal Ntt is a definition language designed to be simple to integrate with C/C++ programs.

Simple modular customizeable definition language.

# Syntax

The language is designed to make template defenitions simple, So the general syntax
looks like this.
```
[property] [Value]
```
Where Value can be a function call, Function calls look like:
```
[name] [args]...
```
Seal-Ntt has 2 builtin types string and number, where number can be either an int or a float.
You can extend and create your own types by embedding the languagge in your own project, and making function to create them.

## Compilation

*Unix/Linux*
If you would only like to used the library just use `make` on linux.
If you would to compile the demo, use `make demo`.

In both senarios a file named `ntt-*.*.so` will be created, which is the shared object file.
If you compile with the `make demo`, the executable is `ntt`.
