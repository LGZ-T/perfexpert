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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */

/* Module headers */
#include "modules/perfexpert_module_base.h"
#include "modules/perfexpert_module_measurement.h"
#include "macvec_module.h"
#include "macvec.h"
#include "macvec_types.h"
#include "macvec_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_cpuinfo.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_string.h"

/* Global variable to define the module itself */
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));
    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    int comp_loaded = PERFEXPERT_FALSE;
    /* Initialize list of events */
    perfexpert_list_construct(&(my_module_globals.profiles));
//    my_module_globals.architecture = NULL;

    // Check if at least one of HPCToolkit or VTune is loaded 
    if ((PERFEXPERT_FALSE == perfexpert_module_available("hpctoolkit")) &&
        (PERFEXPERT_FALSE == perfexpert_module_available("vtune"))) {
        OUTPUT(("%s", _RED("Neither HPCToolkit nor VTune module loaded")));

        // Default to HPCToolkit
        OUTPUT(("%s", _RED("PerfExpert will try to load HPCToolkit module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_load("hpctoolkit")) {
            OUTPUT(("%s", _ERROR("while loading HPCToolkit module")));
            return PERFEXPERT_ERROR;
        }

        // Get the module address
        if (NULL == (my_module_globals.measurement =
                (perfexpert_module_measurement_t *)
                perfexpert_module_get("hpctoolkit"))) {
            OUTPUT(("%s", _ERROR("unable to get measurements module")));
            return PERFEXPERT_SUCCESS;
        }
    }

    // Should we use HPCToolkit?
    if (PERFEXPERT_TRUE == perfexpert_module_available("hpctoolkit")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use HPCToolkit as measurement module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macvec",
            PERFEXPERT_PHASE_ANALYZE, "hpctoolkit", PERFEXPERT_PHASE_MEASURE,
            PERFEXPERT_MODULE_BEFORE)) {
            OUTPUT(("%s", _ERROR("required module/phase not available")));
            return PERFEXPERT_ERROR;
        }
        if (NULL == (my_module_globals.measurement =
            (perfexpert_module_measurement_t *)
            perfexpert_module_get("hpctoolkit"))) {
            OUTPUT(("%s", _ERROR("required module not available")));
            return PERFEXPERT_ERROR;
        }
    }

    // Should we generate metrics to use with Vtune? 
    if (PERFEXPERT_TRUE == perfexpert_module_available("vtune")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use VTune as measurement module")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macvec",
            PERFEXPERT_PHASE_ANALYZE, "vtune", PERFEXPERT_PHASE_MEASURE,
            PERFEXPERT_MODULE_BEFORE)) {
            OUTPUT(("%s", _ERROR("required module/phase not available")));
            return PERFEXPERT_ERROR;
        }
        if (NULL == (my_module_globals.measurement =
            (perfexpert_module_measurement_t *)
            perfexpert_module_get("vtune"))) {
            OUTPUT(("%s", _ERROR("required module not available")));
            return PERFEXPERT_ERROR;
        }
    }

    // Triple check: at least one measurement module should be available
    if (NULL == my_module_globals.measurement) {
        OUTPUT(("%s", _ERROR("No measurement module loaded")));
        return PERFEXPERT_ERROR;
    }
       
    /* Module pre-requisites */
/*  if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macvec",
        PERFEXPERT_PHASE_ANALYZE, NULL, PERFEXPERT_PHASE_MEASURE,
        PERFEXPERT_MODULE_AFTER)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available 1")));
        return PERFEXPERT_ERROR;
    }   
*/    /*if (PERFEXPERT_SUCCESS != perfexpert_module_requires("macvec",
        PERFEXPERT_PHASE_ANALYZE, NULL, PERFEXPERT_PHASE_COMPILE,
        PERFEXPERT_MODULE_BEFORE)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available 2")));
        return PERFEXPERT_ERROR;
    } 
  */  
    /* Parse module options */
    if (PERFEXPERT_SUCCESS != parse_module_args(myself_module.argc,
                myself_module.argv)) {
        OUTPUT(("%s", _ERROR("parsing module arguments")));
        return PERFEXPERT_ERROR;
    }

