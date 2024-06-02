#ifndef MEMORIA_H_
#define MEMORIA_H_
#include "utils/instrucciones.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <utils/procesos.h>
#include <utils/buffer.h>
#include <commons/log.h>

typedef struct{
    void* espacio_contiguo;
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    uint32_t cantidad_marcos;
    t_dictionary* tablas_de_paginas;
} t_memoria;

void inicializar_memoria(uint32_t tam_memoria, uint32_t tam_pagina);
t_list* inicializar_tabla_paginas(uint32_t pid);

#endif