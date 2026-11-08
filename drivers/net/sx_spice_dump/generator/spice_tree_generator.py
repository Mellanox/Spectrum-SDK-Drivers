#!/usr/bin/env python3

import sys
import os
import xml.etree.ElementTree as ET
import sys

sdk_root = os.path.abspath(__file__).split(os.sep + 'sxd_kernel' + os.sep, 1)[0]
applibs_root = os.path.join(sdk_root, 'applibs')
if applibs_root not in sys.path:
    sys.path.append(applibs_root)
from sx_api_utils.utils import license_utils

OUTPUT_DIR = ""

# List of files to be generated
SX_SPICE_TREE_AUTO_H = "sx_spice_tree_auto.h"
SX_SPICE_TREE_AUTO_C = "sx_spice_tree_auto.c"

# SPICE TREE GENERATOR DBs:

# Tree Dictionary: key: tree name, value: tree root node
TREE_DICT = {}

# Indexes DB - holds indexes information used for global declarations.
# Note: hidden_idxs uses list (not set) to preserve insertion order for consistent code generation
INDEXES_DB = {'seq_idxs': {}, 'enum_idxs': {}, 'hidden_idxs': []}

INDEX_SET_DB = {'eth': {}, 'ib': {}}

# Loading mode filters DB - stores named filter definitions per (tree_name, protocol)
# Key: (tree_name, protocol), Value: dict of filter_name -> (index_name, values_list)
LOADING_MODE_FILTERS_DB = {}

AUTO_GENERATION_DICT = {
    SX_SPICE_TREE_AUTO_H: "spice_tree_auto_header_gen()",
    SX_SPICE_TREE_AUTO_C: "spice_tree_auto_source_gen()"}

# Mode flags (internal to generator for parsing loading_mode attribute)
MODE_DEFAULT = 0x1
MODE_REDUCED = 0x2
MODE_PERF_CNTR_ONLY = 0x4

SPICE_TREE_H_AUTO_PATTERN = """
#include <linux/netdevice.h>
#include "sx_spice_wrapper.h"

/************************************************
 *  Type definitions
 ***********************************************/
%s

typedef enum {
    SPICE_LOADING_MODE_DEFAULT = 0, /* Default mode - creates all debugfs entries at init */
    SPICE_LOADING_MODE_REDUCED = 1, /* Reduced mode - lazy/on-demand debugfs creation */
    SPICE_LOADING_MODE_PERF_CNTR_ONLY = 2, /* Performance counters only */
} spice_loading_mode_e;

/************************************************
 *  Global variables
 ***********************************************/

/************************************************
 *  Function declarations
 ***********************************************/

%s
"""

SPICE_TREE_C_AUTO_PATTERN = """
#include <linux/mlx_sx/kernel_user.h>
#include "sx_spice_tree_auto.h"
#include "sx_spice_access_reg.h"
#include "sx_spice_access_reg_auto.h"
#include "sx_spice_tree_dynamic.h"
#include "sx_spice_bulk_cntr.h"
#include "sx_spice_asic_ports.h"

/************************************************
 *  Global variables
 ***********************************************/

extern int loading_mode;

/************************************************
 *  Local variables
 ***********************************************/
%s

/************************************************
 *  Local function declarations
 ***********************************************/
%s

/************************************************
 *  Function implementations
 ***********************************************/
%s

"""


def get_mode_flag_from_attr(mode_attr):
    """Convert loading_mode attribute to mode flags.

    Logic:
    - No attr or "default" → Only in full tree (loading_mode=0)
    - "reduced" → In both full tree AND reduced tree
    - "perf_cntr" → In both full tree AND perf_cntr mode
    - "reduced,perf_cntr" → In all three modes
    """
    if not mode_attr or mode_attr.lower().strip() == 'default':
        return MODE_DEFAULT

    flags = MODE_DEFAULT  # Always include default
    modes = mode_attr.lower().split(',')
    for m in modes:
        m = m.strip()
        if m == 'reduced':
            flags |= MODE_REDUCED
        elif m == 'perf_cntr':
            flags |= MODE_PERF_CNTR_ONLY

    return flags


def spice_tree_index_enum_gen():
    """Generates index type enum declaration."""
    code = ""

    for idx in INDEXES_DB['seq_idxs']:
        code += f"    {idx.upper()},\n"

    for idx in INDEXES_DB['hidden_idxs']:
        code += f"    {idx.upper()},\n"

    # Use list to preserve order, check uniqueness manually
    enum_index_list = []
    for idx in INDEXES_DB['enum_idxs']:
        idx_name = idx[0].upper()
        if idx_name not in enum_index_list:
            enum_index_list.append(idx_name)
    for enum_idx in enum_index_list:
        code += f"    {enum_idx},\n"

    code = f"""
typedef enum {{
{code}    MAX_INDEX_TYPE
}} index_type_e;
"""

    return code


def spice_tree_index_struct_gen():
    """Generates index type structure declaration."""
    return """
typedef struct index_type_data {
    char    field_name[50];
    uint8_t field_value;
} index_type_data_t;
"""


