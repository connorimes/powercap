/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * A tree structure for powercap control types.
 *
 * @author Connor Imes
 * @date 2020-11-15
 */
#ifndef _POWERCAP_TREE_H_
#define _POWERCAP_TREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdlib.h>

// TODO: There could be thousands of zones (e.g., one per CPU on a large system).
// What kind of information does a user need if they want to be able to apply a more efficient structure?
// What can we do to let user optimize their allocations? What about our own allocations?


/**
 * A powercap tree root structure.
 */
typedef struct powercap_tree_root powercap_tree_root;

/**
 * A callback function.
 * The ctx_node argument is the user's context, which the callback function can do with as it pleases, e.g., assign,
 * use, modify, or destroy.
 */
typedef int (powercap_tree_cb)(const char* control_type, const uint32_t* zones, uint32_t depth, void** ctx_node);

/**
 * Build a tree for a control type.
 *
 * @param control_type
 * @return powercap_tree_root pointer on success, NULL otherwise.
 */
powercap_tree_root* powercap_tree_root_init(const char* control_type);

/**
 * Destroy a tree.
 *
 * @param ptr
 * @return 0 on success, a negative error code otherwise.
 */
int powercap_tree_root_destroy(powercap_tree_root* ptr);

/**
 * Walk the powercap tree.
 * This is a preorder depth-first traversal.
 *
 * @param ptr
 * @param cb
 * @return 0 on success, something else otherwise (user's discretion from the callback)
 */
int powercap_tree_root_walk(powercap_tree_root* ptr, powercap_tree_cb* cb);


#ifdef __cplusplus
}
#endif

#endif
