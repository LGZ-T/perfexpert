/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MODULE_LCPI_OUTPUT_H_
#define PERFEXPERT_MODULE_LCPI_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Modules headers */
#include "lcpi_types.h"

static int output_profile(lcpi_hotspot_t *h, FILE *report_FP, const int scale, const int task, const int thread);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_LCPI_OUTPUT_H_ */
