#include "version.h"
#include "version_gen.h"

char const *steez_version_string(void) {
    return STEEZ_VERSION_STRING;
}

steez_build_info_t steez_build_info(void) {
    return (steez_build_info_t){
        .major      =  STEEZ_VERSION_MAJOR,
        .minor      =  STEEZ_VERSION_MINOR,
        .patch      =  STEEZ_VERSION_PATCH,
        .string     = STEEZ_VERSION_STRING,
        .git_commit =     STEEZ_GIT_COMMIT,
        .build_date =     STEEZ_BUILD_DATE,
        .build_time =     STEEZ_BUILD_TIME,
        .build_type =     STEEZ_BUILD_TYPE,
    };
}
