/*
 * File-Name: lhp_metadata.h
 * File-Desc: header file for metadata services in lhp_metadata.c
 * App-Name: lh_parser
 * Project-Name: Las-Header-Parser
 * Copyright: Copyright (c) 2020, DC Slagel
 * License-Identifier: MIT
 */

#ifndef LHP_METADATA_H
#define LHP_METADATA_H

#include "lhp_line.h"

/* public structs */
struct LhpMetadata {
  char* mnemonic_name;
  char* unit;
  char* value;
  char* desc;
};

struct LhpData {
  struct LhpMetadata* array;
  size_t len;
};
/* end public structs */

/* public-functions */
extern struct LhpMetadata* lhp_metadata_init(size_t size);
extern void lhp_data_init(struct LhpData* lhpdata, size_t size);
extern void parse_data_line(struct LhpMetadata* lhp_metadata, struct LhpLine* line);
/* end public-functions */

#endif /* LHP_METADATA_H */
