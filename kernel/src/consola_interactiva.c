

#include "consola_interactiva.h"
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <readline/readline.h>
#include "global_kernel.h"
#include "kernel.h"

t_dictionary* get_comandos(){

    t_dictionary* comandos = dictionary_create();

    dictionary_put(comandos, "EJECUTAR_SCRIPT", (void*) EJECUTAR_SCRIPT);
    dictionary_put(comandos, "INICIAR_PROCESO", (void*) INICIAR_PROCESO);
    dictionary_put(comandos, "FINALIZAR_PROCESO", (void*) FINALIZAR_PROCESO);
    dictionary_put(comandos, "DETENER_PLANIFICACION", (void*) DETENER_PLANIFICACION);
    dictionary_put(comandos, "INICIAR_PLANIFICACION", (void*) INICIAR_PLANIFIACION);
    dictionary_put(comandos, "MULTIPROGRAMACION", (void*) MULTIPROGRAMACION);
    dictionary_put(comandos, "PROCESO_ESTADO", (void*) PROCESO_ESTADO);

    return comandos;
}

void consola_interactiva(){

    char* linea_leida;
    
	linea_leida = readline(">");

	while (strcmp(linea_leida, "q")){

    char ** linea_leida_separada = string_split(linea_leida, " ");

    char *funcion = linea_leida_separada[0];
    string_to_upper(funcion);

    int opcion_funciones_consola;

    // t_dictionary* comandos = get_comandos();

    // if(dictionary_has_key(comandos, funcion)){
    //     opcion_funciones_consola = (int) dictionary_get(comandos, funcion);

    if (strcmp(funcion, "EJECUTAR_SCRIPT") == 0) {
        opcion_funciones_consola = EJECUTAR_SCRIPT;
    } else if (strcmp(funcion, "INICIAR_PROCESO") == 0) {
        opcion_funciones_consola = INICIAR_PROCESO;
    } else if (strcmp(funcion, "FINALIZAR_PROCESO") == 0) {
        opcion_funciones_consola = FINALIZAR_PROCESO;
    } else if (strcmp(funcion, "DETENER_PLANIFICACION") == 0) {
        opcion_funciones_consola = DETENER_PLANIFICACION;
    } else if (strcmp(funcion, "INICIAR_PLANIFICACION") == 0) {
        opcion_funciones_consola = INICIAR_PLANIFIACION;
    } else if (strcmp(funcion, "MULTIPROGRAMACION") == 0) {
        opcion_funciones_consola = MULTIPROGRAMACION;
    } else if (strcmp(funcion, "PROCESO_ESTADO") == 0) {
        opcion_funciones_consola = PROCESO_ESTADO;
    }else
        log_error(g_logger, "ingresaste una funcion no valida");

    switch (opcion_funciones_consola) {

        case EJECUTAR_SCRIPT:
            printf("e\n");
            
            break;
        case INICIAR_PROCESO:
            printf("Se seleccionó la opción 2\n");
            char* path = linea_leida_separada[1];
            
            string_trim(&path);

            if(!es_parametro_valido(path)){
                log_error(g_logger, "El path ingresado no es válido");
                break;
            }

            pthread_t hilo_iniciar_proceso;
            pthread_create(&hilo_iniciar_proceso, NULL, (void*)iniciar_proceso, path);
            pthread_detach(hilo_iniciar_proceso);
            break;
        case FINALIZAR_PROCESO:
            printf("Se seleccionó la opción 3\n");
            break;
        case DETENER_PLANIFICACION:
            printf("Opción no válida\n");
            break;
        case INICIAR_PLANIFIACION:
            printf("Opción no válida\n");
            break;
        case MULTIPROGRAMACION:
            printf("Opción no válida\n");
            break;
        case PROCESO_ESTADO:
            printf("Opción no válida\n");
            break;
    }


        log_info(g_logger, "me llego la instruccion: %s", linea_leida);
		//log_info(logger, linea_leida);

		linea_leida = readline(">");

        free(funcion);
	}

	free(linea_leida);
    
}