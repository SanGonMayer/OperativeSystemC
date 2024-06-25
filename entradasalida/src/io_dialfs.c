#include "global_io.h"
#include "io_dialfs.h"
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

static int blocks_fd;
static int bitmap_fd;
static t_bitarray* bitmap;

t_dictionary* get_comandos(){

    t_dictionary* comandos = dictionary_create();

    dictionary_put(comandos, "IO_FS_CREATE", (void*) IO_DIALFS_CREATE);
    dictionary_put(comandos, "IO_FS_DELETE", (void*) IO_DIALFS_DELETE);
    dictionary_put(comandos, "IO_FS_TRUNCATE", (void*) IO_DIALFS_TRUNCATE);
    dictionary_put(comandos, "IO_FS_WRITE", (void*) IO_DIALFS_WRITE);
    dictionary_put(comandos, "IO_FS_READ", (void*) IO_DIALFS_READ);

    return comandos;
}

void ejecutar_instruccion(t_operacion_dialfs operacion, t_argumentos_instruccion* argumentos){
    switch (operacion) {
        case IO_DIALFS_CREATE:
            io_fs_create();
            break;
        case IO_DIALFS_DELETE:
            io_fs_delete();
            break;
        case IO_DIALFS_TRUNCATE:
            io_fs_truncate();
            break;
        case IO_DIALFS_WRITE:
            io_fs_write();
            break;
        case IO_DIALFS_READ:   
            io_fs_read();
            break;
        default:
            log_error(g_logger, "Instruccion no existente");
            break;
    }
}

void procesar_instruccion_dialfs(int fd, t_instruccion_io* instruccion) {
    t_dictionary comandos = get_comandos();

    if(dictionary_has_key(comandos, instruccion->instruccion)){

        t_operacion_dialfs operacion = (t_operacion_dialfs) dictionary_get(comandos, instruccion->instruccion);
        t_argumentos_instruccion* argumentos = NULL;
        //TODO argumentos
        ejecutar_instruccion(operacion, argumentos);
    }else{
        log_error(g_logger, "ingresaste una funcion no valida");
    }

}

void initialize_fs() {
    char blocks_path[256];
    char bitmap_path[256];
    snprintf(blocks_path, sizeof(blocks_path), "%s/bloques.dat", g_config_io->path_base_dialfs);
    snprintf(bitmap_path, sizeof(bitmap_path), "%s/bitmap.dat", g_config_io->path_base_dialfs);
    
    int block_size = g_config_io->block_size;
    int block_count = g_config_io->block_count;
    int bitmap_size = block_count / 8;

    // Crear archivo de bloques
    blocks_fd = open(blocks_path, O_CREAT | O_RDWR, 0644);
    if (blocks_fd == -1) {
        log_error(logger, "Error al crear o abrir el archivo de bloques");
        exit(EXIT_FAILURE);
    }
    ftruncate(blocks_fd, block_size * block_count);

    // Crear archivo de bitmap
    bitmap_fd = open(bitmap_path, O_CREAT | O_RDWR, 0644);
    if (bitmap_fd == -1) {
        log_error(logger, "Error al crear o abrir el archivo de bitmap");
        close(blocks_fd);
        exit(EXIT_FAILURE);
    }
    ftruncate(bitmap_fd, bitmap_size);

    // Inicializar bitmap
    char* bitmap_data = malloc(bitmap_size);
    if (bitmap_data == NULL) {
        log_error(logger, "Error al asignar memoria para el bitmap");
        close(blocks_fd);
        close(bitmap_fd);
        exit(EXIT_FAILURE);
    }
    
    memset(bitmap_data, 0, bitmap_size);
    write(bitmap_fd, bitmap_data, bitmap_size);
    lseek(bitmap_fd, 0, SEEK_SET);

    bitmap = bitarray_create_with_mode(bitmap_data, bitmap_size, MSB_FIRST);

    // Verificar que el bitarray se haya creado correctamente
    if (bitmap == NULL) {
        log_error(logger, "Error al crear el bitarray");
        free(bitmap_data);
        close(blocks_fd);
        close(bitmap_fd);
        exit(EXIT_FAILURE);
    }

    log_info(logger, "Sistema de archivos inicializado correctamente");
}

void finalize_fs() {
    bitarray_destroy(bitmap);
    close(bitmap_fd);
    close(blocks_fd);
}

static t_config* load_metadata(const char* filename) {
    char metadata_path[256];
    snprintf(metadata_path, sizeof(metadata_path), "%s/%s", g_config_io->path_base_dialfs, filename);
    return config_create(metadata_path);
}

t_config* config_create_empty() {
    t_config* config = malloc(sizeof(t_config));
    config->properties = dictionary_create();
    return config;
}

