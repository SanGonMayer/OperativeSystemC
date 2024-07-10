#include "io_stdout.h"
#include "global_io.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include <commons/string.h>
#include <readline/readline.h>

bool stdout_soporta_instruccion(char* instruccion);

char instruccion_soportada_stdout[15] = "IO_STDOUT_WRITE";

void procesar_instruccion_stdout(int fd, t_instruccion_io* instruccion){
    
    if(!stdout_soporta_instruccion(instruccion->instruccion)){
        log_error(g_logger, "Instruccion no soportada: %s", instruccion->instruccion);
        // que hay que hacer en este caso?
        return;
    }
    
    log_info(g_logger, "PID: %d - Operacion: %s", instruccion->pid, instruccion->instruccion);

    int tamanio = instruccion->tamanio;
    
    char* mensaje = leer_de_memoria(g_socket_memoria,tamanio, instruccion->peticionesMemoria, g_logger);

    log_info(g_logger, "%s", mensaje);

    responder_ok(fd);
}

bool stdout_soporta_instruccion(char* instruccion){
    return string_equals_ignore_case(instruccion, instruccion_soportada_stdout);
}