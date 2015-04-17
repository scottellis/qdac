## qdac

Checkout the [linux] or [freebsd] branch.

Userland driver for an [MCP4728 Quad DAC][datasheet] attached to the I2C
bus of a Gumstix Duovero running FreeBSD. 

I was testing with this [eval board][eval-board]

I'm using `/dev/iic1` since that is the I2C bus brought out on the
Duovero Parlor expansion board header.

I have the I2C bus running at 400 kHz.

The files `mcp4728-qdac.c` and `mcp4728-qdac.h` are for incorporation
into a larger program.

The CLI `qdac.c` is intended only for testing `mcp4728-qdac.c`.

### Usage

    root@duovero:~# ./qdac -h

    Usage: ./qdac [-rweh][<ch> [<val>]]
      <none>            Dump the output of all input and eeprom registers
      -r <ch>           Read register for channel, <ch> range 0-3
      -w <ch> <val>     Write register for channel, <ch> range 0-3, <val> range 0-4095
                        Note: A value of zero will also disable that channel
      -e                Use with -r or -w. Works on the eeprom reg instead of the input reg
      -h                Show this help


The DAC has `input` and `eeprom` registers. Operation happens from the input
registers. The eeprom registers are used to populate the input registers at
startup.

Run `qdac` with no arguments to see the status of all `input` and `eeprom`
regs.

    root@duovero:~/qdac # ./qdac
     input[0]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[0]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[1]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[1]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[2]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[2]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[3]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[3]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0


Write to an `input` register

    root@duovero:~/qdac # ./qdac -w 2 2048

    root@duovero:~/qdac # ./qdac
     input[0]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[0]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[1]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[1]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[2]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 0  gain: 0  data: 2048
    eeprom[2]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[3]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[3]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0


Read just the data for a single `input` register

    root@duovero:~/qdac # ./qdac -r 2
    2048


Write to both the input and eeprom register

    root@duovero:~/qdac # ./qdac -w -e 0 3333

    root@duovero:~/qdac # ./qdac
     input[0]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 0  gain: 0  data: 3333
    eeprom[0]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 0  gain: 0  data: 3333
     input[1]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[1]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[2]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 0  gain: 0  data: 2048
    eeprom[2]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
     input[3]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0
    eeprom[3]: rdy: 1 por: 1  addr: 0  vref: 0  pd: 3  gain: 0  data: 0


[datasheet]: http://ww1.microchip.com/downloads/en/DeviceDoc/22187E.pdf
[eval-board]: http://www.digikey.com/product-search/en/programmers-development-systems/evaluation-boards-digital-to-analog-converters-dacs/2622540?k=mcp4728

