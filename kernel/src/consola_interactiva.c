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

        t_dictionary* comandos = get_comandos();

        if(dictionary_has_key(comandos, funcion)){
            opcion_funciones_consola = (int) dictionary_get(comandos, funcion);
        }else{
            log_error(g_logger, "ingresaste una funcion no valida");
            break;
        }
        ejecutar_comando(opcion_funciones_consola, linea_leida_separada);
        
        log_info(g_logger, "me llego la instruccion: %s", linea_leida);

        linea_leida = readline(">");

        free(funcion);
	}

	free(linea_leida);
}


void ejecutar_comando(t_funciones_consola comando, char** args){
    switch (comando) {

        case EJECUTAR_SCRIPT:
            printf("e\n");
            // char** comandos = leer_script(args[1]);
            // ejecutar_script(comandos);
            break;
        case INICIAR_PROCESO:
            printf("Se seleccionó la opción 2\n");
            char* path = args[1];
            
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
            printf("Opción Detener planificacion\n");
            
            sem_wait(&g_notif_corto_plazo);
            sem_wait(&g_notif_largo_plazo);
            g_planificacion_pausada = 1;

            break;
        
        case INICIAR_PLANIFIACION:
            printf("Opción no válida\n");

            if (g_planificacion_pausada == 0){
                printf("No es posible detener planificacion");
                break;
            }

            g_planificacion_pausada = 1;
            sem_post(&g_notif_corto_plazo);
            sem_post(&g_notif_largo_plazo);
            
            break;
        case MULTIPROGRAMACION:
            printf("Opción no válida\n");
            break;
        case PROCESO_ESTADO:
            
            //listar_procesos

            break;
    }
}