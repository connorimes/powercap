/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Tree tests
 */
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "powercap.h"
#include "powercap-tree.h"

#define CONTROL_TYPE_DEFAULT "intel-rapl"

int test_cb(const char* control_type, const uint32_t* zones, uint32_t depth, void** ctx_node) {
  printf("Got callback\n");
  return 0;
}

int main(int argc, char** argv) {
  const char* control_type = CONTROL_TYPE_DEFAULT;
  int ret;
  if (argc > 1) {
    control_type = argv[1];
  }
  powercap_tree_root* ptr = powercap_tree_root_init(control_type);
  if (!ptr) {
    perror("powercap_tree_root_init");
    return -1;
  }
  ret = powercap_tree_root_walk(ptr, test_cb);
  if (powercap_tree_root_destroy(ptr)) {
    perror("powercap_tree_root_destroy");
    return -1;
  }
  return ret;
}