def spice_tree_func_declaration_gen():
    """Generates public function declaration."""
    code = """int sx_spice_tree_init(void);
void sx_spice_tree_deinit(void);
uint16_t sx_spice_tree_index_value_get(index_type_e index);
"""

    for tree_name, root in TREE_DICT.items():
        code += f"int sx_spice_tree_{tree_name}_app_dir_create(void);\n"
        # Only declare functions for protocols that exist in the ADB
        for prot_node in root:
            if prot_node.tag == 'protocol':
                protocol = prot_node.get('name')
                code += f"int sx_spice_tree_{tree_name}_{protocol}_create(dev_private_data_t *dev_data);\n"

    return code


def spice_tree_auto_header_gen():
    """Generates SPICE tree header code."""
    type_def_code = spice_tree_index_struct_gen() + spice_tree_index_enum_gen()
    func_dec_code = spice_tree_func_declaration_gen()

    return SPICE_TREE_H_AUTO_PATTERN % (type_def_code, func_dec_code)


def spice_tree_debugfs_global_dir_pointer_gen():
    """Generates global debugFS declaration."""
    code = "static struct dentry * root_p;\n"
    for tree_name in TREE_DICT.keys():
        code += f"static struct dentry * {tree_name}_p;\n"
    return code


def spice_tree_index_type_array_gen():
    """Generates indexes array declaration."""
    return "static uint16_t indexes_arr_g[MAX_INDEX_TYPE];\n"


def spice_tree_string_value_declaration_gen():
    """Generate enum index data array declarations."""
    code = ""
    for idx in INDEXES_DB['enum_idxs']:
        idx_name = idx[0]
        idx_reg = idx[1]
        prefix = idx_reg + '_' + idx_name.lower() if idx_reg else idx_name.lower()
        idx_num = len(INDEXES_DB['enum_idxs'][idx])
        code += f"static index_type_data_t {prefix}_data_arr_g[{idx_num}];\n"
    return code


def spice_tree_index_set_array_declaration_gen():
    """Generate index set array declarations."""
    code = ""
    enum_idx_dict = INDEXES_DB['enum_idxs']
    seq_idx_dict = INDEXES_DB['seq_idxs']

    for protocol in INDEX_SET_DB:
        for idx1, idx2 in INDEX_SET_DB[protocol]:
            (reg1, reg2), val_list = INDEX_SET_DB[protocol][(idx1, idx2)]

            if (idx1, reg1) in enum_idx_dict:
                idx1_val_num = len(enum_idx_dict[(idx1, reg1)])
                prefix1 = idx1 + '_' + reg1 if reg1 else idx1
            elif idx1 in seq_idx_dict:
                (min_val, max_val) = seq_idx_dict[idx1]
                idx1_val_num = int(max_val) + 1
                prefix1 = idx1
            else:
                print(f"Error - Index {idx1} does not exist in INDEXES_DB")
                continue

            if (idx2, reg2) in enum_idx_dict:
                idx2_val_num = len(enum_idx_dict[(idx2, reg2)])
                prefix2 = idx2 + '_' + reg2 if reg2 else idx2
            elif idx2 in seq_idx_dict:
                (min_val, max_val) = seq_idx_dict[idx2]
                idx2_val_num = int(max_val) + 1
                prefix2 = idx2
            else:
                print(f"Error - Index {idx2} does not exist in INDEXES_DB")
                continue

            code += f"static char index_set_{protocol}_{prefix1}_{prefix2}_arr_g[{idx1_val_num}][{idx2_val_num}];\n"

    return code


def parse_values_string(values_str):
    """Parse comma-separated values string, handling hex values."""
    values = []
    for v in values_str.split(','):
        v = v.strip()
        if v.startswith('0x') or v.startswith('0X'):
            values.append(int(v, 16))
        else:
            values.append(int(v))
    return values


def filter_name_to_c_condition(filter_name, filter_db):
    """Convert a filter name to its C condition string."""
    if filter_name not in filter_db:
        return filter_name  # Return as-is if not found (will cause error later)
    index_name, values = filter_db[filter_name]
    idx_upper = index_name.upper()

    if len(values) == 1:
        return f"(indexes_arr_g[{idx_upper}] == {values[0]})"
    else:
        # Multiple values: simple OR chain
        values_check = " || ".join([f"indexes_arr_g[{idx_upper}] == {v}" for v in values])
        return f"({values_check})"


def parse_filter_expression(expr, tree_name, protocol):
    """
    Parse a filter expression and return the C condition code.
    Simple approach: substitute filter names with C conditions, then convert operators.

    Example: "(network_only AND pddr_pages) OR (usr_tile AND page0)"
    Becomes: "((port_type==0) && (page_select==0||...)) || ((port_type==5) && (page_select==0))"
    """
    if not expr:
        return None

    filter_db_key = (tree_name, protocol)
    if filter_db_key not in LOADING_MODE_FILTERS_DB:
        print(f"Warning: No filters defined for {tree_name}/{protocol}")
        return None

    filter_db = LOADING_MODE_FILTERS_DB[filter_db_key]
    result = expr

    # Replace filter names with their C conditions (longest names first to avoid partial matches)
    for name in sorted(filter_db.keys(), key=len, reverse=True):
        c_cond = filter_name_to_c_condition(name, filter_db)
        result = result.replace(name, c_cond)

    # Replace operators: AND -> &&, OR -> ||
    result = result.replace(' AND ', ' && ').replace(' OR ', ' || ')

    return result


