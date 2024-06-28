#ifndef IO_STDIN_IO_H_
#define IO_STDIN_IO_H_

#include "utils/instrucciones_io.h"
void procesar_instruccion_stdin(int fd, t_instruccion_io* instruccion);
void guardar_en_memoria(char* texto, t_list* peticionesMemoria);
#endif