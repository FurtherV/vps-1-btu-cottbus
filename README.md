# Game of Life Simulator

## Description

This program simulates the GoL for a configurable amount of cycles, can load and stores boards and has an optional x11
graphical output.

## How to build

This project is build using gnu make.

### Dependencies

- A g++ and gcc version that supports at least c++14
- X11 (Ubuntu Package: `libx11-dev`)
- Boost Program Options (Ubuntu Package: `libboost-program-options-dev`)

### Targets

- clean: removes most output files
- server: builds the server application
- client: builds the client application
- main: builds the local application

### Make Variables

Usage: make target1 target2 target3 ... VARIABLE1=VALUE1 VARIABLE=VALUE2 ...

- DEBUG=1: Enables debug output (if logger is correctly configured in the main)