def parse_loading_mode_filters(reg_node, tree_name, protocol):
    """
    Parse loading mode filters from a <reg> element.
    Supports: reduced_filter="filter1" or "filter1 AND filter2" or "(f1 AND f2) OR f3"
    Returns the C condition code string, or None if no filters.
    """
    expr = reg_node.get('reduced_filter', '')
    return parse_filter_expression(expr, tree_name, protocol) if expr else None


def spice_tree_src_local_vars_gen():
    """Generates SPICE tree C file local variables declarations."""
    code = spice_tree_index_type_array_gen()
    code += '\n' + spice_tree_string_value_declaration_gen()
    code += '\n' + spice_tree_index_set_array_declaration_gen()
    code += '\n' + spice_tree_debugfs_global_dir_pointer_gen()
    return code


def spice_tree_string_value_func_declaration_gen():
    """Generates enum index string value function declaration."""
    code = ""
    for idx in INDEXES_DB['enum_idxs']:
        idx_name = idx[0]
        idx_reg = idx[1]
        prefix = idx_reg + '_' + idx_name.lower() if idx_reg else idx_name.lower()
        code += f"static void {prefix}_arr_init(void);\n"
    return code


def spice_tree_index_set_arr_func_declaration_gen():
    """Generates index set array function declaration."""
    code = ""
    for protocol in INDEX_SET_DB:
        for idx1, idx2 in INDEX_SET_DB[protocol]:
            (reg1, reg2), val_list = INDEX_SET_DB[protocol][(idx1, idx2)]
            prefix1 = idx1 + '_' + reg1 if reg1 else idx1
            prefix2 = idx2 + '_' + reg2 if reg2 else idx2
            code += f"static void index_set_{protocol}_{prefix1}_{prefix2}_arr_init(void);\n"
    return code


def spice_tree_helper_func_declaration_gen():
    """Generate helper function declarations."""
    # No helper functions needed with direct calls approach
    return ""


def spice_tree_func_dec_gen():
    """Generates SPICE tree C local function declaration."""
    code = spice_tree_string_value_func_declaration_gen()
    code += "\nstatic void enum_index_arr_init(void);\n"
    code += '\n' + spice_tree_index_set_arr_func_declaration_gen()
    code += "\nstatic void index_sets_arr_init(void);\n"
    code += spice_tree_helper_func_declaration_gen()
    return code


def enum_dictionary_item_index_get(dict, item):
    """Get index of item in dictionary."""
    index = 0
    for k in dict.keys():
        if k == item:
            return index
        index += 1
    return -1


def spice_tree_string_value_func_impl_gen():
    """Generates enum index string value function implementation."""
    code = ""

    for idx in INDEXES_DB['enum_idxs']:
        count = 0
        idx_name = idx[0]
        idx_reg = idx[1]
        idx_num = len(INDEXES_DB['enum_idxs'][idx])
        arr_prefix = idx_reg + "_" + idx_name.lower() if idx_reg else idx_name.lower()

        code += f"static void {arr_prefix}_arr_init(void)\n{{\n"
        code += f"    memset(&{arr_prefix}_data_arr_g, 0, sizeof(index_type_data_t) * {idx_num});\n\n"

        for k, v in INDEXES_DB['enum_idxs'][idx].items():
            code += f"    {arr_prefix}_data_arr_g[{count}].field_value = {k};\n"
            code += f'    sprintf({arr_prefix}_data_arr_g[{count}].field_name, "{v}");\n'
            count += 1

        code += "\n}\n\n"

    return code


def spice_tree_index_set_arr_func_impl_gen():
    """Generate index set array init functions."""
    code = ""

    for protocol in INDEX_SET_DB:
        for idx1, idx2 in INDEX_SET_DB[protocol]:
            (reg1, reg2), val_list = INDEX_SET_DB[protocol][(idx1, idx2)]
            prefix1 = idx1 + '_' + reg1 if reg1 else idx1
            prefix2 = idx2 + '_' + reg2 if reg2 else idx2

            code += f"static void index_set_{protocol}_{prefix1}_{prefix2}_arr_init(void)\n{{\n"
            code += f"    memset(&index_set_{protocol}_{prefix1}_{prefix2}_arr_g, 0, sizeof(index_set_{protocol}_{prefix1}_{prefix2}_arr_g));\n\n"

            for v1, v2 in val_list:
                if (idx1, reg1) in INDEXES_DB['enum_idxs']:
                    dict1 = INDEXES_DB['enum_idxs'][(idx1, reg1)]
                    pos1 = enum_dictionary_item_index_get(dict1, v1)
                elif idx1 in INDEXES_DB['seq_idxs']:
                    pos1 = v1
                else:
                    continue

                if (idx2, reg2) in INDEXES_DB['enum_idxs']:
                    dict2 = INDEXES_DB['enum_idxs'][(idx2, reg2)]
                    pos2 = enum_dictionary_item_index_get(dict2, v2)
                elif idx2 in INDEXES_DB['seq_idxs']:
                    pos2 = v2
                else:
                    continue

                code += f"    index_set_{protocol}_{prefix1}_{prefix2}_arr_g[{pos1}][{pos2}] = 1;\n"

            code += "}\n\n"

    return code


