/*
  Copyright (c) 2014 Jumpnow Technologies, LLC
 */

#ifndef MCP4728_QDAC_H
#define MCP4728_QDAC_H

int qdac_read(int ch, int eeprom);
int qdac_write(int ch, int val, int eeprom);
int qdac_dump();

#endif
