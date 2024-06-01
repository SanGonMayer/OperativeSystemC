#ifndef  CONSOLA_INTERACTIVA_H
#define  CONSOLA_INTERACTIVA_H

typedef enum {
    EJECUTAR_SCRIPT= 1,
    INICIAR_PROCESO= 2,
    FINALIZAR_PROCESO= 3,
    DETENER_PLANIFICACION= 4,
    INICIAR_PLANIFIACION= 5,
    MULTIPROGRAMACION =6, 
    PROCESO_ESTADO= 7,
} t_funciones_consola;

void consola_interactiva();
void ejecutar_comando(t_funciones_consola comando, char** args);
char** leer_script(char* path);
void ejecutar_script(char** comandos);

#endif