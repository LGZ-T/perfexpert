/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 * 
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 * 
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef TOOLS_MACPO_INST_INCLUDE_VECTOR_STRIDES_H_
#define TOOLS_MACPO_INST_INCLUDE_VECTOR_STRIDES_H_

#include <rose.h>

#include "inst_defs.h"
#include "traversal.h"

class vector_strides_t : public traversal_t {
 public:
    explicit vector_strides_t(const du_table_t& def_table) :
        traversal_t(def_table) {}

 private:
    bool instrument_loop(loop_info_t& loop_info);
    bool contains_non_linear_reference(const reference_list_t& reference_list);
};

#endif  // TOOLS_MACPO_INST_INCLUDE_VECTOR_STRIDES_H_
