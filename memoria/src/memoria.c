#include "memoria.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <math.h>

static t_memoria* memoria = NULL;

int calcular_bits_necesarios(uint32_t cantidad_marcos){
    int bits = cantidad_marcos / 8;

    if(cantidad_marcos % 8 != 0){
        bits++;
    }
    return bits;
}

void inicializar_memoria(uint32_t tam_memoria, uint32_t tam_pagina){
    
    if(memoria == NULL){
        memoria = malloc(sizeof(t_memoria));
        memoria->tam_memoria = tam_memoria;
        memoria->tam_pagina = tam_pagina;
        memoria->cantidad_marcos = tam_memoria / tam_pagina;
        memoria->espacio_contiguo = malloc(tam_memoria);
        memoria->tablas_de_paginas = dictionary_create();

        memoria->bitmap = malloc(calcular_bits_necesarios(memoria->cantidad_marcos));
        memoria->bitarray = bitarray_create_with_mode(
            memoria->bitmap, 
            calcular_bits_necesarios(memoria->cantidad_marcos), 
            LSB_FIRST);
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

    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
   
    for(int i = 0; i < tabla_paginas[i] != NULL; i++){
        int nro_marco = (int) list_get(tabla_paginas, i);
        liberar_marco(nro_marco);
    }
    
    list_destroy(tabla_paginas);
    dictionary_remove(memoria->tablas_de_paginas, pid_string);
}

int leer_nro_marco(uint32_t pid, uint32_t nro_pagina){
    char* pid_string = string_itoa(pid);
    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
    return (int) list_get(tabla_paginas, nro_pagina);
}

int obtener_marco_libre(){
    return bitarray_test_and_set_bit(memoria->bitarray, 0);
}

void liberar_marco(uint32_t nro_marco){
    bitarray_clean_bit(memoria->bitarray, nro_marco);
}

void ajustar_tamanio_proceso(uint32_t pid, uint32_t nuevo_tamanio){
    char* pid_string = string_itoa(pid);
    t_list* tabla_paginas = dictionary_get(memoria->tablas_de_paginas, pid_string);
    uint32_t tamanio_actual = list_size(tabla_paginas);
    uint32_t tamanio_nuevo = nuevo_tamanio / memoria->tam_pagina;

    if(tamanio_nuevo > tamanio_actual){
        uint32_t paginas_a_agregar = tamanio_nuevo - tamanio_actual;
        for(int i = 0; i < paginas_a_agregar; i++){
            int marco_libre = obtener_marco_libre();
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
}


void leer_de_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* buffer){
    void* direccion_fisica_real = memoria->espacio_contiguo + direccion_fisica;
    memcpy(buffer, direccion_fisica_real, tamanio);
}

void escribir_en_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* buffer){
    void* direccion_fisica_real = memoria->espacio_contiguo + direccion_fisica;
    memcpy(direccion_fisica_real, buffer, tamanio);
}











