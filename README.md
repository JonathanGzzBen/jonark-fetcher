# jonark-fetcher

System information fetcher written by Jonark.

## Building

### Dependencies

- C Compiler (gcc or clang)
- GNU Make
- [Collections-C](https://github.com/srdja/Collections-C)

> [!IMPORTANT]
> If you are installing Collections-C in a custom prefix (e.g. `$HOME/Programming/prefixes/jonark-fetcher`),
> you might need to copy the header files too, as they are not copied in its `make install`:

```shell
$ git clone
$ cd Collections-C
$ mkdir -p $HOME/Programming/prefixes/jonark-fetcher/include/
$ cmake -B build -S . -dCMAKE_INSTALL_PREFIX=$HOME/Programming/prefixes/jonark-fetcher
$ cmake --install . --prefix $HOME/Programming/prefixes/jonark-fetcher
$ cp -r src/include/* $HOME/Programming/prefixes/jonark-fetcher/include/
```

## Compiling the project


```shell

$ make

```

Or, if using a custom prefix (e.g. `$HOME/Programming/prefixes/jonark-fetcher`):


```shell

$ make PREFIX=$HOME/Programming/prefixes/jonark-fetcher

```

