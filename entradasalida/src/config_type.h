#ifndef CONFIG_TYPE_IO_H_
#define CONFIG_TYPE_IO_H_

typedef struct {
    int tiempo_unidad_trabajo;
    char* puerto_kernel;
    char* puerto_memoria;
    int block_size;
    int block_count;
    char* tipo_interfaz;
    char* ip_kernel;
    char* ip_memoria;
    char* path_base_dialfs;
} ConfiguracionIO;

#endif
