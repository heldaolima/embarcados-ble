#!/bin/bash

BOARD="nrf52_dk"
CENTRAL_BIN="$(pwd)/central/.pio/build/$BOARD/firmware.elf"
PERIPHERAL_BIN="$(pwd)/peripheral/.pio/build/$BOARD/firmware.elf"

renode -e \
    "using sysbus

emulation CreateBLEMedium \"wireless\"

mach create \"central\"
machine LoadPlatformDescription @platforms/cpus/$BOARD.repl
connector Connect sysbus.radio wireless

showAnalyzer uart0

mach create \"peripheral\"
machine LoadPlatformDescription @platforms/cpus/$BOARD.repl
connector Connect sysbus.radio wireless

showAnalyzer uart0

emulation SetGlobalQuantum \"0.00001\"

macro reset
\"\"\"
    mach set \"central\"
    sysbus LoadELF @$CENTRAL_BIN

    mach set \"peripheral\"
    sysbus LoadELF @$PERIPHERAL_BIN 
\"\"\"

runMacro \$reset

start
"