def spice_tree_enum_index_arr_init_func_impl_gen():
    """Generate enum_index_arr_init function."""
    code = "static void enum_index_arr_init(void)\n{\n"

    for idx in INDEXES_DB['enum_idxs']:
        idx_name = idx[0]
        idx_reg = idx[1]
        arr_prefix = idx_reg + '_' + idx_name.lower() if idx_reg else idx_name.lower()
        code += f"    {arr_prefix}_arr_init();\n"

    return code + "\n}\n\n"


def spice_tree_index_sets_arr_init_func_impl_gen():
    """Generate index_sets_arr_init function."""
    code = "static void index_sets_arr_init(void)\n{\n"

    for protocol in INDEX_SET_DB:
        for idx1, idx2 in INDEX_SET_DB[protocol]:
            (reg1, reg2), val_list = INDEX_SET_DB[protocol][(idx1, idx2)]
            prefix1 = idx1 + '_' + reg1 if reg1 else idx1
            prefix2 = idx2 + '_' + reg2 if reg2 else idx2
            code += f"    index_set_{protocol}_{prefix1}_{prefix2}_arr_init();\n"

    return code + "\n}\n\n"


def spice_tree_helper_func_impl_gen():
    """Generate helper functions implementation."""
    # No helper functions needed with direct calls approach
    return ""


def spice_tree_index_value_get_func_gen():
    """Generate index value get function."""
    code = """uint16_t sx_spice_tree_index_value_get(index_type_e index)
{
    return indexes_arr_g[index];
}

"""
    return code


def spice_tree_module_init_func_gen():
    """Generate init/deinit functions."""
    code = """int sx_spice_tree_init(void)
{
    sxd_log_info(SXD_DEV_NONE, "sx_spice_tree_init\\n");

    enum_index_arr_init();
    index_sets_arr_init();

    root_p = debugfs_create_dir("nvswitch", NULL);
    if (!root_p) {
        goto allocation_failed;
    }

    return 0;

allocation_failed:
    debugfs_remove_recursive(root_p);
    return -ENOMEM;
}

void sx_spice_tree_deinit(void)
{
    sxd_log_info(SXD_DEV_NONE, "In sx_spice_tree_deinit function\\n");
    debugfs_remove_recursive(root_p);
}

"""
    return code


def spice_tree_create_app_dir_impl_gen(tree_name):
    """Generate app directory creation function."""
    code = f"""int sx_spice_tree_{tree_name}_app_dir_create(void)
{{
    {tree_name}_p = debugfs_create_dir("{tree_name}", root_p);
    if (!{tree_name}_p) {{
        goto allocation_failed;
    }}

    return 0;

allocation_failed:
    return -ENOMEM;
}}

"""
    return code


def spice_tree_local_idxs_list_populate(tree, local_idxs_list):
    """Populate local index list from tree (preserves order, avoids duplicates)."""
    for idx in tree.iter('seq_index'):
        name = idx.attrib['name']
        if name not in local_idxs_list:
            local_idxs_list.append(name)

    for idx in tree.iter('enum_index'):
        reg_field = idx.attrib.get('reg')
        if reg_field:
            name = idx.attrib['reg'] + '_' + idx.attrib['name']
        else:
            name = idx.attrib['name']
        if name not in local_idxs_list:
            local_idxs_list.append(name)


def spice_tree_debugfs_local_dir_pointer_gen(local_idxs_list):
    """Generate local directory pointer declarations."""
    code = "    struct dentry * asic_p;\n"
    for idx in local_idxs_list:
        code += f"    struct dentry * {idx.lower()}_p;\n"
    return code


def spice_tree_dir_iterator_dec_gen(local_idxs_list):
    """Generate directory iterator declarations."""
    code = ""
    for idx in local_idxs_list:
        code += f"    int {idx.lower()}_it = 0;\n"
    return code


def hidden_index_code_generate(node, nest_level):
    """Generate hidden index initialization code."""
    nest = "    " * nest_level
    name = node.get('name').upper()
    value = node.get('value')
    code = f"{nest}indexes_arr_g[{name}] = {value};\n"
    return code


