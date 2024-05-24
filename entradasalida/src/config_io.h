#ifndef CONFIG_IO_H_
#define CONFIG_IO_H_

#include "config_type.h"
#include <commons/config.h>

ConfiguracionIO* leer_configuracion(t_config* config);
void configuracionIO_destroy(ConfiguracionIO* config);

#endif
