#ifndef CODIGO_OPERACION_H_
#define CODIGO_OPERACION_H_

#include "utils/codigo_error.h"
typedef enum
{
    HANDSHAKE = 1,
    ENVIO_PCB = 2,
    ENVIO_PID_PC = 3,
    ENVIO_PATH_INSTRUCCIONES = 4,
    INTERRUPCION_QUANTUM = 5,
    FINALIZACION = 6,
    IO_GEN_SLEEP = 7,
    ENVIO_INTERRUPCION = 8,
    ENVIO_INTERFAZ_CONECTADA = 9,
    FINALIZAR_PROCESO_MEMORIA = 10,
    OBTENER_MARCO_MEMORIA = 11,
    RESIZE_MEMORIA = 12,
    ACCEDER_ESPACIO_DE_USUARIO_MEMORIA = 13,
    OBTENER_TAMANIO_PAGINA = 14,
    IO_STDIN_READ = 15,
    IO_STDOUT_WRITE = 16,
    //estas son las dos señales que se mandan con el desalojo para el manejo de recursos
    WAIT = 17, 
    SIGNAL = 18,
    INTERRUPCION_KILL = 19
} t_codigo_operacion;

#endif