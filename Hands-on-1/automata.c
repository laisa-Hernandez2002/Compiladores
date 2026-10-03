#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


typedef enum {
    T_MOV, T_ADD, T_STO, T_END, 
    T_AL, T_BL, T_COMA, T_CL, T_NUM, 
    T_EOF, T_ERR
} TokenType;

typedef struct {
    TokenType type;
    char lexema[20];
    int valor_num;
} Token;

typedef struct {
    char operacion[10];
    char registros[2][5];
    int num_registros;
    int direccion; 
} Instruccion;


Token get_next_token(const char **str) {
    Token t;
    t.valor_num = -1;
    t.lexema[0] = '\0';
    
    
    while (**str == ' ' || **str == '\t') (*str)++;
    
    if (**str == '\0' || **str == '\n') {
        t.type = T_EOF;
        strcpy(t.lexema, "EOF");
        return t;
    }
    
    if (**str == ',') {
        t.type = T_COMA;
        strcpy(t.lexema, ",");
        (*str)++;
        return t;
    }
    
    if (isdigit(**str)) {
        t.type = T_NUM;
        int i = 0;
        while (isdigit(**str)) {
            t.lexema[i++] = **str;
            (*str)++;
        }
        t.lexema[i] = '\0';
        t.valor_num = atoi(t.lexema);
        return t;
    }
    
    if (isalpha(**str)) {
        int i = 0;
        while (isalpha(**str)) {
            t.lexema[i++] = **str;
            (*str)++;
        }
        t.lexema[i] = '\0';
        
        if (strcmp(t.lexema, "MOV") == 0) t.type = T_MOV;
        else if (strcmp(t.lexema, "ADD") == 0) t.type = T_ADD;
        else if (strcmp(t.lexema, "STO") == 0) t.type = T_STO;
        else if (strcmp(t.lexema, "END") == 0) t.type = T_END;
        else if (strcmp(t.lexema, "AL") == 0) t.type = T_AL;
        else if (strcmp(t.lexema, "BL") == 0) t.type = T_BL;
        else t.type = T_ERR;
        return t;
    }
    
    t.type = T_ERR;
    t.lexema[0] = **str;
    t.lexema[1] = '\0';
    (*str)++;
    return t;
}


