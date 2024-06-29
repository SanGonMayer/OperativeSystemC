#include "io_stdout.h"
#include "global_io.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include <commons/string.h>
#include <readline/readline.h>

void leer_de_memoria_stdout(int tamanio, t_list* peticionesMemoria);
bool stdout_soporta_instruccion(char* instruccion);

char instruccion_soportada_stdout[15] = "IO_STDOUT_WRITE";

void procesar_instruccion_stdout(int fd, t_instruccion_io* instruccion){
    
    if(!stdout_soporta_instruccion(instruccion->instruccion)){
        log_error(g_logger, "Instruccion no soportada: %s", instruccion->instruccion);
        // que hay que hacer en este caso?
        return;
    }
    
    int tamanio = instruccion->tamanio;
    
    leer_de_memoria_stdout(tamanio, instruccion->peticionesMemoria);
    
    responder_ok(fd);
}

void leer_de_memoria_stdout(int tamanio, t_list* peticionesMemoria) {

    char* mensaje = string_new();
    for(int i = 0; i < list_size(peticionesMemoria); i++){
        t_peticion_acceso_usuario * peticion = list_get(peticionesMemoria, i);
        t_buffer* buffer =  serializar_peticion_acceso_usuario(peticion);
        t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);
        enviar_paquete(paquete, g_socket_memoria);
        t_buffer* buffer_respuesta = recibir_buffer(g_socket_memoria);
        uint32_t length;
        char* respuesta = buffer_read_string(buffer_respuesta, &length);
        string_append(&mensaje, respuesta);

        free(respuesta);
        eliminar_paquete(paquete);
        buffer_destroy(buffer_respuesta);
    }

    uint32_t length = string_length(mensaje);

    mensaje[length] = '\0';

    log_info(g_logger, "Texto leido de memoria: %s", mensaje);
    
}

bool stdout_soporta_instruccion(char* instruccion){
    return string_equals_ignore_case(instruccion, instruccion_soportada_stdout);
}