#ifndef CODIGO_OPERACION_H_
#define CODIGO_OPERACION_H_

typedef enum
{
    HANDSHAKE = 1,
    ENVIO_PCB = 2,
    ENVIO_PID_PC = 3,
    ENVIO_PATH_INSTRUCCIONES = 4,
} t_codigo_operacion;

#endif