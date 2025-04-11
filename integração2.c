#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MEM_SIZE 256
#define INSTR_BITS 16
#define REG_COUNT 32
#define DATA_SIZE 256

typedef struct{
    char instr_mem[MEM_SIZE][INSTR_BITS+1]; // Matriz para armazenar as instruções
} Memory;

enum classe_inst{
    tipo_R, tipo_I, tipo_J, tipo_INVALIDO
};

struct instrucao{
    enum classe_inst tipo;
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
};

struct memoria_dados{
    int dados[DATA_SIZE];
};

struct pc{
    int current;
    int previous;
};

struct simulador{
    Memory imem;               // Memória de instruções
    struct memoria_dados dmem; // Memória de dados
    struct pc pc;              // Contador de programa
    int reg[REG_COUNT];        // Registradores
    int prog_size;             // Tamanho do programa carregado
    char nome_arquivo_saida[100]; // Nome do arquivo de saída
    struct estadosalvo ultimosalvo;
};

struct estadosalvo{
    int reg[REG_COUNT];
    int dados[DATA_SIZE];
    int pc;
    struct instrucao inst;
};

// Função para carregar instruções na memória
void load_memory(Memory *memory, const char *filename){
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
            strncpy(memory->instr_mem[i], line, INSTR_BITS);
            memory->instr_mem[i][INSTR_BITS] = '\0';
            i++;
        }
    }
    fclose(file);
}

int ula(int operacao, int a, int b){
    switch (operacao) {
        case 0: return a + b;   // ADD
        case 1: return a - b;   // SUB
        case 2: return a & b;   // AND
        case 3: return a | b;   // OR
        default: return 0;
    }
}

