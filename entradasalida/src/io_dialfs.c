#include "io_dialfs.h"
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <dirent.h>

static int blocks_fd;
static int bitmap_fd;
static t_bitarray* bitmap;
t_dictionary* comandos;
char* blocks_path;
char* bitmap_path;


void get_comandos(){

    comandos = dictionary_create();

    dictionary_put(comandos, "IO_FS_CREATE", (void*)IO_DIALFS_CREATE);
    dictionary_put(comandos, "IO_FS_DELETE", (void*)IO_DIALFS_DELETE);
    dictionary_put(comandos, "IO_FS_TRUNCATE", (void*)IO_DIALFS_TRUNCATE);
    dictionary_put(comandos, "IO_FS_WRITE", (void*)IO_DIALFS_WRITE);
    dictionary_put(comandos, "IO_FS_READ", (void*)IO_DIALFS_READ);

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
    blocks_path = string_from_format("%s/blocks.dat", g_config_io->path_base_dialfs);
    bitmap_path = string_from_format("%s/bitmap.dat", g_config_io->path_base_dialfs);

    get_comandos();

    int block_size = g_config_io->block_size;
    int block_count = g_config_io->block_count;
    int bitmap_size = block_count / 8;

    // Crear archivo de bloques
    blocks_fd = open(blocks_path, O_CREAT | O_RDWR, 0644);
    if (blocks_fd == -1) {
        log_error(g_logger, "Error al crear o abrir el archivo de bloques: %s", strerror(errno));
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
    //free de path?
    close(bitmap_fd);
    close(blocks_fd);
}

t_config* load_metadata(const char* filename) {
    char* metadata_path = string_from_format("%s/%s", g_config_io->path_base_dialfs, filename);
    return config_create(metadata_path);
}

void save_metadata(const char* filename, int initial_block, int file_size) {
    char* metadata_path = string_from_format("%s/%s", g_config_io->path_base_dialfs, filename);

    // Crear archivo de metadata si no existe
    FILE* file = fopen(metadata_path, "w");
    if (file == NULL) {
        log_error(g_logger, "Error al crear el archivo de metadata %s", metadata_path);
        free(metadata_path);
        return;
    }
    fclose(file);

    t_config* metadata = config_create(metadata_path);
    if (metadata == NULL) {
        log_error(g_logger, "Error al crear la configuración de metadata para %s", metadata_path);
        free(metadata_path);
        return;
    }

    char initial_block_str[10];
    char file_size_str[10];
    snprintf(initial_block_str, sizeof(initial_block_str), "%d", initial_block);
    snprintf(file_size_str, sizeof(file_size_str), "%d", file_size);

    config_set_value(metadata, "BLOQUE_INICIAL", initial_block_str);
    config_set_value(metadata, "TAMANIO_ARCHIVO", file_size_str);

    // Guardar configuración directamente en el archivo
    config_save_in_file(metadata, metadata_path);
    config_destroy(metadata);

    free(metadata_path);
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
    //Guardar Bitmap
    save_bitmap(bitmap);

    if (initial_block == -1) {
        printf("No hay bloques disponibles\n");
        return;
    }

    log_info(g_logger, "Creando archivo %s en el bloque %d", filename, initial_block);
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

    char* metadata_path = string_from_format("%s/%s", g_config_io->path_base_dialfs, filename);
    remove(metadata_path);
    config_destroy(metadata);
    free(metadata_path);
}

t_list* get_fcb_list() {
    t_list* fcb_list = list_create();
    DIR* dir = opendir(g_config_io->path_base_dialfs);
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {

            if(string_contains(entry->d_name, ".txt")){
                t_config* metadata = load_metadata(entry->d_name);
                t_fcb* fcb = malloc(sizeof(t_fcb));
                fcb->path = string_duplicate(entry->d_name);
                fcb->metadata = metadata;
                list_add(fcb_list,  fcb);
            }
        }
    }

    closedir(dir);
    return fcb_list;
}

void compactar_fs() {
    log_info(g_logger, "Iniciando compactación del sistema de archivos");

    t_list* fcbs = get_fcb_list();

    int tamanio_bloques_usados = 0;
    int bloques_libres = 0;
    int bloques_usados = 0;

    for (int i = 0; i < g_config_io->block_count; i++) {
        if (bitarray_test_bit(bitmap, i)) {
            bloques_usados++;
            tamanio_bloques_usados += g_config_io->block_size;
        } else {
            bloques_libres++;
        }
    }

    void* buffer = malloc(tamanio_bloques_usados);
    int offset = 0;

    t_list_iterator* it = list_iterator_create(fcbs);
    while(list_iterator_has_next(it)){
        t_fcb* fcb = list_iterator_next(it);
        int initial_block = config_get_int_value(fcb->metadata, "BLOQUE_INICIAL");
        int file_size = config_get_int_value(fcb->metadata, "TAMANIO_ARCHIVO");
        int blocks_to_read = (file_size + g_config_io->block_size - 1) / g_config_io->block_size;

        int new_initial_block = offset / g_config_io->block_size;

        for (int i = 0; i < blocks_to_read; i++) {
            lseek(blocks_fd, (initial_block + i) * g_config_io->block_size, SEEK_SET);
            read(blocks_fd, buffer + offset, g_config_io->block_size);
            offset += g_config_io->block_size;
        }

        save_metadata(fcb->path, new_initial_block, file_size);
    }

    list_iterator_destroy(it);

    for (int i = 0; i < bloques_usados; i++) {
        lseek(blocks_fd, i * g_config_io->block_size, SEEK_SET);
        write(blocks_fd, buffer + i * g_config_io->block_size, g_config_io->block_size);
        bitarray_set_bit(bitmap, i);
    }

    free(buffer);

    for (int i = bloques_usados; i < g_config_io->block_count; i++) {
        bitarray_clean_bit(bitmap, i);
    }



    save_bitmap(bitmap);
    usleep(g_config_io->retraso_compactacion * 1000);
    log_info(g_logger, "Compactación del sistema de archivos completada");
}

