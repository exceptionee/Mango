<div align="center">
  <img src="https://cdn.discordapp.com/attachments/1164976468741345311/1171108909675720724/mango.png?ex=655b7b2a&is=6549062a&hm=06a9aaba81c1c98f9ff0d11b4872c56a65ae45c4aec685b9e36e01a3319c7ff0&"/>
</div>

![Static Badge](https://img.shields.io/badge/License-MIT-blue?style=flat-square)

The official repository for the Mango programming Language.
## Installation

Download the repository and then the needed `.jar` files from the following links.

[antlr-4.13.1-complete.jar](https://www.antlr.org/download/antlr-4.13.1-complete.jar)

[picocli-4.7.5.jar](https://github.com/remkop/picocli/releases/download/v4.7.5/picocli-4.7.5.jar)
## Run Locally

If you are on windows you can use the following ``batch`` script to run the compiler.

``` bat
@echo off

java -cp "bin/;lib/*"; Compiler %*
```

Otherwise just use the Java command.

``` bash
java -cp "bin/;lib/*"; Compiler %* <flags>
```
## Flags

| Flag        | Description                                 |
| :-----------| :-------------------------------------------|
| `--gui`     | Opens a window with a GUI of the parse tree.|

## Contributing

Contributions are always welcome!

See `contributing.md` for ways to get started.