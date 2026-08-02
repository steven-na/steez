#pragma once

#include "common.h"

typedef struct {
    i32 major, minor, patch;
    char const *string;
    char const *git_commit;
    char const *build_date;
    char const *build_time;
    char const *build_type;
} steez_build_info_t;

char const *steez_version_string(void);
steez_build_info_t steez_build_info(void);
