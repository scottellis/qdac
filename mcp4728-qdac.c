/*
  Copyright (c) 2014 Jumpnow Technologies, LLC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

// #define DEBUG

#define SET_VREF_PD_GAIN_TO_ZERO 1

#define I2C_BUS "/dev/i2c-1"
#define MCP4728_ADDRESS 0x60

struct dac_reg {
    uint8_t type;
    uint8_t rdy;
    uint8_t por;
    uint8_t ch;
    uint8_t addr;
    uint8_t vref;
    uint8_t pd;
    uint8_t gain;
    uint16_t data;
};

struct dac {
    struct dac_reg input;
    struct dac_reg eeprom;
};

#define NUM_DACS 4
struct qdac {
    struct dac dac[NUM_DACS];
};

static int qdac_open()
{
    int fd = open(I2C_BUS, O_RDWR);
    
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, MCP4728_ADDRESS) < 0) {
        perror("ioctl(I2C_SLAVE)");
        close(fd);
        return -1;
    }

    return fd;
}

/*
  b[0]: rdy | por | dac1 | dac0 | ? | a2 | a1 | a0
  b[1]: vref | pd1 | pd0 | gain | d11 | d10 | d8 | d9
  b[2]: d7-d0

  Looks like '?' is 0 for input reg and 1 for eeprom.
  Not sure though and not in the documentation.
*/
static void qdac_parse_read_reg(struct dac_reg *reg, uint8_t *d)
{
    reg->type = (d[0] >> 3) & 0x01;
    reg->rdy = (d[0] >> 7) & 0x01;
    reg->por = (d[0] >> 6) & 0x01;
    reg->ch = (d[0] >> 4) & 0x03;
    reg->addr = d[0] & 0x07;
    reg->vref = (d[1] >> 7) & 0x01;
    reg->pd = (d[1] >> 5) & 0x03;
    reg->gain = (d[1] >> 4) & 0x01;
    reg->data = ((d[1] & 0x0f) << 8) + d[2];
}

/*
  Write input register. EEPROM is not updated.
  c2=0, c1=1, c0=0, w1=0, w0=0 : 0x40

  Write input and EEPROM registers.
  c2=0, c1=1, c0=0, w1=1, w0=1 : 0x58

  b[0]: c2 | c1 | c0 | w1 | w0 | dac1 | dac0 | /udac
  b[1]: vref | pd1 | pd0 | gain | d11 | d10 | d9 | d8
  b[2]: d7-d0
*/
static int qdac_write_reg(int fd, struct dac_reg *reg, int eeprom)
{
    int len;
    uint8_t buff[4];

    if (eeprom)
        buff[0] = 0x58 | ((reg->ch & 0x3) << 1);
    else
        buff[0] = 0x40 | ((reg->ch & 0x3) << 1);

#ifdef SET_VREF_PD_GAIN_TO_ZERO
    reg->vref = 0;
    reg->pd = 0;
    reg->gain = 0;
#endif

    buff[1] = (reg->vref & 0x01) << 7;
    buff[1] |= (reg->pd & 0x3) << 5;
    buff[1] |= (reg->gain & 0x01) << 4;
    buff[1] |= (reg->data >> 8) & 0x0f;
    buff[2] = reg->data & 0xff;

#ifdef DEBUG
    printf("writing: %02X %02X %02X\n", buff[0], buff[1], buff[2]);
#endif

    len = write(fd, buff, 3);

    if (len != 3) {
        printf("write_reg error: %d\n", len);
        return -1;
    }

    return len;
}

static int qdac_read_regs(int fd, struct qdac *qdac)
{
    int len, i, j;
    uint8_t d[32];

    fd = qdac_open();

    if (fd < 0)
        return -1;

    bzero(d, sizeof(d));

    len = read(fd, d, 24);

    if (len != 24) {
        if (len < 0)
            perror("read");
        else
            printf("read returned %d\n", len);

        return -1;
    }
    else {
#ifdef DEBUG 
        for (i = 0; i < 24; i += 6) {
            printf("CH%d: REGS: %02X %02X %02X   EEPROM: %02X %02X %02X\n",
                i % 6, d[i], d[i+1], d[i+2], d[i+3], d[i+4], d[i+5]);
        }
#endif
        for (i = 0; i < NUM_DACS; i++) {
            j = i * 6;
            qdac_parse_read_reg(&qdac->dac[i].input, d + j);
            qdac_parse_read_reg(&qdac->dac[i].eeprom, d + j + 3);
        }
    }

    return 0;
}

static void dump_reg(struct dac_reg *reg)
{
    if (reg->type)
        printf("eeprom[%u]: ", reg->ch);
    else
        printf(" input[%u]: ", reg->ch);

    printf("rdy: %u por: %u  addr: %u  vref: %u  pd: %u  gain: %u  data: %u\n",
        reg->rdy, reg->por, reg->addr, reg->vref, reg->pd, reg->gain, reg->data);
}

int qdac_read(int ch, int eeprom)
{
    int fd;
    struct qdac qdac;

    if (ch < 0 || ch >= NUM_DACS)
        return -1;

    fd = qdac_open();

    if (fd < 0)
        return -1;

    if (qdac_read_regs(fd, &qdac) < 0) {
        close(fd);
        return -1;
    }

    close(fd);

    if (eeprom)
        return qdac.dac[ch].eeprom.data;
    else
        return qdac.dac[ch].input.data;
}

int qdac_write(int ch, int val, int eeprom)
{
    int fd, ret;
    struct qdac qdac;
    struct dac_reg *reg;

    if (ch < 0 || ch >= NUM_DACS)
        return -1;

    if (val < 0 || val > 4095)
        return -1;

    fd = qdac_open();

    if (fd < 0)
        return -1;

    if (qdac_read_regs(fd, &qdac) < 0) {
        close(fd);
        return -1;
    }    

    if (eeprom)
        reg = &qdac.dac[ch].eeprom;
    else
        reg = &qdac.dac[ch].input;

    reg->data = val;

    if (val == 0)
        reg->pd = 3;        
    else
        reg->pd = 0;

    ret = qdac_write_reg(fd, reg, eeprom);

    close(fd);

    return ret; 
}

int qdac_dump()
{
    int fd, i;
    struct qdac qdac;

    fd = qdac_open();

    if (fd < 0)
        return -1;

    if (qdac_read_regs(fd, &qdac) < 0) {
        close(fd);
        return -1;
    }    

    close(fd);

    for (i = 0; i < NUM_DACS; i++) {
        dump_reg(&qdac.dac[i].input);
        dump_reg(&qdac.dac[i].eeprom);
    } 

    return 0;
}

