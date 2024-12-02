# HiddenFrame Backend

The HiddenFrame backend has a API server that gets exposed to localhost on port `8080` ([`http://localhost:8080`](http://localhost:8080))
This API server is build using [crow](https://crowcpp.org/master/) which relies on the [boost::asio](https://www.boost.org/) library

## Getting started

Before we build the backend executable you need to make sure [boost](https://www.boost.org/) is installed and the make file is pointing to the right paths

### MacOS

- Install [boost](https://www.boost.org/) using [homebrew](https://brew.sh/): `brew install boost`
- Ensure `$IncludePaths` in makefile includes both this project's `include/` subdirectory and homebrew's include directory `/opt/homebrew/include`
- Now the project can be built by running `make run`
- The executable can be found at `bin/HiddenFrame`

### Linux

- Install [boost](https://www.boost.org/) using your preferred package manager
  - On debian based systems you can run: `apt install libboost-all-dev`
- Ensure `$IncludePaths` in makefile includes both this project's `include/` subdirectory and homebrew's include directory `/usr/local/include` or the correct include path
- Now the project can be built by running `make run`
- The executable can be found at `bin/HiddenFrame`

### Windows

- Install [boost](https://www.boost.org/) 
- Ensure `$IncludePaths` in makefile includes both this project's `include/` subdirectory  directory `FIND CORRECT PATH` or the correct include path
- Now the project can be built by running `make run`
- The executable can be found at `bin/HiddenFrame.exe`
