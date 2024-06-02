#include "memoria.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <stdint.h>

static t_memoria* memoria = NULL;
static t_list* tabla_paginas = NULL;

void inicializar_memoria(uint32_t tam_memoria, uint32_t tam_pagina){
    
    if(memoria == NULL){
        memoria = malloc(sizeof(t_memoria));
        memoria->tam_memoria = tam_memoria;
        memoria->tam_pagina = tam_pagina;
        memoria->cantidad_marcos = tam_memoria / tam_pagina;
        memoria->espacio_contiguo = malloc(tam_memoria);
        memoria->tablas_de_paginas = dictionary_create();
    }
}

t_list* inicializar_tabla_paginas(uint32_t pid){
    t_list* tabla_paginas = list_create();
    char* pid_string = string_itoa(pid);
    dictionary_put(memoria->tablas_de_paginas, pid_string, tabla_paginas);

    return tabla_paginas;
}

void liberar_tabla_paginas(uint32_t pid){
    char* pid_string = string_itoa(pid);

    // liberar bitmap de marcos usados
    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
    list_destroy(tabla_paginas);
    dictionary_remove(memoria->tablas_de_paginas, pid_string);
}

int leer_nro_marco(uint32_t pid, uint32_t nro_pagina){
    char* pid_string = string_itoa(pid);
    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
    return (int) list_get(tabla_paginas, nro_pagina);
}


void ajustar_tamanio_proceso(uint32_t pid, uint32_t nuevo_tamanio){
    char* pid_string = string_itoa(pid);
    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
    uint32_t tamanio_actual = list_size(tabla_paginas);
    uint32_t tamanio_nuevo = nuevo_tamanio / memoria->tam_pagina;

    if(tamanio_nuevo > tamanio_actual){
        // Ampliación de un proceso
        uint32_t paginas_a_agregar = tamanio_nuevo - tamanio_actual;
        for(int i = 0; i < paginas_a_agregar; i++){
            int pagina_vacia = -1;
            list_add(tabla_paginas, (void*) pagina_vacia);
        }
    } else {
        // Reducción de un proceso
        uint32_t paginas_a_eliminar = tamanio_actual - tamanio_nuevo;
        // liberar bitmap de marcos usados
        for(int i = 0; i < paginas_a_eliminar; i++){
            list_remove(tabla_paginas, list_size(tabla_paginas) - 1);
        }
    }
}

void leer_de_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* buffer){
    void* direccion_fisica_real = memoria->espacio_contiguo + direccion_fisica;
    memcpy(buffer, direccion_fisica_real, tamanio);
}

void escribir_en_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* buffer){
    void* direccion_fisica_real = memoria->espacio_contiguo + direccion_fisica;
    memcpy(direccion_fisica_real, buffer, tamanio);
}