def spice_tree_create_impl_gen_db(tree_name, protocol, tree_node):
    """Generate tree creation function using database approach."""
    local_idxs_list = []
    spice_tree_local_idxs_list_populate(tree_node, local_idxs_list)

    dec_code = spice_tree_debugfs_local_dir_pointer_gen(local_idxs_list)
    iter_dec = spice_tree_dir_iterator_dec_gen(local_idxs_list)
    if iter_dec:
        dec_code += '\n' + iter_dec

    # Generate hidden index initialization code
    hidden_code = ""
    for child in tree_node:
        if child.tag == 'hidden_index':
            hidden_code += hidden_index_code_generate(child, 1)

    # Generate the main tree creation logic - direct register calls for tree root
    tree_code = "    /* Create ASIC-level registers */\n"
    tree_code += generate_direct_reg_calls(tree_node, "asic_p", "    ", tree_name, protocol)

    # Generate loops for each top-level index
    for child in tree_node:
        if child.tag == 'seq_index':
            tree_code += generate_index_loop_code(tree_name, protocol, child, tree_node, 1)
        elif child.tag == 'enum_index':
            tree_code += generate_enum_index_loop_code(tree_name, protocol, child, tree_node, 1)

    # Build the full function
    func_code = f"""int sx_spice_tree_{tree_name}_{protocol}_create(dev_private_data_t *dev_data)
{{
    int err;
    char dname[60] = {{ 0 }};

{dec_code}
    sxd_log_info(dev_data->dev, "sx_spice_tree_{tree_name}_{protocol}_create\\n");

    snprintf(dname, sizeof(dname), "ASIC.%d", dev_data->dev->device_id);
    asic_p = debugfs_create_dir(dname, {tree_name}_p);
    if (!asic_p) {{
        goto allocation_failed;
    }}

    dev_data->{tree_name}_root_inode_p = asic_p;

{hidden_code}{tree_code}
    return 0;

allocation_failed:
    debugfs_remove_recursive(asic_p);
    return -ENOMEM;
}}

"""

    return func_code


def split_long_line(text, indent, max_len=120):
    """Split text at || or && boundaries to keep lines under max_len."""
    if len(indent + text) <= max_len:
        return text
    result = []
    while len(indent + text) > max_len:
        cut = max_len - len(indent)
        # Prefer breaking after || or &&
        best = text.rfind(' || ', 0, cut)
        if best == -1:
            best = text.rfind(' && ', 0, cut)
        if best != -1:
            cut = best + 4  # Include the operator
        result.append(text[:cut].rstrip())
        text = text[cut:].lstrip()
    if text:
        result.append(text)
    return ("\n" + indent).join(result)


def generate_direct_reg_calls(node, parent_dir_var, nest, tree_name, protocol):
    """Generate direct register creation calls for all <reg> children of a node.

    Args:
        node: XML node whose <reg> children to process
        parent_dir_var: Variable name of parent directory (e.g., "sensor_index_p")
        nest: Indentation string
        tree_name: Name of the tree (e.g., "ber")
        protocol: Protocol name (e.g., "eth")

    Returns:
        Generated C code string for direct register calls
    """
    code = ""

    for child in node:
        if child.tag == 'reg':
            reg_name = child.get('name')
            mode_attr = child.get('loading_mode', '')
            mode_flags = get_mode_flag_from_attr(mode_attr)
            filter_cond = parse_loading_mode_filters(child, tree_name, protocol)

            func_name = f"sx_spice_access_reg_{reg_name.lower()}_file_create"

            # Build mode condition
            mode_cond = get_mode_flag_string(mode_flags)

            # Generate the call with conditions
            if filter_cond:
                # Has loading mode filters - need conditional creation
                # In default mode, always create. In reduced mode, apply filters.
                inner_indent = nest + "        "
                split_cond = split_long_line(filter_cond, inner_indent)
                code += f"""{nest}if ((loading_mode == SPICE_LOADING_MODE_DEFAULT) ||
{nest}    ((loading_mode == SPICE_LOADING_MODE_REDUCED) && ({split_cond}))) {{
{nest}    err = {func_name}({parent_dir_var}, dev_data);
{nest}    if (err) {{
{nest}        goto allocation_failed;
{nest}    }}
{nest}}}
"""
            else:
                # No filters - simple mode check (or no check if all modes)
                if mode_cond:
                    code += f"""{nest}if {mode_cond} {{
{nest}    err = {func_name}({parent_dir_var}, dev_data);
{nest}    if (err) {{
{nest}        goto allocation_failed;
{nest}    }}
{nest}}}
"""
                else:
                    # All modes - no condition needed
                    code += f"""{nest}err = {func_name}({parent_dir_var}, dev_data);
{nest}if (err) {{
{nest}    goto allocation_failed;
{nest}}}
"""

    return code


def get_mode_flag_string(mode_flags):
    """Convert mode flags integer to C condition string using direct comparisons."""
    modes = []
    if mode_flags & 0x1:  # MODE_DEFAULT
        modes.append("loading_mode == SPICE_LOADING_MODE_DEFAULT")
    if mode_flags & 0x2:  # MODE_REDUCED
        modes.append("loading_mode == SPICE_LOADING_MODE_REDUCED")
    if mode_flags & 0x4:  # MODE_PERF_CNTR
        modes.append("loading_mode == SPICE_LOADING_MODE_PERF_CNTR_ONLY")

    if not modes or mode_flags == 0x7:  # All modes - no check needed
        return None

    if len(modes) == 1:
        return f"({modes[0]})"

    return "(" + " || ".join(modes) + ")"


