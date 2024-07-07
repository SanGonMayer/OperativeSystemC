#include "io_dialfs.h"

static int blocks_fd;
static int bitmap_fd;
static t_bitarray* bitmap;
t_dictionary* comandos;

void get_comandos(){

    comandos = dictionary_create();

    dictionary_put(comandos, "IO_FS_CREATE", IO_DIALFS_CREATE);
    dictionary_put(comandos, "IO_FS_DELETE", IO_DIALFS_DELETE);
    dictionary_put(comandos, "IO_FS_TRUNCATE", IO_DIALFS_TRUNCATE);
    dictionary_put(comandos, "IO_FS_WRITE", IO_DIALFS_WRITE);
    dictionary_put(comandos, "IO_FS_READ", IO_DIALFS_READ);

}

void ejecutar_instruccion(int fd, t_operacion_dialfs operacion, t_instruccion_io* instruccion){
    switch (operacion) {
        case IO_DIALFS_CREATE:
            io_fs_create(instruccion->nombre_archivo);
            responder_ok(fd);
            break;
        case IO_DIALFS_DELETE:
            io_fs_delete(instruccion->nombre_archivo);
            responder_ok(fd);
            break;
        case IO_DIALFS_TRUNCATE:
            io_fs_truncate(instruccion->nombre_archivo, instruccion->tamanio);
            responder_ok(fd);
            break;
        case IO_DIALFS_WRITE:
            io_fs_write(instruccion);
            responder_ok(fd);
            break;
        case IO_DIALFS_READ:   
            io_fs_read(instruccion);
            responder_ok(fd);
            break;
        default:
            log_error(g_logger, "Instruccion no existente");
            break;
    }
}

void procesar_instruccion_dialfs(int fd, t_instruccion_io* instruccion) {

    if(dictionary_has_key(comandos, instruccion->instruccion)){
        t_operacion_dialfs operacion = (t_operacion_dialfs) dictionary_get(comandos, instruccion->instruccion);
        ejecutar_instruccion(fd, operacion, instruccion);
    }else{
        log_error(g_logger, "ingresaste una funcion no valida");
    }

}

void initialize_fs() {
    char blocks_path[256];
    char bitmap_path[256];
    snprintf(blocks_path, sizeof(blocks_path), "%s/bloques.dat", g_config_io->path_base_dialfs);
    snprintf(bitmap_path, sizeof(bitmap_path), "%s/bitmap.dat", g_config_io->path_base_dialfs);
    
    //Inicia diccionario
    get_comandos();

    int block_size = g_config_io->block_size;
    int block_count = g_config_io->block_count;
    int bitmap_size = block_count / 8;

    // Crear archivo de bloques
    blocks_fd = open(blocks_path, O_CREAT | O_RDWR, 0644);
    if (blocks_fd == -1) {
        log_error(g_logger, "Error al crear o abrir el archivo de bloques");
        exit(EXIT_FAILURE);
    }
    ftruncate(blocks_fd, block_size * block_count);

    // Crear archivo de bitmap
    bitmap_fd = open(bitmap_path, O_CREAT | O_RDWR, 0644);
    if (bitmap_fd == -1) {
        log_error(g_logger, "Error al crear o abrir el archivo de bitmap");
        close(blocks_fd);
        exit(EXIT_FAILURE);
    }
    ftruncate(bitmap_fd, bitmap_size);

    // Inicializar bitmap
    char* bitmap_data = malloc(bitmap_size);
    if (bitmap_data == NULL) {
        log_error(g_logger, "Error al asignar memoria para el bitmap");
        close(blocks_fd);
        close(bitmap_fd);
        exit(EXIT_FAILURE);
    }
    
    memset(bitmap_data, 0, bitmap_size);
    write(bitmap_fd, bitmap_data, bitmap_size);
    lseek(bitmap_fd, 0, SEEK_SET);

    bitmap = bitarray_create_with_mode(bitmap_data, bitmap_size, LSB_FIRST);

    // Verificar que el bitarray se haya creado correctamente
    if (bitmap == NULL) {
        log_error(g_logger, "Error al crear el bitarray");
        free(bitmap_data);
        close(blocks_fd);
        close(bitmap_fd);
        exit(EXIT_FAILURE);
    }

    log_info(g_logger, "FS inicializado correctamente");
}

void finalize_fs() {
    dictionary_destroy(comandos);
    bitarray_destroy(bitmap);
    close(bitmap_fd);
    close(blocks_fd);
}

t_config* load_metadata(const char* filename) {
    char metadata_path[256];
    snprintf(metadata_path, sizeof(metadata_path), "%s/%s", g_config_io->path_base_dialfs, filename);
    return config_create(metadata_path);
}

