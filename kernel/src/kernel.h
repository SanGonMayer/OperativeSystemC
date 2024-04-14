#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <utils/client.h>

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log* logger); //para leer linea a linea desde la consola

#endif