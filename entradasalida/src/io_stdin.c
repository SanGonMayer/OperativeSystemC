#include "io_stdin.h"
#include "global_io.h"
#include "utils/buffer.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include "utils/server.h"
#include <commons/string.h>
#include <readline/readline.h>
#include <stdint.h>

char* stdin_leer_texto();
void guardar_en_memoria(char* texto, int direccion_fisica);
bool stdin_soporta_instruccion(char* instruccion);

char instruccion_soportada_stdin[14] = "IO_STDIN_READ";

void procesar_instruccion_stdin(int fd, t_instruccion_io* instruccion) {

    if(!stdin_soporta_instruccion(instruccion->instruccion)){
        log_error(g_logger, "Instruccion no soportada: %s", instruccion->instruccion);
        // que hay que hacer en este caso?
        return;
    }
    
    char* texto = stdin_leer_texto();
    
    if(string_length(texto)>instruccion->tamanio){
        log_error(g_logger, "Texto ingresado es mayor al tamanio permitido");
        responder_error(fd, ERROR_TAMANIO_PALABRA);
        return;
    }
    
    guardar_en_memoria(texto, instruccion->peticionesMemoria);

    responder_ok(fd);
}

char* stdin_leer_texto() {
    char* texto = readline("Ingrese texto: ");
    return texto;
}

void guardar_en_memoria(char* texto, t_list* peticionesMemoria) {

    for(int i = 0; i < list_size(peticionesMemoria); i++){
        t_peticion_acceso_usuario* peticion = list_get(peticionesMemoria, i);
        t_buffer* buffer = serializar_peticion_acceso_usuario(peticion);
        t_paquete* paquete = crear_paquete(ACCEDER_ESPACIO_DE_USUARIO_MEMORIA, buffer);
        enviar_paquete(paquete, g_socket_memoria);
        bool ok = recibir_ok(g_socket_memoria);
        if(ok){
            log_info(g_logger, "Se escribio correctamente en memoria");
        } else {
            log_error(g_logger, "No se pudo escribir en memoria");
        }
        eliminar_paquete(paquete);
    }

}

bool stdin_soporta_instruccion(char* instruccion){
    return string_equals_ignore_case(instruccion, instruccion_soportada_stdin);
}