void save_metadata(const char* filename, int initial_block, int file_size) {
    char metadata_path[256];
    snprintf(metadata_path, sizeof(metadata_path), "%s/%s", g_config_io->path_base_dialfs, filename);

    t_config* metadata = config_create(metadata_path);

    if (metadata == NULL) {
        metadata = config_create(metadata_path);
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

void io_fs_create(char* filename) {
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

void io_fs_delete(char* filename) {
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

t_bitarray* load_bitmap() {
    FILE* bitmap_file = fopen("bitmap.dat", "rb");
    if (bitmap_file == NULL) {
        perror("Error al abrir el archivo de bitmap");
        return NULL;
    }

    fseek(bitmap_file, 0, SEEK_END);
    long file_size = ftell(bitmap_file);
    fseek(bitmap_file, 0, SEEK_SET);

    char* bitmap_data = malloc(file_size);
    fread(bitmap_data, 1, file_size, bitmap_file);
    fclose(bitmap_file);

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_data, file_size, LSB_FIRST);
    return bitmap;
}

void save_bitmap(t_bitarray* bitmap) {
    FILE* bitmap_file = fopen("bitmap.dat", "wb");
    if (bitmap_file == NULL) {
        perror("Error al abrir el archivo de bitmap");
        return;
    }
    //Revisar
    fwrite(bitmap->bitarray, 1, bitarray_get_max_bit(bitmap) / 8, bitmap_file);
    fclose(bitmap_file);
}

void compactar_fs() {
    log_info(g_logger, "Iniciando compactación del sistema de archivos");

    int block_size = g_config_io->block_size;
    int block_count = g_config_io->block_count;
    int free_block_index = -1;

    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            if (free_block_index == -1) {
                free_block_index = i;
            }
        } else if (free_block_index != -1) {
            // Mover bloque ocupado al bloque libre
            void* data = malloc(block_size);
            lseek(blocks_fd, i * block_size, SEEK_SET);
            read(blocks_fd, data, block_size);

            lseek(blocks_fd, free_block_index * block_size, SEEK_SET);
            write(blocks_fd, data, block_size);

            // Actualizar el bitmap
            bitarray_clean_bit(bitmap, i);
            bitarray_set_bit(bitmap, free_block_index);

            free(data);
            free_block_index++;
        }
    }

    save_bitmap(bitmap);
    usleep(g_config_io->retraso_compactacion * 1000);
    log_info(g_logger, "Compactación del sistema de archivos completada");
}

void io_fs_truncate(char* filename, int new_size) {
    t_config* metadata = load_metadata(filename);
    if (metadata == NULL) {
        printf("El archivo no existe\n");
        return;
    }

    int initial_block = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int old_size = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int block_size = g_config_io->block_size;
    int old_blocks = (old_size + block_size - 1) / block_size;
    int new_blocks = (new_size + block_size - 1) / block_size;

    if (new_blocks > old_blocks) {
        int free_blocks = 0;
        int start_block = -1;

        for (int i = 0; i < g_config_io->block_count; i++) {
            if (!bitarray_test_bit(bitmap, i)) {
                if (start_block == -1) {
                    start_block = i;
                }
                free_blocks++;
                if (free_blocks >= (new_blocks - old_blocks)) {
                    break;
                }
            } else {
                start_block = -1;
                free_blocks = 0;
            }
        }

        if (free_blocks < (new_blocks - old_blocks)) {
            compactar_fs();

            free_blocks = 0;
            start_block = -1;
            for (int i = 0; i < g_config_io->block_count; i++) {
                if (!bitarray_test_bit(bitmap, i)) {
                    if (start_block == -1) {
                        start_block = i;
                    }
                    free_blocks++;
                    if (free_blocks >= (new_blocks - old_blocks)) {
                        break;
                    }
                } else {
                    start_block = -1;
                    free_blocks = 0;
                }
            }

            if (free_blocks < (new_blocks - old_blocks)) {
                printf("No hay suficiente espacio libre después de la compactación\n");
                return;
            }
        }

        for (int i = start_block; i < start_block + (new_blocks - old_blocks); i++) {
            bitarray_set_bit(bitmap, i);
        }

        save_bitmap(bitmap);
    } else if (new_blocks < old_blocks) {
        for (int i = initial_block + new_blocks; i < initial_block + old_blocks; i++) {
            bitarray_clean_bit(bitmap, i);
        }
        save_bitmap(bitmap);
    }

    save_metadata(filename, initial_block, new_size);
    config_destroy(metadata);
}

void io_fs_write(t_instruccion_io* instruccion) {
    
    char* filename = instruccion->nombre_archivo;
    int size = instruccion->tamanio;
    int offset = instruccion->puntero_archivo;
    char* data = string_new();

    data = leer_de_memoria(g_socket_memoria,size, instruccion->peticionesMemoria);
    
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
    char* current_data = string_duplicate(data);

    for (int i = block_start; i <= block_end; i++) {
        int write_size = g_config_io->block_size - block_offset;
        if (i == block_end) {
            write_size = (offset + size) % g_config_io->block_size;
        }

        lseek(blocks_fd, (initial_block + i) * g_config_io->block_size + block_offset, SEEK_SET);
        write(blocks_fd, current_data, write_size);
        current_data += write_size;
        block_offset = 0;
    }

    config_destroy(metadata);
}

void io_fs_read(t_instruccion_io* instruccion) {
    char* filename = instruccion->nombre_archivo;
    int size = instruccion->tamanio;
    int offset = instruccion->puntero_archivo;  // Usar puntero_archivo como offset
    char* data = malloc(size);

    t_config* metadata = load_metadata(filename);
    if (metadata == NULL) {
        printf("El archivo no existe\n");
        free(data);
        return;
    }

    int initial_block = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int file_size = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

    if (offset + size > file_size) {
        printf("Error: Intento de lectura fuera del tamaño del archivo\n");
        free(data);
        config_destroy(metadata);
        return;
    }

    int block_start = offset / g_config_io->block_size;
    int block_end = (offset + size - 1) / g_config_io->block_size;
    int block_offset = offset % g_config_io->block_size;

    char* current_data = string_duplicate(data);

    for (int i = block_start; i <= block_end; i++) {
        int read_size = g_config_io->block_size - block_offset;
        if (i == block_end) {
            read_size = (offset + size) % g_config_io->block_size;
        }

        lseek(blocks_fd, (initial_block + i) * g_config_io->block_size + block_offset, SEEK_SET);
        read(blocks_fd, current_data, read_size);
        current_data += read_size;
        block_offset = 0;
    }

    config_destroy(metadata);

    // Escribir en la memoria
    //Ver si funciona por el g_socket_memoria o hay que pasarle un socket
    guardar_en_memoria(g_socket_memoria,data, instruccion->peticionesMemoria);

}