def generate_index_loop_code(tree_name, protocol, node, parent, nest_level):
    """Generate code for a sequence index loop."""
    code = ""
    index_name = node.get('name')
    parent_str = 'asic' if parent.tag == 'tree' else parent.get('name')
    min_val = node.get('min')
    max_val = node.get('max')
    is_system = node.get('system') == 'true'

    # Handle enum index parent
    parent_enum_name = None
    parent_enum_reg = None
    if parent.tag == 'enum_index':
        parent_enum_name = parent.get('name')
        parent_enum_reg = parent.get('reg', '')
        if parent_enum_reg:
            parent_str = parent_enum_reg + '_' + parent_enum_name

    nest = "    " * nest_level
    index_upper = index_name.upper()

    # Check if there's an index_set for this parent/child combination
    index_set_check = None
    if parent_enum_name and protocol in INDEX_SET_DB:
        # Try to find index_set for (parent_enum, seq_index) combination
        for (idx1, idx2), ((reg1, reg2), val_list) in INDEX_SET_DB[protocol].items():
            if idx1 == parent_enum_name and idx2 == index_name:
                parent_prefix = parent_enum_reg + '_' + parent_enum_name if parent_enum_reg else parent_enum_name
                index_set_check = f"index_set_{protocol}_{parent_prefix}_{index_name}_arr_g[{parent_prefix}_it][{index_name}_it]"
                break

    # Generate for loop and directory creation
    if is_system:
        max_expr = f"dev_data->max_{index_name}"
    else:
        max_expr = max_val

    code += f"""
{nest}for ({index_name}_it = {min_val}; {index_name}_it <= {max_expr}; {index_name}_it++) {{"""

    # Add index_set filtering if applicable
    if index_set_check:
        code += f"""
{nest}    if ({index_set_check}) {{"""
        inner_nest = nest + "    "
    else:
        inner_nest = nest

    code += f"""
{inner_nest}    snprintf(dname, sizeof(dname), "{index_name}.%d", {index_name}_it);
{inner_nest}    {index_name}_p = debugfs_create_dir(dname, {parent_str}_p);
{inner_nest}    if (!{index_name}_p) {{
{inner_nest}        goto allocation_failed;
{inner_nest}    }}
{inner_nest}    indexes_arr_g[{index_upper}] = {index_name}_it;
"""

    # Generate direct register calls for this node's <reg> children
    code += generate_direct_reg_calls(node, f"{index_name}_p", inner_nest + "    ", tree_name, protocol)

    # Process children
    for child in node:
        if child.tag == 'seq_index':
            code += generate_index_loop_code(tree_name, protocol, child, node, nest_level + 1 + (1 if index_set_check else 0))
        elif child.tag == 'enum_index':
            code += generate_enum_index_loop_code(tree_name, protocol, child, node, nest_level + 1 + (1 if index_set_check else 0))
        elif child.tag == 'dynamic_tree':
            code += generate_dynamic_tree_code(tree_name, protocol, child, node, nest_level + 1 + (1 if index_set_check else 0))

    # Close index_set if block
    if index_set_check:
        code += f"{nest}    }}\n"

    code += f"{nest}}}\n"
    return code


def generate_enum_index_loop_code(tree_name, protocol, node, parent, nest_level):
    """Generate code for an enum index loop."""
    code = ""
    index_name = node.get('name')
    reg = node.get('reg', '')
    parent_str = 'asic' if parent.tag == 'tree' else parent.get('name')

    # Handle enum index parent
    parent_enum_name = None
    parent_enum_reg = None
    if parent.tag == 'enum_index':
        parent_enum_name = parent.get('name')
        parent_enum_reg = parent.get('reg', '')
        if parent_enum_reg:
            parent_str = parent_enum_reg + '_' + parent_enum_name

    nest = "    " * nest_level
    index_upper = index_name.upper()

    # Get array prefix and size
    arr_prefix = reg + '_' + index_name if reg else index_name
    idx_key = (index_name, reg)
    if idx_key in INDEXES_DB['enum_idxs']:
        dir_num = len(INDEXES_DB['enum_idxs'][idx_key])
    else:
        dir_num = 1

    # Check if there's an index_set for this parent/child combination
    index_set_check = None
    if parent_enum_name and protocol in INDEX_SET_DB:
        # Try to find index_set for (parent, child) combination
        for (idx1, idx2), ((reg1, reg2), val_list) in INDEX_SET_DB[protocol].items():
            if idx1 == parent_enum_name and idx2 == index_name:
                parent_prefix = parent_enum_reg + '_' + parent_enum_name if parent_enum_reg else parent_enum_name
                child_prefix = reg + '_' + index_name if reg else index_name
                index_set_check = f"index_set_{protocol}_{parent_prefix}_{child_prefix}_arr_g[{parent_prefix}_it][{child_prefix}_it]"
                break

    # Generate for loop, snprintf, and directory creation
    code += f"""
{nest}for ({arr_prefix}_it = 0; {arr_prefix}_it < {dir_num}; {arr_prefix}_it++) {{"""

    # Add index_set filtering if applicable
    if index_set_check:
        code += f"""
{nest}    if ({index_set_check}) {{"""
        inner_nest = nest + "    "
    else:
        inner_nest = nest

    code += f"""
{inner_nest}    snprintf(dname, sizeof(dname), "{index_name}.%s.0x%X",
{inner_nest}        {arr_prefix}_data_arr_g[{arr_prefix}_it].field_name,
{inner_nest}        {arr_prefix}_data_arr_g[{arr_prefix}_it].field_value);
{inner_nest}    {arr_prefix}_p = debugfs_create_dir(dname, {parent_str}_p);
{inner_nest}    if (!{arr_prefix}_p) {{
{inner_nest}        goto allocation_failed;
{inner_nest}    }}
{inner_nest}    indexes_arr_g[{index_upper}] = {arr_prefix}_data_arr_g[{arr_prefix}_it].field_value;
"""

    # Generate direct register calls for this node's <reg> children
    code += generate_direct_reg_calls(node, f"{arr_prefix}_p", inner_nest + "    ", tree_name, protocol)

    # Process children
    for child in node:
        if child.tag == 'seq_index':
            code += generate_index_loop_code(tree_name, protocol, child, node, nest_level + 1 + (1 if index_set_check else 0))
        elif child.tag == 'enum_index':
            code += generate_enum_index_loop_code(tree_name, protocol, child, node, nest_level + 1 + (1 if index_set_check else 0))
        elif child.tag == 'dynamic_tree':
            code += generate_dynamic_tree_code(tree_name, protocol, child, node, nest_level + 1 + (1 if index_set_check else 0))

    # Close index_set if block
    if index_set_check:
        code += f"{nest}    }}\n"

    code += f"{nest}}}\n"
    return code


