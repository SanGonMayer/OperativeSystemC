
#include "global_io.h"
#include "utils/instrucciones_io.h"
#include <commons/string.h>
#include <unistd.h>

// instrucciones soportadas
// IO_GEN_SLEEP

char instruccion_soportada[13] = "IO_GEN_SLEEP";

void procesar_instruccion_generica(int fd, t_instruccion_io* instruccion){
    // semaforo para bloquear el uso de la io

    
    if(!string_equals_ignore_case(instruccion->instruccion, instruccion_soportada)){
        
        log_error(g_logger, "Instruccion no soportada: %s", instruccion->instruccion);
        // que hay que hacer en este caso?
        return;
    }

    int tiempo_sleep = instruccion->unidades_trabajo * g_config_io->tiempo_unidad_trabajo;

    log_info(g_logger, "Durmiendo %d segundos", tiempo_sleep);
    sleep(tiempo_sleep);

    // responder al kernel
}