#ifndef MEMORIA_H_
#define MEMORIA_H_
#include "utils/instrucciones.h"
#include <utils/procesos.h>
#include <utils/buffer.h>
#include <commons/log.h>

typedef struct{
    void* memoria;
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    uint32_t cantidad_marcos;
} t_memoria;

typedef struct{
    

}t_tabla_paginas;

void inicializar_memoria(uint32_t tam_memoria, uint32_t tam_pagina);

#endif