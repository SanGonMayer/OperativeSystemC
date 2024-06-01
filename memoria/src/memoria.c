#include "memoria.h"
#include <commons/collections/dictionary.h>
#include <stdint.h>

static t_memoria* memoria = NULL;

void inicializar_memoria(uint32_t tam_memoria, uint32_t tam_pagina){
    
    if(memoria == NULL){
        memoria = malloc(sizeof(t_memoria));
        memoria->tam_memoria = tam_memoria;
        memoria->tam_pagina = tam_pagina;
        memoria->cantidad_marcos = tam_memoria / tam_pagina;
        memoria->memoria = malloc(tam_memoria);
    }
}