bool bloque_inicial_actual_puede_asignar_mas_de_forma_contigua(int bloque_inicial, int cantidad_bloques_actual, int bloques_necesarios){
    int bloque_final = bloque_inicial + cantidad_bloques_actual;
    int bloques_delta = bloques_necesarios - cantidad_bloques_actual;

    for(int i = bloque_final; i < bloque_final + bloques_delta; i++){
        if(bitarray_test_bit(bitmap, i)){
            return false;
        }
    }

    return true;
}

bool buscar_bloques_libres_contiguos(t_bitarray* bitmap, int block_count, int required_blocks, int* start_block) {
    int free_blocks = 0;
    *start_block = -1;

    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            if (*start_block == -1) {
                *start_block = i;
            }
            free_blocks++;
            if (free_blocks >= required_blocks) {
                return true; // Suficientes bloques libres encontrados
            }
        } else {
            *start_block = -1;
            free_blocks = 0;
        }
    }

    return false; // No se encontraron suficientes bloques libres
}

void liberar_bloques(int initial_block, int block_count) {
    for (int i = initial_block; i < initial_block + block_count; i++) {
        bitarray_clean_bit(bitmap, i);
    }
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
    old_blocks = old_size == 0 ? 1 : old_blocks;

    int new_blocks = (new_size + block_size - 1) / block_size;

    if (new_blocks > old_blocks) {
        int start_block = -1;

        if(
            !bloque_inicial_actual_puede_asignar_mas_de_forma_contigua(initial_block, old_blocks, new_blocks)
        ){
            if (!buscar_bloques_libres_contiguos(bitmap, g_config_io->block_count, new_blocks, &start_block)) {
                compactar_fs();

                if (!buscar_bloques_libres_contiguos(bitmap, g_config_io->block_count, new_blocks, &start_block)) {
                    log_info(g_logger,"No hay suficiente espacio libre después de la compactación\n");
                    return;
                }
            }

            liberar_bloques(initial_block, old_blocks);

            // Mover bloques viejos usados a bloques nuevos
            // deja persistidos los bloques en el archivo

            void* data = malloc(block_size);
            for (int i = 0; i < old_blocks; i++) {
                lseek(blocks_fd, (initial_block + i) * block_size, SEEK_SET);
                read(blocks_fd, data, block_size);

                lseek(blocks_fd, (start_block + i) * block_size, SEEK_SET);
                write(blocks_fd, data, block_size);
            }
            free(data);


        }else{
            start_block = initial_block;
        }


        for (int i = start_block; i < start_block + (new_blocks); i++) {
            bitarray_set_bit(bitmap, i);
        }

        save_bitmap(bitmap);
        initial_block = start_block;
    } else if (new_blocks < old_blocks) {
        for (int i = initial_block + new_blocks; i < initial_block + old_blocks; i++) {
            bitarray_clean_bit(bitmap, i);
        }
        save_bitmap(bitmap);
    }

    log_info(g_logger, "Truncando archivo %s de %d a %d bytes", filename, old_size, new_size);
    save_metadata(filename, initial_block, new_size);

    config_destroy(metadata);
}


void io_fs_write(t_instruccion_io* instruccion) {
    
    char* filename = instruccion->nombre_archivo;
    int size = instruccion->tamanio;
    int offset = instruccion->puntero_archivo;
    char* data = string_new();

    data = leer_de_memoria(g_socket_memoria,size, instruccion->peticionesMemoria, g_logger);
    
    log_info(g_logger, "String leido en memoria: %s", data);

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
    char* data = string_new();

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


    for (int i = block_start; i <= block_end; i++) {

        int read_size = g_config_io->block_size - block_offset;
        if (i == block_end) {
            read_size = (offset + size) % g_config_io->block_size;
        }

        lseek(blocks_fd, (initial_block + i) * g_config_io->block_size + block_offset, SEEK_SET);
        char* buffer = malloc(read_size + 1);
        read(blocks_fd, buffer, read_size);
        buffer[read_size] = '\0';
        string_append(&data, string_duplicate(buffer));

        free(buffer);
        block_offset = 0;
    }

    config_destroy(metadata);

    log_info(g_logger, "Leido de archivo %s: %s", filename, data);
    // Escribir en la memoria
    actualizar_peticiones_con_valor(instruccion->peticionesMemoria, data);

    
    //Ver si funciona por el g_socket_memoria o hay que pasarle un socket
    guardar_en_memoria(g_socket_memoria,data, instruccion->peticionesMemoria, g_logger);

    free(data);

}