int procesar_instruccion(const char *entrada, Instruccion *inst) {
    int estado = 0; // q0
    const char *p = entrada;
    Token t;
    
    
    strcpy(inst->operacion, "");
    inst->num_registros = 0;
    inst->direccion = -1;
    
    printf("VALIDACION MEDIANTE AFD\n");
    
    
    Token historial[10];
    int num_tokens = 0;

    while (1) {
        t = get_next_token(&p);
        if (t.type != T_EOF && t.type != T_ERR) {
            historial[num_tokens++] = t;
        }

        switch (estado) {
            case 0: 
                if (t.type == T_MOV) { estado = 1; strcpy(inst->operacion, "MOV"); }
                else if (t.type == T_ADD) { estado = 2; strcpy(inst->operacion, "ADD"); }
                else if (t.type == T_STO) { estado = 3; strcpy(inst->operacion, "STO"); }
                else if (t.type == T_END) { estado = 4; strcpy(inst->operacion, "END"); }
                else goto error;
                break;
                
            case 1: 
                if (t.type == T_AL || t.type == T_BL) { 
                    estado = 5; 
                    strcpy(inst->registros[inst->num_registros++], t.lexema);
                } else goto error_falta_registro;
                break;
                
            case 2: 
                if (t.type == T_AL) {
                    estado = 8;
                    strcpy(inst->registros[inst->num_registros++], t.lexema);
                } else goto error;
                break;
                

                
            case 3: 
                if (t.type == T_NUM) {
                    estado = 11; 
                    inst->direccion = t.valor_num;
                } else goto error;
                break;
                
            case 4: 
                if (t.type == T_EOF) goto accept;
                else goto error;
                break;
                
            case 5: 
                if (t.type == T_COMA) estado = 6;
                else goto error_falta_coma;
                break;
                
            case 6: 
                if (t.type == T_NUM) {
                    estado = 7; 
                    inst->direccion = t.valor_num;
                } else goto error;
                break;
                
            case 7: 
            case 10: 
            case 11: 
                if (t.type == T_EOF) goto accept;
                else goto error;
                break;
                
            case 8: 
                if (t.type == T_COMA) estado = 9;
                else goto error_falta_coma;
                break;
                
            case 9: 
                if (t.type == T_BL) {
                    estado = 10; 
                    strcpy(inst->registros[inst->num_registros++], t.lexema);
                } else goto error;
                break;

            case 12: 
                if (t.type == T_CL) {
                    estado = 8;
                    strcpy(inst->registros[inst->num_registros++], t.lexema);
                } else goto error;
                break;



                                

                
        }
    }

accept:
    printf("Instruccion valida.\n\nTOKENS RECONOCIDOS\n");
    for (int i = 0; i < num_tokens; i++) {
        const char *nombre = "";
        if (historial[i].type == T_MOV || historial[i].type == T_ADD || historial[i].type == T_STO || historial[i].type == T_END) nombre = historial[i].lexema;
        else if (historial[i].type == T_AL || historial[i].type == T_BL) nombre = "REGISTRO";
        else if (historial[i].type == T_COMA) nombre = "COMA";
        else if (historial[i].type == T_NUM) nombre = "NUMERO";
        printf("%s(\"%s\")\n", nombre, historial[i].lexema);
    }
    
    printf("\nCOMPONENTES IDENTIFICADOS\n");
    printf("Mnemonico: %s\n", inst->operacion);
    for(int i=0; i < inst->num_registros; i++) {
        printf("Registro %s: %s\n", i==0 ? "destino/1" : "origen/2", inst->registros[i]);
    }
    if(inst->direccion != -1) {
        printf("Direccion de memoria: %d\n", inst->direccion);
        printf("Direccionamiento: directo\n");
    }
    
    printf("\nOBJETO INSTRUCCION\n{\n");
    printf("  operacion: \"%s\",\n", inst->operacion);
    printf("  registros: [");
    for(int i=0; i<inst->num_registros; i++) {
        printf("\"%s\"%s", inst->registros[i], i < inst->num_registros-1 ? ", " : "");
    }
    printf("],\n");
    if(inst->direccion != -1) printf("  direccion: %d\n", inst->direccion);
    else printf("  direccion: null\n");
    printf("}\n");
    
    return 1; 

error_falta_coma:
    printf("Instruccion invalida: falta la coma entre el registro y la direccion.\nNo se construye el objeto instruccion.\nNo se generan microoperaciones.\n");
    return 0;
error_falta_registro:
    printf("Instruccion invalida: registro no valido o faltante.\nNo se construye el objeto instruccion.\nNo se generan microoperaciones.\n");
    return 0;
error:
    printf("Instruccion invalida: sintaxis incorrecta o token inesperado.\nNo se construye el objeto instruccion.\nNo se generan microoperaciones.\n");
    return 0;
}


void maquina_moore(Instruccion *inst) {
    printf("\nMICROOPERACIONES GENERADAS POR MOORE\n");
    int paso = 1;
    
    if (strcmp(inst->operacion, "MOV") == 0) {
        
        printf("%d. MAR <- %d\n", paso++, inst->direccion);
        
        printf("%d. MBR <- M[MAR]\n", paso++);
        
        printf("%d. %s <- MBR\n", paso++, inst->registros[0]);
    } 
    else if (strcmp(inst->operacion, "ADD") == 0) {
        
        printf("%d. ACC <- %s + %s\n", paso++, inst->registros[0], inst->registros[1]);
    }
    else if (strcmp(inst->operacion, "STO") == 0) {
        
        printf("%d. MAR <- %d\n", paso++, inst->direccion);
        
        printf("%d. MBR <- ACC\n", paso++);
        
        printf("%d. M[MAR] <- MBR\n", paso++);
    }
    else if (strcmp(inst->operacion, "END") == 0) {
        
        printf("%d. HALT <- 1\n", paso++);
    }
    
    printf("generacion terminada.");
}


int main() {
    char entrada[100];
    Instruccion inst;
    
    printf("--- Simulador de Analisis Lexico y Automata ---\n");
    printf("(Escribe 'salir' para terminar el programa)\n\n");
    
    while (1) {
        printf("\nIntroduce una instruccion: ");
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) break;
        
        
        entrada[strcspn(entrada, "\n")] = 0;
        
        
        if (strcmp(entrada, "salir") == 0 || strcmp(entrada, "SALIR") == 0) break;
        
        
        if (strlen(entrada) == 0) continue;

        printf("\nENTRADA\n%s\n", entrada);
        
        if (procesar_instruccion(entrada, &inst)) {
            maquina_moore(&inst);
        }
    }
    
    return 0;
}