#include "buffer.h"


// Crea un buffer vacío de tamaño size y offset 0
t_buffer *buffer_create(uint32_t size){
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = size;
    buffer->offset = 0;
    buffer->stream = malloc(size);
    return buffer;
}

t_buffer* recibir_buffer(int socket){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    recv(socket, &buffer->size, sizeof(uint32_t), MSG_WAITALL);
    buffer->stream = malloc(buffer->size);
    recv(socket, buffer->stream, buffer->size, MSG_WAITALL);
    buffer->offset = 0;
    return buffer;
}

// Libera la memoria asociada al buffer
void buffer_destroy(t_buffer *buffer){
    free(buffer->stream);
    free(buffer);
}

// Agrega un stream al buffer en la posición actual y avanza el offset
void buffer_add(t_buffer *buffer, void *data, uint32_t size){
    memcpy(buffer->stream + buffer->offset, data, size);
    buffer->offset += size;
}

// Guarda size bytes del principio del buffer en la dirección data y avanza el offset
void buffer_read(t_buffer *buffer, void *data, uint32_t size){
    memcpy(data, buffer->stream + buffer->offset, size);
    buffer->offset += size;
}

// Agrega un uint32_t al buffer
void buffer_add_uint32(t_buffer *buffer, uint32_t data){
    buffer_add(buffer, &data, sizeof(uint32_t));
}

// Lee un uint32_t del buffer y avanza el offset
uint32_t buffer_read_uint32(t_buffer *buffer){
    uint32_t data;
    buffer_read(buffer, &data, sizeof(uint32_t));
    return data;
}

// Agrega un uint8_t al buffer
void buffer_add_uint8(t_buffer *buffer, uint8_t data){
    buffer_add(buffer, &data, sizeof(uint8_t));
}

// Lee un uint8_t del buffer y avanza el offset
uint8_t buffer_read_uint8(t_buffer *buffer){
    uint8_t data;
    buffer_read(buffer, &data, sizeof(uint8_t));
    return data;
}

// Agrega string al buffer con un uint32_t adelante indicando su longitud
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string){
    buffer_add_uint32(buffer, length);
    buffer_add(buffer, string, length);
}

// Lee un string y su longitud del buffer y avanza el offset
char *buffer_read_string(t_buffer *buffer, uint32_t *length){
    *length = buffer_read_uint32(buffer);
    char *string = malloc(*length);
    buffer_read(buffer, string, *length);
    string[*length] = '\0';
    return string;
}