static void save_metadata(const char* filename, int initial_block, int file_size) {
    char metadata_path[256];
    snprintf(metadata_path, sizeof(metadata_path), "%s/%s", g_config_io->path_base_dialfs, filename);

    t_config* metadata = config_create(metadata_path);

    if (metadata == NULL) {
        metadata = config_create_empty();
        config_save_in_file(metadata, metadata_path);
        metadata = config_create(metadata_path);
    }

    char initial_block_str[10];
    char file_size_str[10];
    snprintf(initial_block_str, sizeof(initial_block_str), "%d", initial_block);
    snprintf(file_size_str, sizeof(file_size_str), "%d", file_size);

    config_set_value(metadata, "BLOQUE_INICIAL", initial_block_str);
    config_set_value(metadata, "TAMANIO_ARCHIVO", file_size_str);
    config_save(metadata);
    config_destroy(metadata);
}

void io_fs_create(const char* filename) {
    if (access(filename, F_OK) == 0) {
        printf("El archivo ya existe\n");
        return;
    }

    int initial_block = -1;
    for (int i = 0; i < g_config_io->block_count; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            initial_block = i;
            bitarray_set_bit(bitmap, i);
            break;
        }
    }

    if (initial_block == -1) {
        printf("No hay bloques disponibles\n");
        return;
    }

    save_metadata(filename, initial_block, 0);
}

void io_fs_delete(const char* filename) {
    t_config* metadata = load_metadata(filename);
    if (metadata == NULL) {
        printf("El archivo no existe\n");
        return;
    }

    int initial_block = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int file_size = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    int blocks_to_free = (file_size + g_config_io->block_size - 1) / g_config_io->block_size;
    for (int i = 0; i < blocks_to_free; i++) {
        bitarray_clean_bit(bitmap, initial_block + i);
    }

    char metadata_path[256];
    snprintf(metadata_path, sizeof(metadata_path), "%s/%s", g_config_io->path_base_dialfs, filename);
    remove(metadata_path);
    config_destroy(metadata);
}

void io_fs_truncate(const char* filename, int new_size) {
    t_config* metadata = load_metadata(filename);
    if (metadata == NULL) {
        printf("El archivo no existe\n");
        return;
    }

    int initial_block = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int file_size = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    int new_blocks_needed = (new_size + g_config_io->block_size - 1) / g_config_io->block_size;
    int current_blocks = (file_size + g_config_io->block_size - 1) / g_config_io->block_size;

    if (new_blocks_needed > current_blocks) {
        // Ampliación del archivo
        for (int i = current_blocks; i < new_blocks_needed; i++) {
            if (bitarray_test_bit(bitmap, initial_block + i)) {
                // Necesitamos compactar
                printf("Espacio fragmentado, compactando...\n");
                config_destroy(metadata);
                // Aquí iría la lógica de compactación
                usleep(g_config_io->retraso_compactacion * 1000);
                io_fs_truncate(filename, new_size);
                return;
            } else {
                bitarray_set_bit(bitmap, initial_block + i);
            }
        }
    } else if (new_blocks_needed < current_blocks) {
        // Reducción del archivo
        for (int i = new_blocks_needed; i < current_blocks; i++) {
            bitarray_clean_bit(bitmap, initial_block + i);
        }
    }

    save_metadata(filename, initial_block, new_size);
    config_destroy(metadata);
}

void io_fs_write(const char* filename, const char* data, int size, int offset) {
    t_config* metadata = load_metadata(filename);
    if (metadata == NULL) {
        printf("El archivo no existe\n");
        return;
    }

    int initial_block = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int file_size = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    int end_offset = offset + size;
    if (end_offset > file_size) {
        io_fs_truncate(filename, end_offset);
        config_destroy(metadata);
        metadata = load_metadata(filename);
        initial_block = config_get_int_value(metadata, "BLOQUE_INICIAL");
        file_size = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    }

    int block_start = offset / g_config_io->block_size;
    int block_end = (offset + size - 1) / g_config_io->block_size;
    int block_offset = offset % g_config_io->block_size;

    for (int i = block_start; i <= block_end; i++) {
        int write_size = g_config_io->block_size - block_offset;
        if (i == block_end) {
            write_size = (offset + size) % g_config_io->block_size;
        }

        lseek(blocks_fd, (initial_block + i) * g_config_io->block_size + block_offset, SEEK_SET);
        write(blocks_fd, data, write_size);
        data += write_size;
        block_offset = 0;
    }

    config_destroy(metadata);
}

void io_fs_read(const char* filename, char* buffer, int size, int offset) {
    t_config* metadata = load_metadata(filename);
    if (metadata == NULL) {
        printf("El archivo no existe\n");
        return;
    }

    int initial_block = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int file_size = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    if (offset + size > file_size) {
        printf("Lectura fuera de los límites del archivo\n");
        config_destroy(metadata);
        return;
    }

    int block_start = offset / g_config_io->block_size;
    int block_end = (offset + size - 1) / g_config_io->block_size;
    int block_offset = offset % g_config_io->block_size;

    for (int i = block_start; i <= block_end; i++) {
        int read_size = g_config_io->block_size - block_offset;
        if (i == block_end) {
            read_size = (offset + size) % g_config_io->block_size;
        }

        lseek(blocks_fd, (initial_block + i) * g_config_io->block_size + block_offset, SEEK_SET);
        read(blocks_fd, buffer, read_size);
        buffer += read_size;
        block_offset = 0;
    }

    config_destroy(metadata);
}