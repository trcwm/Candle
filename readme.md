CandleM
-----------
GRBL controller application with G-Code visualizer written in Qt.

![CandleM screenshot](screenshots/CandleM_v1.3.png?raw=true "CandleM screenshot")

Supported functions:
* Controlling GRBL-based cnc-machine via console commands, buttons on form, numpad.
* Monitoring cnc-machine state.
* Loading, editing, saving and sending of G-code files to cnc-machine.
* Visualizing G-code files.

Changes from the original Candle:
* Made buttons and jog permantently accessible.
* Moved console, override and spindle to a tab control.
* Removed heightfield code.

System requirements for running "CandleM":
-------------------
* Windows/Linux x86
* CPU with SSE2 instruction set support
* Graphics card with OpenGL 2.0 support
* 120 MB free storage space

Build requirements:
------------------
* Qt 5.12.2 with MinGW/GCC compiler
* libqt5serialport5-dev

Before creating new issue:
------
CandleM works with CNC controlled by GRBL firmware, many problems can be solved by using proper version of GRBL, using proper configuration.

Please read GRBL wiki:
- GRBL v1.1: https://github.com/gnea/grbl/wiki
