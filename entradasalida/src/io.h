#ifndef IO_H_
#define IO_H_

#include "utils/instrucciones_io.h"


typedef void* (*ProcesarInstruccion)(int fd, t_instruccion_io* instruccion);

int iniciar_conexion_kernel(char* tipo_interfaz);
int iniciar_conexion_memoria();
t_instruccion_io* recibir_instruccion_io(int conexion_kernel);
void atender_instrucciones(int conexion_kernel, ProcesarInstruccion procesar_instruccion);

#endif