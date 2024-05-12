#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>



void cargar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid, char* path_instrucciones);

char* leer_instruccion(t_dictionary* memoria_archivo, uint32_t pid, int* pc);

void liberar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid);

#endif /* INSTRUCCIONES_H_ */      