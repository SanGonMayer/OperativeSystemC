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
bool stdin_soporta_instruccion(char* instruccion);

char instruccion_soportada_stdin[14] = "IO_STDIN_READ";

void procesar_instruccion_stdin(int fd, t_instruccion_io* instruccion) {

    if(!stdin_soporta_instruccion(instruccion->instruccion)){
        log_error(g_logger, "Instruccion no soportada: %s", instruccion->instruccion);
        // que hay que hacer en este caso?
        return;
    }
    
    log_info(g_logger, "PID: %d - Operacion: %s", instruccion->pid, instruccion->instruccion);

    char* texto = stdin_leer_texto();
    
    if(string_length(texto)>instruccion->tamanio){
        log_error(g_logger, "Texto ingresado es mayor al tamanio permitido");
        responder_error(fd, ERROR_TAMANIO_PALABRA);
        return;
    }
    
    actualizar_peticiones_con_valor(instruccion->peticionesMemoria, texto);

    guardar_en_memoria(g_socket_memoria,texto, instruccion->peticionesMemoria, g_logger);

    responder_ok(fd);
}

char* stdin_leer_texto() {
    char* texto = readline("Ingrese texto: ");
    return texto;
}

bool stdin_soporta_instruccion(char* instruccion){
    return string_equals_ignore_case(instruccion, instruccion_soportada_stdin);
}