def generate_dynamic_tree_code(tree_name, protocol, node, parent, nest_level):
    """Generate code for dynamic tree (conditional structures)."""
    nest = "    " * nest_level

    code = ""
    first_child = True
    fallback_regs = []

    for child in node:
        if child.tag == 'enum_index' or child.tag == 'seq_index':
            idx_name = child.get('name')
            if child.tag == 'enum_index' and child.get('reg'):
                idx_name = child.get('reg') + '_' + idx_name

            if node.get('type') == 'system':
                condition = f"is_{idx_name}_dir_supported(dev_data)"
            else:
                # Index-based condition
                index_str = node.get('index')
                if parent.tag == 'seq_index':
                    condition = f"is_{idx_name}_dir_supported(dev_data, {index_str}_it)"
                else:
                    condition = f"is_{idx_name}_dir_supported(dev_data)"

            if first_child:
                code += f"{nest}if ({condition}) {{\n"
                first_child = False
            else:
                code += f" else if ({condition}) {{\n"

            # Generate child loop
            if child.tag == 'seq_index':
                code += generate_index_loop_code(tree_name, protocol, child, parent, nest_level + 1)
            else:
                code += generate_enum_index_loop_code(tree_name, protocol, child, parent, nest_level + 1)

            code += f"{nest}}}"

        elif child.tag == 'reg':
            # Fallback case - register created when none of the conditional sub directories are supported
            fallback_regs.append(child)

    # Generate else clause for fallback registers if we had conditional blocks
    if code and not first_child and fallback_regs:
        code += " else {\n"
        # Get parent directory pointer name
        if parent.tag == 'seq_index':
            parent_idx = parent.get('name')
            parent_dir = f"{parent_idx}_p"
        elif parent.tag == 'enum_index':
            parent_idx = parent.get('name')
            if parent.get('reg'):
                parent_idx = parent.get('reg') + '_' + parent_idx
            parent_dir = f"{parent_idx}_p"
        else:
            parent_dir = "root_p"

        for reg in fallback_regs:
            reg_name = reg.get('name')
            mode_attr = reg.get('loading_mode', '')
            mode_flags = get_mode_flag_from_attr(mode_attr)
            mode_cond = get_mode_flag_string(mode_flags)
            func_name = f"sx_spice_access_reg_{reg_name.lower()}_file_create"

            if mode_cond:
                code += f"{nest}    if {mode_cond} {{\n"
                code += f"{nest}        err = {func_name}({parent_dir}, dev_data);\n"
                code += f"{nest}        if (err) {{\n"
                code += f"{nest}            goto allocation_failed;\n"
                code += f"{nest}        }}\n"
                code += f"{nest}    }}\n"
            else:
                code += f"{nest}    err = {func_name}({parent_dir}, dev_data);\n"
                code += f"{nest}    if (err) {{\n"
                code += f"{nest}        goto allocation_failed;\n"
                code += f"{nest}    }}\n"
        code += f"{nest}}}\n"
    elif code and not first_child:
        code += "\n"

    return code


def spice_tree_src_func_impl_iterate():
    """Generate tree creation functions."""
    code = ""

    for tree_name in TREE_DICT.keys():
        code += spice_tree_create_app_dir_impl_gen(tree_name)

        root = TREE_DICT[tree_name]
        for prot_node in root:
            protocol = prot_node.get('name')
            tree_node = prot_node.find('tree')
            if tree_node is not None:
                code += spice_tree_create_impl_gen_db(tree_name, protocol, tree_node)

    return code


def spice_tree_src_func_impl_gen():
    """Generates SPICE tree C function implementation segment."""
    code = ""
    code += spice_tree_index_value_get_func_gen()
    code += spice_tree_string_value_func_impl_gen()
    code += spice_tree_index_set_arr_func_impl_gen()
    code += spice_tree_enum_index_arr_init_func_impl_gen()
    code += spice_tree_index_sets_arr_init_func_impl_gen()
    code += spice_tree_helper_func_impl_gen()
    code += spice_tree_module_init_func_gen()
    code += spice_tree_src_func_impl_iterate()
    return code


