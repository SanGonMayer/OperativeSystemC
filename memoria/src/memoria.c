#include <memoria.h>

t_log* logger;

void iterator(char* value) {
    log_info(logger,"%s", value);
}


