#include "memoria.h"
#include "config.h"
#include <commons/collections/dictionary.h>
#include <stdint.h>


t_paqueteMemoria* inicializar_paquete_memoria(){
    t_paqueteMemoria* paqueteMemoria = malloc(sizeof(t_paqueteMemoria));
    paqueteMemoria->PID = 0;
    return paqueteMemoria;
}

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria, t_log*logger){
    t_buffer* buffer = recibir_buffer(socket);

    if (buffer->stream == -1){
        log_error(logger, "Error al recibir contexto de kernel");
        return;
    }

    paqueteMemoria->PID = buffer_read_uint32(buffer);
    paqueteMemoria->path = buffer_read_string(buffer, &paqueteMemoria->path_length);

    free(buffer->stream);
    free(buffer);
}

void retardo_respuesta_memoria(){
    int retardo_respuesta = get_config_memoria()->retardo_respuesta;
    sleep(retardo_respuesta / 1000);
}


