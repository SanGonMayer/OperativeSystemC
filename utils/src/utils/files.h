#ifndef FILES_H_
#define FILES_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/config.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>

bool abrir_archivo_txt(char* path, FILE** archivo);

char* leer_archivo_txt(char* path);


#endif /* FILES_H_ */      