//    if (my_module_globals.report_file == NULL) {
        if (PERFEXPERT_TRUE == perfexpert_module_available("make")) {
            myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("make");
            if (NULL != myself_module.measurement) {
                comp_loaded = PERFEXPERT_TRUE;
            }        
        }
        if (PERFEXPERT_TRUE == perfexpert_module_available("icc")) {
            myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("icc");
            if (NULL != myself_module.measurement) {
                comp_loaded = PERFEXPERT_TRUE;
            }        
        }
        if (PERFEXPERT_TRUE == perfexpert_module_available("gcc")) {
            myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("gcc");
            if (NULL != myself_module.measurement) {
                comp_loaded = PERFEXPERT_TRUE;
            }        
        }
//    }

    if (!comp_loaded) {
        OUTPUT(("%s", _ERROR(" this module needs a compilation module to be defined")));
        return PERFEXPERT_ERROR;
    }

    /* If the architecture was not set, we should try to identify it... */
    /*  
    if (NULL == my_module_globals.architecture) {
        char *error = NULL, sql[MAX_BUFFER_SIZE];
        int family = perfexpert_cpuinfo_get_family();
        int model  = perfexpert_cpuinfo_get_model();

        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "SELECT description FROM arch_processor WHERE family=%d "
            "AND model=%d;", family, model);
        OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
            perfexpert_database_get_string,
            (void *)&my_module_globals.architecture, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        if (NULL == my_module_globals.architecture) {
            OUTPUT_VERBOSE((1, "Unknown architecture, using PAPI defaults"));
            my_module_globals.architecture = "unknown";
        }

        OUTPUT_VERBOSE((1, "Architecture not set but it looks like [%s]",
            my_module_globals.architecture));
    }

    // Initialize the measurements module before using it
    if (PERFEXPERT_MODULE_LOADED == my_module_globals.measurement->status) {
        if (PERFEXPERT_SUCCESS != my_module_globals.measurement->init()) {
            OUTPUT(("%s [%s]", _ERROR("error initializing module"),
                my_module_globals.measurement->name));
            return PERFEXPERT_ERROR;
        }
    }

    OUTPUT(("%s", _YELLOW("Setting performance events")));

    // Jaketown (or SandyBridgeEP)
    if (0 == strcmp("jaketown",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != counters_jaketown()) {
            OUTPUT(("%s", _ERROR("setting counters (Jaketown)")));
            return PERFEXPERT_ERROR;
        }
    }
    // MIC (or KnightsCorner)
    else if (0 == strcmp("mic",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != metrics_mic()) {
            OUTPUT(("%s", _ERROR("setting counters (MIC)")));
            return PERFEXPERT_ERROR;
        }
    }
    // Unknown 
    else if (0 == strcmp("unknown",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        if (PERFEXPERT_SUCCESS != counters_papi()) {
            OUTPUT(("%s", _ERROR("setting counters (PAPI)")));
            return PERFEXPERT_ERROR;
        }
    }
    // If not any of the above, I'm sorry...
    else {
        OUTPUT(("%s (%s)", _ERROR("setting counters"),
            my_module_globals.architecture));
        return PERFEXPERT_ERROR;
    }
    */

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

static int cmp_relevance(const macvec_hotspot_t **a,
        const macvec_hotspot_t **b) {
    if ((*a)->importance > (*b)->importance) {
        return -1;
    }

    if ((*a)->importance < (*b)->importance) {
        return +1;
    }

    return 0;
}

