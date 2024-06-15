#include "consola_interactiva.h"
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <readline/readline.h>
#include "global_kernel.h"
#include "kernel.h"
#include "utils/files.h"
#include <readline/history.h>

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

	while (!string_equals_ignore_case(linea_leida, "q")){

        char ** linea_leida_separada = string_split(linea_leida, " ");

        char *funcion = string_duplicate(linea_leida_separada[0]);
        string_to_upper(funcion);

        int opcion_funciones_consola;

        t_dictionary* comandos = get_comandos();

        if(dictionary_has_key(comandos, funcion)){

            add_history(linea_leida);

            opcion_funciones_consola = (int) dictionary_get(comandos, funcion);
            ejecutar_comando(opcion_funciones_consola, linea_leida_separada);
        }else{
            log_error(g_logger, "ingresaste una funcion no valida");
        }
        
        linea_leida = readline(">");

        free(funcion);
        for(int i = 0; linea_leida_separada[i] != NULL; i++){
            free(linea_leida_separada[i]);
        }
        free(linea_leida_separada);
	}

	free(linea_leida);
}

char** leer_script(char* path){
    
    char* script = leer_archivo_txt(path);
    char** comandos = string_split(script, "\n");

    return comandos;
}

void ejecutar_script(char** comandos){
    for(int i = 0; comandos[i] != NULL; i++){
        char** comando = string_split(comandos[i], " ");
        char* funcion = comando[0];
        string_to_upper(funcion);

        int opcion_funciones_consola;

        t_dictionary* comandos_diccionario = get_comandos();

        if(dictionary_has_key(comandos_diccionario, funcion)){
            opcion_funciones_consola = (int) dictionary_get(comandos_diccionario, funcion);
        }else{
            log_error(g_logger, "ingresaste una funcion no valida");
            break;
        }
        char ** args =  comando;
        ejecutar_comando(opcion_funciones_consola, args);
    }
}



void ejecutar_comando(t_funciones_consola comando, char** args){
    switch (comando) {

        case EJECUTAR_SCRIPT:
            printf("e\n");
            char** comandos = leer_script(args[1]);
            ejecutar_script(comandos);
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
            printf("Se seleccionó la opción Finalizar Proceso\n");
            uint32_t pid = (uint32_t)atoi(args[1]);
            t_PCB* pcb = buscar_pid_en_sistema(pid);
            if(pcb == NULL){
                log_info(g_logger, "El PCB con pid %d no fue encontrado", pid);
            } else {
                log_info(g_logger, "El PCB con pid %d fue encontrado con exito", pid);
                finalizar_proceso(pcb);
            }
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
            listar_procesos();
            break;
    }
}