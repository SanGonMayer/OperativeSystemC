#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <utils/procesos.h>
#include <utils/buffer.h>
#include <commons/log.h>

typedef struct{
    uint32_t PID;
    char* path;
    uint32_t path_length;
    t_registrosMem registrosMemoria;
}t_paqueteMemoria

void iterator(char* value, t_log* logger);

#endif