def spice_tree_auto_source_gen():
    """Generates SPICE tree source file."""
    local_vars_code = spice_tree_src_local_vars_gen()
    local_func = spice_tree_func_dec_gen()
    impl_func = spice_tree_src_func_impl_gen()
    return SPICE_TREE_C_AUTO_PATTERN % (local_vars_code, local_func, impl_func)


def spice_tree_db_add(tree):
    """Add tree to database."""
    root = tree.getroot()
    tree_name = root.attrib['name'].lower()
    TREE_DICT[tree_name] = tree.getroot()


def __spice_tree_adb_files():
    """
    Collect SPICE ADB source files for tree generation.
    """
    xml_dir = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'adb'))
    adb_files = []

    for root, dirs, files in os.walk(xml_dir):
        for file_name in sorted(files):
            if file_name.endswith('.adb'):
                adb_files.append(os.path.join(root, file_name))

    return sorted(adb_files)


def spice_tree_db_init():
    """
    Initialize SPICE TREE DB
    """
    adb_files = __spice_tree_adb_files()

    for adb_file in adb_files:
        tree = ET.parse(adb_file)
        spice_tree_db_add(tree)


def spice_enum_indexes_db_update(idx_node):
    """Updates INDEXES_DB with enum indexes data."""
    for enum_idx in idx_node.iter('enum_index'):
        enum_idx_dict = {}
        for idx_type in enum_idx.iter('index_type'):
            enum_idx_dict[idx_type.attrib['value']] = idx_type.attrib['name']

        INDEXES_DB['enum_idxs'][(enum_idx.attrib['name'], enum_idx.attrib['reg'])] = enum_idx_dict


def spice_index_set_db_update(protocol, idx_node):
    """Updates INDEX_SET_DB with index set data."""
    for idx_set in idx_node.iter('index_set'):
        idx_set_list = []
        reg1 = idx_set.attrib['reg1']
        reg2 = idx_set.attrib['reg2']
        for idx_val in idx_set.iter('index_set_values'):
            item = (idx_val.attrib['idx1'], idx_val.attrib['idx2'])
            if item not in idx_set_list:  # Maintain uniqueness while preserving order
                idx_set_list.append(item)

        INDEX_SET_DB[protocol][(idx_set.attrib['idx1_name'], idx_set.attrib['idx2_name'])] = ((reg1, reg2), idx_set_list)


def spice_tree_indexes_db_update(tree):
    """Updates INDEXES_DB with sequence and hidden indexes data."""
    for idx in tree.iter('seq_index'):
        INDEXES_DB['seq_idxs'][idx.attrib['name']] = (idx.attrib['min'], idx.attrib['max'])

    for idx in tree.iter('hidden_index'):
        idx_name = idx.attrib['name']
        if idx_name not in INDEXES_DB['hidden_idxs']:
            INDEXES_DB['hidden_idxs'].append(idx_name)


def spice_loading_mode_filters_db_update(tree_name, protocol_name, loading_mode_filters_node):
    """Parse <loading_mode_filters> section and populate LOADING_MODE_FILTERS_DB."""
    filter_dict = {}
    for filter_elem in loading_mode_filters_node.iter('filter'):
        filter_name = filter_elem.get('name', '')
        index_name = filter_elem.get('index', '')
        values_str = filter_elem.get('values', '')
        if filter_name and index_name and values_str:
            values = parse_values_string(values_str)
            filter_dict[filter_name] = (index_name, values)

    LOADING_MODE_FILTERS_DB[(tree_name, protocol_name)] = filter_dict


def spice_indexes_db_init():
    """Initialize indexes databases."""
    for tree_name, root in TREE_DICT.items():
        for protocol in root:
            protocol_name = protocol.attrib['name']
            for child in protocol:
                if child.tag == 'indexes':
                    spice_enum_indexes_db_update(child)
                    spice_index_set_db_update(protocol_name, child)
                elif child.tag == 'loading_mode_filters':
                    spice_loading_mode_filters_db_update(tree_name, protocol_name, child)
                elif child.tag == 'tree':
                    spice_tree_indexes_db_update(child)
                else:
                    raise Exception(f"{child.tag} xml tag is not supported.")
                    return 1

    return 0


def Main():
    try:
        # Initialize XML Tree DB and Indexes DB
        spice_tree_db_init()
        spice_indexes_db_init()

        OUTPUT_DIR = os.path.normpath(os.path.dirname(os.path.abspath(__file__)) + '/../')

        for fileName, command in AUTO_GENERATION_DICT.items():
            fullFileName = OUTPUT_DIR + os.sep + fileName
            with open(fullFileName, "w") as fileDesc:
                code = eval(command)
                fileDesc.write(code)
            license_utils.apply_license_headers(fullFileName, __file__, __spice_tree_adb_files())

            print(f"[{fullFileName}] - created successfully.\n")

        print("License header update finished successfully.\n")

    except SystemExit:
        return 0

    except Exception as exp:
        import traceback
        traceback.print_exc()
        sys.stderr.write(f"-E- {exp}\n")
        return 1
    return 0


if __name__ == "__main__":
    rc = 0
    try:
        rc = Main()
    except SystemExit as e:
        rc = e.code
    except Exception as e:
        sys.stderr.write(f"-E- {e}\n")
        rc = 1
    sys.exit(rc)
