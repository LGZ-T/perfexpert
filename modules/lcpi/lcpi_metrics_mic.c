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

/* Modules headers */
#include "lcpi.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"

int metrics_mic(void) {
    char s[MAX_LCPI];

    OUTPUT_VERBOSE((8, "Setting events for the MIC"));
    /* Set the events on the measurement module */
    USE_EVENT("L2_DATA_READ_MISS_MEM_FILL");   // 0
    USE_EVENT("BRANCHES");                     // 1
    USE_EVENT("L2_VICTIM_REQ_WITH_DATA");      // 0
    USE_EVENT("BRANCHES_MISPREDICTED");        // 1
    USE_EVENT("L2_DATA_WRITE_MISS_MEM_FILL");  // 0
    USE_EVENT("CPU_CLK_UNHALTED");             // 1
    USE_EVENT("SNP_HITM_L2");                  // 0
    USE_EVENT("CODE_READ");                    // 1
    USE_EVENT("HWP_L2MISS");                   // 0
    USE_EVENT("CODE_CACHE_MISS");              // 1
    USE_EVENT("DATA_READ_OR_WRITE");           // 0
    USE_EVENT("CODE_PAGE_WALK");               // 1
    USE_EVENT("DATA_READ_MISS_OR_WRITE_MISS"); // 0
    USE_EVENT("DATA_PAGE_WALK");               // 1
    USE_EVENT("INSTRUCTIONS_EXECUTED");        // 0
    USE_EVENT("VPU_ELEMENTS_ACTIVE");          // 1
    USE_EVENT("VPU_INSTRUCTIONS_EXECUTED");    // 0
    USE_EVENT("LONG_DATA_PAGE_WALK");          // 1

    OUTPUT_VERBOSE((8, "Events set for the MIC"));
    /* Set the profile total cycles and total instructions counters */

    my_module_globals.measurement->total_cycles_counter = "CPU_CLK_UNHALTED";
    my_module_globals.measurement->total_inst_counter = "INSTRUCTIONS_EXECUTED";

    /* ratio.floating_point */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("ratio.VPU_instructions",
        "VPU_INSTRUCTIONS_EXECUTED / INSTRUCTIONS_EXECUTED")) {
        return PERFEXPERT_ERROR;
    }

    /* ratio.data_accesses */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("ratio.data_accesses",
        "(DATA_READ_OR_WRITE + DATA_READ_MISS_OR_WRITE_MISS) / "
        "INSTRUCTIONS_EXECUTED")) {
        return PERFEXPERT_ERROR;
    }

    /* overall */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("overall",
        "CPU_CLK_UNHALTED / INSTRUCTIONS_EXECUTED / CPI_threshold")) {
        return PERFEXPERT_ERROR;
    }

    /* vectorization intensity */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("vectorization_level.overall",
        "VPU_ELEMENTS_ACTIVE / VPU_INSTRUCTIONS_EXECUTED")) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.overall */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.overall",
        "((DATA_READ_OR_WRITE * L1_dlat) + ((DATA_READ_MISS_OR_WRITE_MISS - "
        "L2_DATA_READ_MISS_CACHE_FILL - L2_DATA_WRITE_MISS_MEM_FILL) * L2d_lat)"
        " + ((L2_DATA_READ_MISS_MEM_FILL + L2_DATA_WRITE_MISS_MEM_FILL) * "
        "mem_lat)) / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L1_cache_hits */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L1_cache_hits",
        "(DATA_READ_OR_WRITE * L1_dlat) / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L2_cache_hits */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L2_cache_hits",
        "((DATA_READ_MISS_OR_WRITE_MISS - L2_DATA_READ_MISS_CACHE_FILL - "
        "L2_DATA_WRITE_MISS_MEM_FILL) * L2_dlat) / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.LLC_misses */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.LLC_misses",
        "((L2_DATA_READ_MISS_MEM_FILL + L2_DATA_WRITE_MISS_MEM_FILL) * "
            "mem_lat) / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* memory_bandwidth.overall */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("memory_bandwidth.overall",
        "(((L2_DATA_READ_MISS_MEM_FILL + L2_DATA_WRITE_MISS_MEM_FILL + "
            "HWP_L2MISS + L2_VICTIM_REQ_WITH_DATA + SNP_HITM_L2) * 64) / "
            "CPU_CLK_UNHALTED) * CPU_freq * 100 / mem_bandwidth")) {
        return PERFEXPERT_ERROR;
    }

    /* memory_bandwidth.read */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("memory_bandwidth.read_(%)",
        "((L2_DATA_READ_MISS_MEM_FILL + L2_DATA_WRITE_MISS_MEM_FILL + HWP_L2MISS"
        ") * 64) / CPU_CLK_UNHALTED * CPU_freq * 100 / mem_bandwidth")) {
        return PERFEXPERT_ERROR;
    }

    /* memory_bandwidth.write */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("memory_bandwidth.write_(%)",
        "((L2_VICTIM_REQ_WITH_DATA + SNP_HITM_L2) * 64) / CPU_CLK_UNHALTED * "
        "CPU_freq * 100 / mem_bandwidth")) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.overall */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.overall",
        "((CODE_READ * L1_ilat) + (CODE_CACHE_MISS * mem_lat)) / "
        "VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L1_hits */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L1_hits",
        "(CODE_READ * L1_ilat) / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L1_misses */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L1_misses",
        "(CODE_CACHE_MISS * mem_lat) / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* data_TLB.overall */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_TLB.overall",
        "(DATA_PAGE_WALK + LONG_DATA_PAGE_WALK) * TLB_lat / "
        "VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_TLB.overall */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_TLB.overall",
        "CODE_PAGE_WALK * TLB_lat / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.overall */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.overall",
        "(((BRANCHES - BRANCHES_MISPREDICTED) * BR_lat) + "
            "(BRANCHES_MISPREDICTED * BR_miss_lat)) / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.correctly_predicted */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.correctly_predicted",
        "(BRANCHES - BRANCHES_MISPREDICTED) * BR_lat / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.mispredicted */
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.mispredicted",
        "BRANCHES_MISPREDICTED * BR_miss_lat / VPU_ELEMENTS_ACTIVE")) {
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

// EOF