int filter_and_sort_hotspots(perfexpert_list_t* hotspots, double threshold) {
    if (0 == perfexpert_list_get_size(hotspots)) {
        return PERFEXPERT_SUCCESS;
    }

    perfexpert_list_item_t **items, *item;
    items = (perfexpert_list_item_t**)malloc(sizeof(perfexpert_list_item_t*) *
            perfexpert_list_get_size(hotspots));

    if (NULL == items) {
        return PERFEXPERT_ERROR;
    }

    int index = 0;
    while (NULL != (item = perfexpert_list_get_first(hotspots))) {
        perfexpert_list_remove_item(hotspots, item);
        macvec_hotspot_t* hotspot = (macvec_hotspot_t*) item;
        if (hotspot->importance >= threshold &&
                hotspot->type == PERFEXPERT_HOTSPOT_LOOP) {
            items[index++] = item;
        }
    }

    qsort(items, index, sizeof(perfexpert_list_item_t*),
            (int(*)(const void*, const void*)) cmp_relevance);

    int n;
    for (n = 0; n < index; n++) {
        // Log hotspots.
        macvec_hotspot_t* hotspot = (macvec_hotspot_t*) items[n];
        OUTPUT_VERBOSE((4, "Found hotspot %s at %s:%ld [imp: %.2lf]",
                hotspot->name, hotspot->file, hotspot->line,
                hotspot->importance));
        perfexpert_list_append(hotspots, items[n]);
    }

    free(items);

    return PERFEXPERT_SUCCESS;
}
/* module_analyze */
int module_analyze(void) {
    macvec_profile_t *p = NULL;
    macvec_hotspot_t *h = NULL;

    OUTPUT(("%s", _YELLOW("Analyzing measurements")));
/*  
    // Import measurements to memory
    if (PERFEXPERT_SUCCESS != database_import(&(my_module_globals.profiles),
        my_module_globals.measurement->name)) {
        OUTPUT(("%s", _ERROR("unable to import profiles")));
        return PERFEXPERT_ERROR;
    }
*/
    char *cflags = getenv("CFLAGS");
    char *newenv;
            PERFEXPERT_ALLOC(char, newenv, strlen(cflags) + 30);
            snprintf(newenv, strlen(cflags) + 30,
                     "-vec-report=6 ", cflags);
            if (setenv("CFLAGS", newenv, 1) < 0){
                return PERFEXPERT_ERROR;
            }
            PERFEXPERT_DEALLOC(newenv);
    /*   
            OUTPUT(("2"));
            char *cxxflags = getenv("CXXFLAGS");
            PERFEXPERT_ALLOC(char, newenv, strlen(cxxflags) + strlen(my_module_globals.report_file) + 30);
            snprintf(newenv, strlen(cxxflags) + strlen(my_module_globals.report_file) + 30,
                     "-vec-report=6 -opt-report=%s %s", my_module_globals.report_file, cxxflags); 
            if (setenv("CXXFLAGS", newenv, 1) < 0){
                return PERFEXPERT_ERROR;
            }
            PERFEXPERT_DEALLOC(newenv);
     
            OUTPUT(("3"));
            char *fcflags = getenv("FCFLAGS");
            PERFEXPERT_ALLOC(char, newenv, strlen(fcflags)+strlen(my_module_globals.report_file) + 30);
            snprintf(newenv, strlen(fcflags) + strlen(my_module_globals.report_file) + 30,
                     "-vec-report=6 -opt-report=%s %s", my_module_globals.report_file, fcflags); 
            if (setenv("FCFLAGS", newenv, 1) < 0){
                return PERFEXPERT_ERROR;
            }
            PERFEXPERT_DEALLOC(newenv);
*/
            myself_module.measurement->compile();
   //     }
   // }
    /* For each profile... */
   
    OUTPUT(("Code recompiled")); 
    perfexpert_list_t files;
    perfexpert_list_construct(&files);
    OUTPUT(("Going to import the list of files"));
    list_files_hotspots(&files);
    OUTPUT(("Files imported"));
    
    double threshold = globals.threshold;

    char_t *filename;
    perfexpert_list_for (filename, &files, char_t) {        
        macvec_profile_t* profile;
        database_import(&(my_module_globals.profiles), filename->name);
        perfexpert_list_for(profile, &(my_module_globals.profiles),
                macvec_profile_t) {
            perfexpert_list_t* hotspots = &(profile->hotspots);
            filter_and_sort_hotspots(hotspots, threshold);
            process_hotspots(hotspots); //, my_module_globals.report_file);
        }   
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
