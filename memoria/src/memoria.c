#include "memoria.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdint.h>
#include <string.h>
#include "global_memoria.h"
#include <stdlib.h>

static t_memoria* memoria = NULL;

int calcular_bytes_necesarios(uint32_t cantidad_marcos){
    int bytes = cantidad_marcos / 8;

    if(cantidad_marcos % 8 != 0){
        bytes++;
    }
    return bytes;
}

void inicializar_memoria(uint32_t tam_memoria, uint32_t tam_pagina){
    
    if(memoria == NULL){
        memoria = malloc(sizeof(t_memoria));
        memoria->tam_memoria = tam_memoria;
        memoria->tam_pagina = tam_pagina;
        memoria->cantidad_marcos = tam_memoria / tam_pagina;
        memoria->espacio_contiguo = malloc(tam_memoria);
        memoria->tablas_de_paginas = dictionary_create();

        memoria->bitmap = malloc(calcular_bytes_necesarios(memoria->cantidad_marcos));
        memoria->bitarray = bitarray_create_with_mode(
            memoria->bitmap, 
            calcular_bytes_necesarios(memoria->cantidad_marcos), 
            LSB_FIRST);
        
        for (int i = 0; i < memoria->cantidad_marcos; i++){
            bitarray_clean_bit(memoria->bitarray, i);
        }
    }
}

t_list* inicializar_tabla_paginas(uint32_t pid){
    t_list* tabla_paginas = list_create();
    char* pid_string = string_itoa(pid);
    dictionary_put(memoria->tablas_de_paginas, pid_string, tabla_paginas);

    free(pid_string);
    return tabla_paginas;
}

void liberar_tabla_paginas(uint32_t pid){
    char* pid_string = string_itoa(pid);

    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
   
    for(int i = 0; i < list_size(tabla_paginas); i++){
        int nro_marco = (int) list_get(tabla_paginas, i);
        liberar_marco(nro_marco);
    }
    
    list_destroy(tabla_paginas);
    dictionary_remove(memoria->tablas_de_paginas, pid_string);
    free(pid_string);
}

int leer_nro_marco(uint32_t pid, uint32_t nro_pagina){
    char* pid_string = string_itoa(pid);
    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
    free(pid_string);
    
    return (int) list_get(tabla_paginas, nro_pagina);
}

int obtener_marco_libre(){

    for(int nro_marco = 0; nro_marco < memoria->cantidad_marcos; nro_marco++){
        bool marco_libre = bitarray_test_bit(memoria->bitarray, nro_marco) == 0;
        if(marco_libre){
            bitarray_set_bit(memoria->bitarray, nro_marco);
            return nro_marco;
        }
    }
    return -1;
}

void liberar_marco(uint32_t nro_marco){
    bitarray_clean_bit(memoria->bitarray, nro_marco);
}

void ajustar_tamanio_proceso(uint32_t pid, uint32_t nuevo_tamanio){
    char* pid_string = string_itoa(pid);
    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);

    free(pid_string);

    uint32_t tamanio_actual = list_size(tabla_paginas);
    uint32_t tamanio_nuevo = nuevo_tamanio / memoria->tam_pagina;

    if(nuevo_tamanio % memoria->tam_pagina != 0){
        tamanio_nuevo++;
    }

    if(tamanio_nuevo > tamanio_actual){
        uint32_t paginas_a_agregar = tamanio_nuevo - tamanio_actual;
        for(int i = 0; i < paginas_a_agregar; i++){
            int marco_libre = obtener_marco_libre();

            if(marco_libre == -1){
                log_error(g_logger, "No hay marcos libres");
                break;
            }

            list_add(tabla_paginas, (void*) marco_libre);
        }
    } else {
        uint32_t paginas_a_eliminar = tamanio_actual - tamanio_nuevo;
        for(int i = 0; i < paginas_a_eliminar; i++){

            int pagina_a_eliminar = list_size(tabla_paginas) - 1;
            int nro_marco = (int) list_get(tabla_paginas, pagina_a_eliminar);

            liberar_marco(nro_marco);
            list_remove(tabla_paginas, pagina_a_eliminar);
        }
    }

    loggear_tabla_paginas(tabla_paginas);
}

void loggear_tabla_paginas(t_list* tabla_paginas){

    log_info(g_logger, "Tabla de páginas actualizada");
    for(int i = 0; i < list_size(tabla_paginas); i++){
        int nro_marco = (int) list_get(tabla_paginas, i);
        log_info(g_logger, "Página %d -> Marco %d", i, nro_marco);
    }
}

void leer_de_memoria(uint32_t direccion_fisica, uint32_t tamanio, void* buffer){
    void* direccion_fisica_real = memoria->espacio_contiguo + direccion_fisica;
    memcpy(buffer, direccion_fisica_real, tamanio);
}

void escribir_en_memoria(uint32_t direccion_fisica, uint32_t tamanio, void* buffer){
    void* direccion_fisica_real = memoria->espacio_contiguo + direccion_fisica;
    memcpy(direccion_fisica_real, buffer, tamanio);
}











