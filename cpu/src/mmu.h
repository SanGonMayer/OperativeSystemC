#ifndef MMU_H
#define MMU_H

#include <stdint.h>
void set_tamanio_pagina(int tamanio);
int traducir_a_direccion_fisica(uint32_t pid, int direccion_logica);
int pedir_marco_a_memoria(uint32_t pid, int nro_pagina);

#endif