// Gera arquivo assembly
void criaasm(struct instrucao *inst, const char *nome_arquivo){
    FILE *arqasm = fopen(nome_arquivo, "a");
    if (!arqasm) {
        printf("Erro ao abrir arquivo de saída\n");
        return;
    }

    switch(inst->tipo) {
        case tipo_R:
            switch(inst->funct) {
                case 0: fprintf(arqasm, "add $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt); break;
                case 1: fprintf(arqasm, "sub $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt); break;
                case 2: fprintf(arqasm, "and $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt); break;
                case 3: fprintf(arqasm, "or $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt); break;
            }
            break;
            
        case tipo_I:
            switch(inst->opcode) {
                case 4: fprintf(arqasm, "lw $%d, %d($%d)\n", inst->rt, inst->imm, inst->rs); break;
                case 5: fprintf(arqasm, "sw $%d, %d($%d)\n", inst->rt, inst->imm, inst->rs); break;
                case 8: fprintf(arqasm, "addi $%d, $%d, %d\n", inst->rt, inst->rs, inst->imm); break;
                case 10: fprintf(arqasm, "beq $%d, $%d, %d\n", inst->rs, inst->rt, inst->imm); break;
            }
            break;
            
        case tipo_J:
            fprintf(arqasm, "j %d\n", inst->addr);
            break;
    }
    
    fclose(arqasm);
}

void decodificar(struct simulador *sim, struct instrucao *inst, int pc_pos){
    char *binario = sim->imem.instr_mem[pc_pos];
    char opcode_str[5];
    strncpy(opcode_str, binario, 4);
    opcode_str[4] = '\0';
    inst->opcode = strtol(opcode_str, NULL, 2);

    if (inst->opcode == 0){
        inst->tipo = tipo_R;
        char rs_str[4], rt_str[4], rd_str[4], funct_str[4];
        strncpy(rs_str, binario + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, binario + 7, 3); rt_str[3] = '\0';
        strncpy(rd_str, binario + 10, 3); rd_str[3] = '\0';
        strncpy(funct_str, binario + 13, 3); funct_str[3] = '\0';
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->rd = strtol(rd_str, NULL, 2);
        inst->funct = strtol(funct_str, NULL, 2);
    } 
    else if (inst->opcode == 2 || inst->opcode == 3) {
        inst->tipo = tipo_J;
        char addr_str[13];
        strncpy(addr_str, binario + 4, 12); addr_str[12] = '\0';
        inst->addr = strtol(addr_str, NULL, 2);
    } 
    else {
        inst->tipo = tipo_I;
        char rs_str[4], rt_str[4], imm_str[7];
        strncpy(rs_str, binario + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, binario + 7, 3); rt_str[3] = '\0';
        strncpy(imm_str, binario + 10, 6); imm_str[6] = '\0';
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->imm = strtol(imm_str, NULL, 2);
        
        if (inst->imm & 0x20) {
            inst->imm |= 0xFFFFFFC0;
        }
    }

    sim->ultimosalvo.inst = inst;
}

// Carrega um programa na memória de instruções
void load_program(struct simulador *sim, const char *filename){
    load_memory(&sim->imem, filename);
    
    // Conta quantas instruções válidas foram carregadas
    sim->prog_size = 0;
    while (sim->prog_size < MEM_SIZE && sim->imem.instr_mem[sim->prog_size][0] != '\0') {
        sim->prog_size++;
    }

    // Gera o arquivo .asm
    strncpy(sim->nome_arquivo_saida, filename, sizeof(sim->nome_arquivo_saida));
    char *ext = strrchr(sim->nome_arquivo_saida, '.');
    if (ext) *ext = '\0';
    strcat(sim->nome_arquivo_saida, ".asm");

    FILE *arqasm = fopen(sim->nome_arquivo_saida, "w");
    if (arqasm) fclose(arqasm);

    for (int i = 0; i < sim->prog_size; i++) {
        struct instrucao inst;
        decodificar(sim, &inst, i);
        criaasm(&inst, sim->nome_arquivo_saida);
    }

    printf("Programa carregado: %d instruções\n", sim->prog_size);
    printf("Arquivo .asm gerado: %s\n", sim->nome_arquivo_saida);
}

// Executa uma instrução
void executar_instrucao(struct simulador *sim){

    sim->ultimosalvo.pc = sim->pc.current;
    memcpy(sim->ultimosalvo.reg, sim->reg, sizeof(sim->reg));
    memcpy(sim->ultimosalvo.dados, sim->dmem.dados, sizeof(sim->dmem.dados));

    if (sim->pc.current >= sim->prog_size) {
        printf("Fim do programa\n");
        return;
    }

    struct instrucao inst;
    decodificar(sim, &inst, sim->pc.current);
    sim->pc.previous = sim->pc.current;

    printf("\n[PC=%d] %s > ", sim->pc.current, sim->imem.instr_mem[sim->pc.current]);

    switch(inst.tipo) {
        case tipo_R:
            sim->reg[inst.rd] = ula(inst.funct, sim->reg[inst.rs], sim->reg[inst.rt]);
            printf("R%d = R%d op R%d | Resultado: %d\n", 
                  inst.rd, inst.rs, inst.rt, sim->reg[inst.rd]);
            break;
            
        case tipo_I:
            switch(inst.opcode){
                case 4: // LW
                    sim->reg[inst.rt] = sim->dmem.dados[sim->reg[inst.rs] + inst.imm];
                    printf("LW R%d, %d(R%d) | Valor: %d\n", 
                          inst.rt, inst.imm, inst.rs, sim->reg[inst.rt]);
                    break;
                case 5: // SW
                    sim->dmem.dados[sim->reg[inst.rs] + inst.imm] = sim->reg[inst.rt];
                    printf("SW R%d, %d(R%d)\n", inst.rt, inst.imm, inst.rs);
                    break;
                case 8: // ADDI
                    sim->reg[inst.rt] = sim->reg[inst.rs] + inst.imm;
                    printf("ADDI R%d, R%d, %d | Resultado: %d\n",
                          inst.rt, inst.rs, inst.imm, sim->reg[inst.rt]);
                    break;
                case 10: // BEQ
                    printf("BEQ R%d, R%d, %d | ", inst.rs, inst.rt, inst.imm);
                    if (sim->reg[inst.rs] == sim->reg[inst.rt]) {
                        sim->pc.current += inst.imm;
                        printf("Desvio tomado, novo PC = %d\n", sim->pc.current);
                    } else {
                        printf("Desvio não tomado\n");
                    }
                    return; // Evita incrementar o PC duas vezes
            }
            break;
            
        case tipo_J:
            printf("J %d\n", inst.addr);
            sim->pc.current = inst.addr - 1; // -1 porque vai incrementar depois
            break;
            
        default:
            printf("Instrução não implementada\n");
    }

    sim->pc.current++;
}


void mostrar_registradores(struct simulador *sim){
    printf("\nRegistradores (não-zero):\n");
    for (int i = 0; i < REG_COUNT; i++) {
        if (sim->reg[i] != 0) {
            printf("R%-2d = %-10d (0x%08X)\n", i, sim->reg[i], sim->reg[i]);
        }
    }
}

// Mostra toda a memória de instruções
void imprimir_memoria_instrucao(struct simulador *sim){
    printf("\nMemória de Instruções (completa):\n");
    printf("Endereço  | Conteúdo (binário)\n");
    printf("----------|-------------------\n");
    
    for (int i = 0; i < MEM_SIZE; i++) {
        printf("  %3d     | ", i);
        if (i < sim->prog_size && sim->imem.instr_mem[i][0] != '\0'){
            printf("%s", sim->imem.instr_mem[i]);
        } else {
            printf("0000000000000000");
        }
        printf("\n");
    }
}

// Mostra toda a memória de dados
void imprimir_memoria_dados(struct simulador *sim){
    printf("\nMemória de Dados (completa):\n");
    printf("Endereço  | Valor (decimal)\n");
    printf("----------|----------------\n");
    
    for (int i = 0; i < DATA_SIZE; i++) {
        printf("  %3d     | %d\n", i, sim->dmem.dados[i]);
    }
}

// Salva as instruções executadas em arquivo
void salvar_memoria_instrucao_arquivo(struct simulador *sim){
    char nome_arquivo[100];
    printf("Digite o nome do arquivo para salvar as instruções executadas: ");
    scanf("%99s", nome_arquivo);

    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo){
        printf("Erro ao criar arquivo de saída!\n");
        return;
    }

    for (int i = 0; i < sim->pc.current && i < sim->prog_size; i++){
        fprintf(arquivo, "%s\n", sim->imem.instr_mem[i]);
    }

    fclose(arquivo);
    printf("Instruções executadas (até PC=%d) salvas em '%s'\n", sim->pc.current, nome_arquivo);
}

// Salva a memória de dados em arquivo
void salvar_memoria_dados_arquivo(struct simulador *sim){
    char nome_arquivo[100];
    printf("Digite o nome do arquivo para salvar a memória de dados: ");
    scanf("%99s", nome_arquivo);

    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) {
        printf("Erro ao criar arquivo de saída!\n");
        return;
    }

    for (int i = 0; i < DATA_SIZE; i++) {
        fprintf(arquivo, "%d\n", sim->dmem.dados[i]);
    }

    fclose(arquivo);
    printf("Memória de dados salva em '%s'\n", nome_arquivo);
}

void back(struct simulador *sim){
    printf("\n--- Reexecução da última instrução ---\n");

    memcpy(sim->reg, sim->ultimosalvo.reg, sizeof(sim->reg));
    memcpy(sim->dmem.dados, sim->ultimosalvo.dados, sizeof(sim->dmem.dados));
    sim->pc.current = sim->ultimosalvo.pc;

    printf("Estado restaurado para PC = %d\n", sim->pc.current);

    executar_instrucao(sim);
}

// Menu principal 
void menu_principal(struct simulador *sim){
    char filename[100];
    char opcao;
    
    printf("Simulador MIPS - Modo Interativo\n");
    printf("--------------------------------\n");
    
    printf("Digite o nome do arquivo de instruções: ");
    scanf("%99s", filename);
    load_program(sim, filename);
    
    do{
        printf("\nMenu:\n");
        printf("1. Executar próxima instrução\n");
        printf("2. Mostrar registradores\n");
        printf("3. Mostrar memória de instruções\n");
        printf("4. Mostrar memória de dados\n");
        printf("5. Salvar instruções executadas em arquivo\n");
        printf("6. Salvar memória de dados em arquivo\n");
        printf("7. Voltar e reexecutar última instrução\n");
        printf("8. Sair\n");
        printf("Opção: ");
        scanf(" %c", &opcao);
        
        switch(tolower(opcao)) {
            case '1':
                executar_instrucao(sim);
                break;
            case '2':
                mostrar_registradores(sim);
                break;
            case '3':
                imprimir_memoria_instrucao(sim);
                break;
            case '4':
                imprimir_memoria_dados(sim);
                break;
            case '5':
                salvar_memoria_instrucao_arquivo(sim);
                break;
            case '6':
                salvar_memoria_dados_arquivo(sim);
                break;
            case '7':
                back(sim);
            break;
            case '8':
                printf("Encerrando simulador. Arquivo .asm gerado: %s\n", sim->nome_arquivo_saida);
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while(tolower(opcao) != '8');
}

int main(){
    struct simulador sim = {0}; // Inicializa tudo com zero
    menu_principal(&sim);
    return 0;
}
