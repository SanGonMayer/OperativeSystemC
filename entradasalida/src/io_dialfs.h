#ifndef IO_DIALFS_H
#define IO_DIALFS_H
#include "utils/instrucciones_io.h"

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
void io_fs_truncate(const char* filename, int new_size);
void io_fs_write(t_instruccion_io* instruccion);
void io_fs_read(t_instruccion_io* instruccion);

#endif // IO_DIALFS_H