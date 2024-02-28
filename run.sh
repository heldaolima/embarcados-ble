#!/bin/bash

BOARD="nrf52840"
FOLDER="nrf52840_dk"
CENTRAL_BIN="./central/.pio/build/$FOLDER/firmware.elf"
PERIPHERAL_BIN="./peripheral/.pio/build/$FOLDER/firmware.elf"

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
