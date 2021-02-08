/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * A tree structure for powercap control types.
 *
 * @author Connor Imes
 * @date 2020-11-15
 */
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "powercap-sysfs.h"
#include "powercap-tree.h"

#ifndef POWERCAP_TREE_MAX_DEPTH_START
#define POWERCAP_TREE_MAX_DEPTH_START 64
#endif

typedef struct powercap_tree_node powercap_tree_node;

/**
 * The root node for a control type.
 */
struct powercap_tree_root {
  powercap_tree_node* children;
  uint32_t n_children;
  char* name;
  size_t depth_max;
};

/**
 * A node in a powercap tree: with <=1 zone, >=0 constraints, and >=0 children.
 */
struct powercap_tree_node {
  powercap_tree_node* children;
  uint32_t n_children;
  void* ctx_node;
};


static void powercap_tree_node_destroy(powercap_tree_node *ptn) {
  uint32_t i;
  for (i = 0; i < ptn->n_children; i++) {
    powercap_tree_node_destroy(&ptn->children[i]);
  }
  free(ptn->children);
}

static int powercap_tree_node_init(powercap_tree_root* ptr, powercap_tree_node *ptn, uint32_t** zones, size_t* zones_len, uint32_t depth) {
  uint32_t* zones_realloc;
  uint32_t i;
  int err_save;

  if (depth > ptr->depth_max) {
    ptr->depth_max = depth;
  }

  if (depth >= *zones_len) {
    if (!(zones_realloc = realloc(*zones, *zones_len * 2 * sizeof(**zones)))) {
      return -1;
    }
    *zones = zones_realloc;
    *zones_len = *zones_len * 2;
  }

  // children
  (*zones)[depth] = 0;
  while (!powercap_sysfs_zone_exists(ptr->name, *zones, depth + 1)) {
    (*zones)[depth]++;
  }
  ptn->n_children = (*zones)[depth];
  if (ptn->n_children && !(ptn->children = calloc(ptn->n_children, sizeof(*ptn->children)))) {
    err_save = errno;
    ptn->n_children = 0;
    powercap_tree_node_destroy(ptn);
    errno = err_save;
    return -1;
  }

  // recurse
  for (i = 0; i < ptn->n_children; i++) {
    (*zones)[depth] = i;
    if (powercap_tree_node_init(ptr, &ptn->children[i], zones, zones_len, depth + 1)) {
      err_save = errno;
      ptn->n_children = i;
      powercap_tree_node_destroy(ptn);
      errno = err_save;
      return -1;
    }
  }

  return 0;
}

powercap_tree_root* powercap_tree_root_init(const char* name) {
  powercap_tree_root* ptr;
  uint32_t* zones;
  size_t zones_len = POWERCAP_TREE_MAX_DEPTH_START;
  uint32_t i;
  int err_save;

  if (powercap_sysfs_control_type_exists(name)) {
    errno = ENODEV;
    return NULL;
  }

  if (!(ptr = calloc(1, sizeof(*ptr)))) {
    return NULL;
  }

  // name
  if (!(ptr->name = strdup(name))) {
    free(ptr);
    return NULL;
  }

  // children
  ptr->n_children = 0;
  while (!powercap_sysfs_zone_exists(name, &ptr->n_children, 1)) {
    ptr->n_children++;
  }
  if (ptr->n_children) {
    if (!(ptr->children = calloc(ptr->n_children, sizeof(*ptr->children)))) {
      free(ptr->name);
      free(ptr);
      return NULL;
    }
  }

  // recurse
  if (ptr->n_children) {
    if (!(zones = malloc(zones_len * sizeof(*zones)))) {
      ptr->n_children = 0;
      err_save = errno;
      powercap_tree_root_destroy(ptr);
      errno = err_save;
      return NULL;
    }
    for (i = 0; i < ptr->n_children; i++) {
      zones[0] = i;
      if (powercap_tree_node_init(ptr, &ptr->children[i], &zones, &zones_len, 1)) {
        err_save = errno;
        ptr->n_children = i;
        powercap_tree_root_destroy(ptr);
        free(zones);
        errno = err_save;
        return NULL;
      }
    }
    free(zones);
  }

  return ptr;
}

int powercap_tree_root_destroy(powercap_tree_root* ptr) {
  size_t i;
  for (i = 0; i < ptr->n_children; i++) {
    powercap_tree_node_destroy(&ptr->children[i]);
  }
  free(ptr->children);
  free(ptr->name);
  free(ptr);
  return 0;
}

static int powercap_tree_node_walk(powercap_tree_root* ptr, powercap_tree_cb* cb, powercap_tree_node* ptn, uint32_t* zones, uint32_t depth) {
  int ret;
  size_t i;
  if ((ret = cb(ptr->name, zones, depth, &ptn->ctx_node))) {
    return ret;
  }
  for (i = 0; i < ptn->n_children; i++) {
    zones[depth] = i;
    if ((ret = powercap_tree_node_walk(ptr, cb, &ptn->children[i], zones, depth + 1))) {
      break;
    }
  }
  return ret;
}

int powercap_tree_root_walk(powercap_tree_root* ptr, powercap_tree_cb* cb) {
  size_t i;
  int ret = 0;
  uint32_t* zones;
  if (ptr->depth_max) {
    if (!(zones = malloc(ptr->depth_max * sizeof(*zones)))) {
      return -1;
    }
    for (i = 0; i < ptr->n_children; i++) {
      zones[0] = i;
      if ((ret = powercap_tree_node_walk(ptr, cb, &ptr->children[i], zones, 1))) {
        break;
      }
    }
    free(zones);
  }
  return ret;
}
