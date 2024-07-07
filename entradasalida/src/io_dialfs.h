#ifndef IO_DIALFS_H
#define IO_DIALFS_H
#include "global_io.h"
#include "utils/instrucciones_io.h"
#include <commons/bitarray.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "global_io.h"
#include "utils/buffer.h"
#include "utils/codigo_operacion.h"
#include "utils/peticiones_memoria.h"
#include "utils/client.h"
#include "utils/server.h"
#include <commons/string.h>
#include <readline/readline.h>
#include <stdint.h>
#include "io_memoria.h"
#include <errno.h>
typedef enum {
    IO_DIALFS_CREATE = 0,
    IO_DIALFS_DELETE = 1,
    IO_DIALFS_TRUNCATE = 2,
    IO_DIALFS_WRITE = 3,
    IO_DIALFS_READ = 4
} t_operacion_dialfs;

void procesar_instruccion_dialfs(int fd, t_instruccion_io* instruccion);
void ejecutar_instruccion(int fd, t_operacion_dialfs operacion, t_instruccion_io* instruccion);
void get_comandos();
//realizadas 
void initialize_fs();
void finalize_fs();
void io_fs_create(char* filename);
void io_fs_delete(char* filename);
t_config* load_metadata(const char* filename);
void save_metadata(const char* filename, int initial_block, int file_size);
t_bitarray* load_bitmap();
void save_bitmap(t_bitarray* bitmap);
void compactar_fs();
void io_fs_truncate(char* filename, int new_size);
void io_fs_write(t_instruccion_io* instruccion);
void io_fs_read(t_instruccion_io* instruccion);

#endif // IO_DIALFS_H