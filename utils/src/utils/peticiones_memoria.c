#include "peticiones_memoria.h"
#include "utils/buffer.h"
#include <commons/string.h>
#include <stdint.h>

t_peticion_marco* crear_peticion_marco(uint32_t pid, int pagina){
    t_peticion_marco* peticion = malloc(sizeof(t_peticion_marco));
    peticion->pid = pid;
    peticion->pagina = pagina;
    return peticion;
}

void destruir_peticion_marco(t_peticion_marco* peticion){
    free(peticion);
}

t_buffer* serializar_peticion_marco(t_peticion_marco* peticion){
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + sizeof(int));

    buffer_add_uint32(buffer, peticion->pid);
    buffer_add_int(buffer, peticion->pagina);

    return buffer;
}

t_peticion_marco* deserializar_peticion_marco(t_buffer* buffer){
    t_peticion_marco* peticion = malloc(sizeof(t_peticion_marco));
    
    peticion->pid = buffer_read_uint32(buffer);
    peticion->pagina = buffer_read_int(buffer);

    return peticion;
}


t_peticion_resize* crear_peticion_resize(uint32_t pid, int tamanio){
    t_peticion_resize* peticion = malloc(sizeof(t_peticion_resize));

    peticion->pid = pid;
    peticion->tamanio = tamanio;

    return peticion;
}

void destruir_peticion_resize(t_peticion_resize* peticion){
    free(peticion);
}

t_buffer* serializar_peticion_resize(t_peticion_resize* peticion){
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + sizeof(int));

    buffer_add_uint32(buffer, peticion->pid);
    buffer_add_int(buffer, peticion->tamanio);

    return buffer;
}

t_peticion_resize* deserializar_peticion_resize(t_buffer* buffer){
    t_peticion_resize* peticion = malloc(sizeof(t_peticion_resize));

    peticion->pid = buffer_read_uint32(buffer);
    peticion->tamanio = buffer_read_int(buffer);

    return peticion;
}


t_peticion_acceso_usuario* crear_peticion_lectura(uint32_t tamanio_a_leer, int direccion_fisica){
    t_peticion_acceso_usuario* peticion = malloc(sizeof(t_peticion_acceso_usuario));

    peticion->tamanio_a_leer = tamanio_a_leer;
    peticion->tipo_acceso = LECTURA;
    peticion->direccion_fisica = direccion_fisica;
    peticion->string = string_new();

    return peticion;
}

t_peticion_acceso_usuario* crear_peticion_escritura_stdin(int direccion_fisica, int tamanio_a_leer){
    t_peticion_acceso_usuario* peticion = malloc(sizeof(t_peticion_acceso_usuario));

    peticion->tamanio_a_leer = tamanio_a_leer;
    peticion->tipo_acceso = ESCRITURA;
    peticion->direccion_fisica = direccion_fisica;
    peticion->string = string_new();

    return peticion;
}

t_peticion_acceso_usuario* crear_peticion_escritura(int direccion_fisica, char* string){
    t_peticion_acceso_usuario* peticion = malloc(sizeof(t_peticion_acceso_usuario));

    peticion->tamanio_a_leer = string_length(string);
    peticion->tipo_acceso = ESCRITURA;
    peticion->direccion_fisica = direccion_fisica;
    if(string != NULL){
        peticion->string = string_duplicate(string);
    } else {
        peticion->string = string_new();
    }

    return peticion;
}

void destruir_peticion_acceso_usuario(t_peticion_acceso_usuario* peticion){
    if(peticion->string != NULL){
        free(peticion->string);
    }
    free(peticion);
}

t_buffer* serializar_peticion_acceso_usuario(t_peticion_acceso_usuario* peticion){
    int size = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int);

    if(peticion->string != NULL){
        size += sizeof(uint32_t) + strlen(peticion->string) + 1;
    }

    t_buffer* buffer = buffer_create(size);

    buffer_add_uint32(buffer, peticion->tamanio_a_leer);
    buffer_add_uint32(buffer, peticion->tipo_acceso);
    buffer_add_int(buffer, peticion->direccion_fisica);

    if(peticion->string != NULL){
        buffer_add_string(buffer, strlen(peticion->string) + 1, peticion->string);
    }

    return buffer;
}

t_peticion_acceso_usuario* deserializar_peticion_acceso_usuario(t_buffer* buffer){
    t_peticion_acceso_usuario* peticion = malloc(sizeof(t_peticion_acceso_usuario));

    peticion->tamanio_a_leer = buffer_read_uint32(buffer);
    peticion->tipo_acceso = buffer_read_uint32(buffer);
    peticion->direccion_fisica = buffer_read_int(buffer);

    if(peticion->tipo_acceso == ESCRITURA){
        uint32_t size;
        peticion->string = buffer_read_string(buffer, &size);
    }else{
        peticion->string = string_new();
    }

    return peticion;
}

t_peticion_finalizar_proceso* crear_peticion_finalizar_proceso(uint32_t pid){
    t_peticion_finalizar_proceso* peticion = malloc(sizeof(t_peticion_finalizar_proceso));

    peticion->pid = pid;

    return peticion;

}

void destruir_peticion_finalizar_proceso(t_peticion_finalizar_proceso* peticion){
    free(peticion);
}

t_buffer* serializar_peticion_finalizar_proceso(t_peticion_finalizar_proceso* peticion){
    t_buffer* buffer = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(buffer, peticion->pid);

    return buffer;
}

t_peticion_finalizar_proceso* deserializar_peticion_finalizar_proceso(t_buffer* buffer){
    t_peticion_finalizar_proceso* peticion = malloc(sizeof(t_peticion_finalizar_proceso));

    peticion->pid = buffer_read_uint32(buffer);

    return peticion;
}