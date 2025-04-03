#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256
#define INSTR_BITS 16
#define Registradores 8


// Tipos de instruções (R, I, J)
enum classe_inst {
    tipo_R, tipo_I, tipo_J, tipo_INVALIDO
};

// Estrutura para armazenar uma instrução decodificada
struct instrucao {
    enum classe_inst tipo;
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
    char binario[INSTR_BITS + 1]; // Guarda a instrução em binário
};

// Estrutura para a memória de instruções
typedef struct {
    char instr_mem[MEM_SIZE][INSTR_BITS + 1];
} Memory;

Memory mem; // Variável global para armazenar as instruções

// Decodifica uma instrução de 16 bits
void decodificar(const char *inst_str, struct instrucao *inst) {
    strncpy(inst->binario, inst_str, INSTR_BITS);
    inst->binario[INSTR_BITS] = '\0';

    char opcode_str[5];
    strncpy(opcode_str, inst_str, 4);
    opcode_str[4] = '\0';
    inst->opcode = strtol(opcode_str, NULL, 2);

    if (inst->opcode == 0) {
        inst->tipo = tipo_R;
        char rs_str[4], rt_str[4], rd_str[4], funct_str[4];
        strncpy(rs_str, inst_str + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst_str + 7, 3); rt_str[3] = '\0';
        strncpy(rd_str, inst_str + 10, 3); rd_str[3] = '\0';
        strncpy(funct_str, inst_str + 13, 3); funct_str[3] = '\0';
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->rd = strtol(rd_str, NULL, 2);
        inst->funct = strtol(funct_str, NULL, 2);
    } 
    else if (inst->opcode == 2 || inst->opcode == 3) {
        inst->tipo = tipo_J;
        char addr_str[13];
        strncpy(addr_str, inst_str + 4, 12); addr_str[12] = '\0';
        inst->addr = strtol(addr_str, NULL, 2);
    } 
    else {
        inst->tipo = tipo_I;
        char rs_str[4], rt_str[4], imm_str[7];
        strncpy(rs_str, inst_str + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst_str + 7, 3); rt_str[3] = '\0';
        strncpy(imm_str, inst_str + 10, 6); imm_str[6] = '\0';
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->imm = strtol(imm_str, NULL, 2);
    }
}

// Imprime uma instrução decodificada
void print_instrucao(const struct instrucao *inst) {
    printf("Binário: %s\n", inst->binario);
    printf("Opcode: %d | Tipo: ", inst->opcode);

    switch (inst->tipo) {
        case tipo_R:
            printf("R | rs: %d, rt: %d, rd: %d, funct: %d\n", inst->rs, inst->rt, inst->rd, inst->funct);
            break;
        case tipo_I:
            printf("I | rs: %d, rt: %d, imm: %d\n", inst->rs, inst->rt, inst->imm);
            break;
        case tipo_J:
            printf("J | addr: %d\n", inst->addr);
            break;
        default:
            printf("Inválido\n");
    }
    printf("------------------------\n");
}

// Carrega instruções do arquivo para a memória
void load_memory() {
    FILE *file = fopen("teste.mem", "r");
    if (!file) {
        perror("Erro ao abrir teste.mem");
        return;
    }

    char linha[INSTR_BITS + 2];
    int i = 0;

    while (fgets(linha, sizeof(linha), file) && i < MEM_SIZE) {
        linha[strcspn(linha, "\n")] = '\0';
        if (linha[0] == '#' || linha[0] == '\0') continue;

        if (strlen(linha) == INSTR_BITS) {
            strncpy(mem.instr_mem[i], linha, INSTR_BITS);
            mem.instr_mem[i][INSTR_BITS] = '\0';
            i++;
        }
    }

    fclose(file);
    printf("Instruções carregadas: %d\n", i);
}

// Imprime a memória de instruções
void imprimimemoriadeinstrucoes() {
    printf("\n=== Memória de Instruções ===\n");
    for (int i = 0; i < MEM_SIZE && mem.instr_mem[i][0] != '\0'; i++) {
        printf("[%03d] %s\n", i, mem.instr_mem[i]);
        struct instrucao inst;
        decodificar(mem.instr_mem[i], &inst);
        print_instrucao(&inst);
    }
}

// Menu principal
void mostrar_menu() {
    printf("\n**************Menu principal************\n");
    printf("1. Carregar memória de instruções (.mem)\n");
    printf("2. Imprimir memória de instruções\n");
    printf("3. Encerrar o programa\n");
    printf("Opção: ");
}

int main() {
    int op = 0;
    
    do {
        mostrar_menu();
        scanf("%d", &op);
        
        switch(op) {
            case 1: 
                load_memory();
                break;
            case 2: 
                imprimimemoriadeinstrucoes(); 
                break;
            case 3: 
                printf("Encerrando...\n"); 
                break;
            default: 
                printf("Opção inválida!\n");
        }
    } while(op != 3);
    
    return 0;
}