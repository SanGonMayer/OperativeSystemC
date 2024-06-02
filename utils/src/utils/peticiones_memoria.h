#define PETICIONES_MEMORIA_H_
#ifdef  PETICIONES_MEMORIA_H_
#include "utils/buffer.h"

typedef struct {
    uint32_t pid;
    int pagina;
}t_peticion_marco;

t_peticion_marco* crear_peticion_marco(uint32_t pid, int pagina);
void destruir_peticion_marco(t_peticion_marco* peticion);
t_buffer* serializar_peticion_marco(t_peticion_marco* peticion);
t_peticion_marco* deserializar_peticion_marco(t_buffer* buffer);


typedef struct{
    uint32_t pid;
    int tamanio;
} t_peticion_resize;

t_peticion_resize* crear_peticion_resize(uint32_t pid, int tamanio);
void destruir_peticion_resize(t_peticion_resize* peticion);
t_buffer* serializar_peticion_resize(t_peticion_resize* peticion);
t_peticion_resize* deserializar_peticion_resize(t_buffer* buffer);

typedef enum {
    LECTURA = 1,
    ESCRITURA = 2
} t_tipo_acceso;

typedef struct{
    uint32_t pid;
    t_tipo_acceso tipo_acceso;
    int direccion_fisica;
    char* string;
} t_peticion_acceso_usuario;

t_peticion_acceso_usuario* crear_peticion_lectura(uint32_t pid, int direccion_fisica);
t_peticion_acceso_usuario* crear_peticion_escritura(uint32_t pid, int direccion_fisica, char* string);
void destruir_peticion_acceso_usuario(t_peticion_acceso_usuario* peticion);
t_buffer* serializar_peticion_acceso_usuario(t_peticion_acceso_usuario* peticion);
t_peticion_acceso_usuario* deserializar_peticion_acceso_usuario(t_buffer* buffer);

typedef struct {
    uint32_t pid;
} t_peticion_finalizar_proceso;

t_peticion_finalizar_proceso* crear_peticion_finalizar_proceso(uint32_t pid);
void destruir_peticion_finalizar_proceso(t_peticion_finalizar_proceso* peticion);
t_buffer* serializar_peticion_finalizar_proceso(t_peticion_finalizar_proceso* peticion);
t_peticion_finalizar_proceso* deserializar_peticion_finalizar_proceso(t_buffer* buffer);

#endif