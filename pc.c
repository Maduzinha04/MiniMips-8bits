#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MEM_SIZE 256
#define INSTR_BITS 16
#define DATA_SIZE 256
#define REG_COUNT 16

typedef struct {
    char instr_mem[MEM_SIZE][INSTR_BITS+1];
    int instr_count;
    int data_mem[DATA_SIZE];
    int reg[REG_COUNT];
    struct {
        int current;
        int previous;
    } pc;
} Computer;

void init_computer(Computer *comp) {
    memset(comp, 0, sizeof(Computer));
}

void load_program(Computer *comp, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    char line[INSTR_BITS+2];
    int i = 0;
    
    while (fgets(line, sizeof(line), file) && i < MEM_SIZE) {
        line[strcspn(line, "\n")] = '\0';
        
        if (strlen(line) == INSTR_BITS) {
            strncpy(comp->instr_mem[i], line, INSTR_BITS);
            comp->instr_mem[i][INSTR_BITS] = '\0';
            i++;
        }
    }
    
    comp->instr_count = i;
    fclose(file);
}

void print_status(Computer *comp) {
    printf("\n--- Estado Atual ---\n");
    printf("PC: %d\n", comp->pc.current);
    printf("Registradores:\n");
    for (int i = 0; i < REG_COUNT; i++) {
        if (comp->reg[i] != 0) {
            printf("  R%d = %d (0x%08X)\n", i, comp->reg[i], comp->reg[i]);
        }
    }
    printf("Memória de Dados (não-zero):\n");
    for (int i = 0; i < DATA_SIZE; i++) {
        if (comp->data_mem[i] != 0) {
            printf("  mem[%d] = %d (0x%08X)\n", i, comp->data_mem[i], comp->data_mem[i]);
        }
    }
    printf("-------------------\n\n");
}

int execute_step(Computer *comp) {
    if (comp->pc.current >= comp->instr_count) {
        printf("Fim do programa alcançado.\n");
        return 0;
    }

    char current_instr[INSTR_BITS+1];
    strcpy(current_instr, comp->instr_mem[comp->pc.current]);
    
    printf("Próxima instrução (PC=%d): %s\n", comp->pc.current, current_instr);
    
    // Extrai opcode (primeiros 4 bits)
    char opcode_str[5] = {0};
    strncpy(opcode_str, current_instr, 4);
    int opcode = strtol(opcode_str, NULL, 2);
    
    comp->pc.previous = comp->pc.current;
    comp->pc.current++;
    
    switch(opcode) {
        case 0: { // Tipo R
            char rd_str[4], rs_str[4], rt_str[4], funct_str[4];
            strncpy(rd_str, current_instr + 4, 3); rd_str[3] = '\0';
            strncpy(rs_str, current_instr + 7, 3); rs_str[3] = '\0';
            strncpy(rt_str, current_instr + 10, 3); rt_str[3] = '\0';
            strncpy(funct_str, current_instr + 13, 3); funct_str[3] = '\0';
            
            int rd = strtol(rd_str, NULL, 2);
            int rs = strtol(rs_str, NULL, 2);
            int rt = strtol(rt_str, NULL, 2);
            int funct = strtol(funct_str, NULL, 2);
            
            if (funct == 0) { // ADD
                comp->reg[rd] = comp->reg[rs] + comp->reg[rt];
                printf("EXEC: ADD R%d, R%d, R%d\n", rd, rs, rt);
            } else {
                printf("ERRO: Função R-type não implementada: %d\n", funct);
            }
            break;
        }
        case 4: { // LW
            char rt_str[4], rs_str[4], imm_str[7];
            strncpy(rt_str, current_instr + 4, 3); rt_str[3] = '\0';
            strncpy(rs_str, current_instr + 7, 3); rs_str[3] = '\0';
            strncpy(imm_str, current_instr + 10, 6); imm_str[6] = '\0';
            
            int rt = strtol(rt_str, NULL, 2);
            int rs = strtol(rs_str, NULL, 2);
            int imm = strtol(imm_str, NULL, 2);
            
            comp->reg[rt] = comp->data_mem[comp->reg[rs] + imm];
            printf("EXEC: LW R%d, %d(R%d)\n", rt, imm, rs);
            break;
        }
        case 5: { // SW
            char rt_str[4], rs_str[4], imm_str[7];
            strncpy(rt_str, current_instr + 4, 3); rt_str[3] = '\0';
            strncpy(rs_str, current_instr + 7, 3); rs_str[3] = '\0';
            strncpy(imm_str, current_instr + 10, 6); imm_str[6] = '\0';
            
            int rt = strtol(rt_str, NULL, 2);
            int rs = strtol(rs_str, NULL, 2);
            int imm = strtol(imm_str, NULL, 2);
            
            comp->data_mem[comp->reg[rs] + imm] = comp->reg[rt];
            printf("EXEC: SW R%d, %d(R%d)\n", rt, imm, rs);
            break;
        }
        case 2: { // J
            char addr_str[13];
            strncpy(addr_str, current_instr + 4, 12); addr_str[12] = '\0';
            int addr = strtol(addr_str, NULL, 2);
            
            comp->pc.current = addr;
            printf("EXEC: J %d\n", addr);
            break;
        }
        default:
            printf("ERRO: Instrução não implementada (opcode %d)\n", opcode);
    }
    
    print_status(comp);
    return 1;
}

void run_interactive(Computer *comp) {
    printf("\nModo interativo ativado. Digite:\n");
    printf("  's' - Executar próxima instrução\n");
    printf("  'q' - Sair do programa\n");
    
    char input[10];
    while (1) {
        printf("\n> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        
        if (tolower(input[0]) == 'q') {
            printf("Execução interrompida pelo usuário.\n");
            break;
        }
        
        if (tolower(input[0]) == 's') {
            if (!execute_step(comp)) {
                break;
            }
        } else {
            printf("Comando inválido. Use 's' ou 'q'.\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_instrucoes.txt>\n", argv[0]);
        return 1;
    }

    Computer my_computer;
    init_computer(&my_computer);
    
    // Pré-carrega alguns valores de teste
    my_computer.reg[0] = 0;  // R0 sempre zero
    my_computer.reg[1] = 10;
    my_computer.data_mem[5] = 123;
    my_computer.data_mem[10] = 456;
    
    load_program(&my_computer, argv[1]);
    run_interactive(&my_computer);

    return 0;
}