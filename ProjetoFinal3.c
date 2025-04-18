#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256
#define INSTR_BITS 16
#define DATA_SIZE 256
#define REG_COUNT 8
#define END_OPCODE 255
#define OVERFLOW_FLAG 1
#define BEQ_FLAG 0
#define NO_FLAG -1


// Tipos de instruções
enum classe_inst { tipo_R, tipo_I, tipo_J, tipo_INVALIDO };

// Operações da ULA
enum ops_ula { ULA_ADD, ULA_SUB, ULA_AND, ULA_OR};

// Estrutura de uma instrução decodificada
struct instrucao {
    enum classe_inst tipo;
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
    char binario[INSTR_BITS + 1];
};

// Memória de instruções
typedef struct {
    char instr_mem[MEM_SIZE][INSTR_BITS+1];
    struct instrucao decodificadas[MEM_SIZE];
    int num_instrucoes;
} Memory;

// Memória de dados
struct memoria_dados {
    int dados[DATA_SIZE];
};

// Program Counter
struct pc {
    int valor;
    int prev_valor;
};

// Estado anterior
struct estadoanterior{
    int reg[REG_COUNT];
    int dados[DATA_SIZE];
    int pc;
    enum classe_inst tipo;
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
};

// Estado completo do processador
struct estado_processador {
    Memory mem_instrucoes;
    struct memoria_dados mem_dados;
    int registradores[REG_COUNT];
    struct pc pc;
    int halt_flag;
    int passos_executados;
    char nomearqsaida[256];
    struct estadoanterior ultimoestado;
};

// Protótipos de funções
void decodificar(const char *inst_str, struct instrucao *inst, struct estado_processador *cpu);
void print_instrucao(const struct instrucao *inst);
void load_memory(Memory *memory, const char *filename, struct estado_processador *cpu);
void imprimir_memoria_instrucoes(Memory *mem);
int ula(enum ops_ula operacao, int a, int b, int *flag);
void executa_instrucao(struct estado_processador *estado);
void executa_proxima_instrucao(struct estado_processador *estado);
void mostrar_registradores(int registradores[]);
void mostrar_memoria_dados(struct memoria_dados *mem);
void salvar_instrucoes_executadas(struct estado_processador *estado, Memory *mem, const char *filename);
void salvar_memoria_dados(struct memoria_dados *mem, const char *filename);
void mostrar_estado_processador(struct estado_processador *estado);
void display_menu_principal();
void display_menu_execucao();
void inicializar_processador(struct estado_processador *cpu);
void criaasm(struct instrucao *inst, const char *nomearqsaida);
void back(struct estado_processador *cpu);
void salvar_estado_para_arquivo(struct estado_processador *estado, const char *filename);
int proxima_instrucao_eh_halt(struct estado_processador *estado);

FILE *arqasm;


// Função principal
int main() {
    struct estado_processador cpu;
    inicializar_processador(&cpu);
    
    int option;
    char filename[256];
    int em_execucao = 0;
    
    

    int sair = 0;

    do {
        if (em_execucao && !cpu.halt_flag) {
            display_menu_execucao();
        } else {
            display_menu_principal();
        }
        
        scanf("%d", &option);
        getchar();
        
        if (em_execucao && !cpu.halt_flag) {
            switch(option) {
                case 1:
                    executa_proxima_instrucao(&cpu);
                    break;
                case 2:
                    mostrar_registradores(cpu.registradores);
                    break;
                case 3:
                    imprimir_memoria_instrucoes(&cpu.mem_instrucoes);
                    break;
                case 4:
                    mostrar_memoria_dados(&cpu.mem_dados);
                    break;
                case 5:
                    printf("Nome do arquivo para salvar instruções: ");
                    fgets(filename, sizeof(filename), stdin);
                    filename[strcspn(filename, "\n")] = '\0';
                    salvar_instrucoes_executadas(&cpu, &cpu.mem_instrucoes, filename);
                    break;
                case 6:
                    printf("Nome do arquivo para salvar dados: ");
                    fgets(filename, sizeof(filename), stdin);
                    filename[strcspn(filename, "\n")] = '\0';
                    salvar_memoria_dados(&cpu.mem_dados, filename);
                    break;
                case 7:
                    back(&cpu);
                    break;
                case 8:
                    em_execucao = 0;
                    break;
                case 9:
                printf("Total de instruções executadas: %d\n", cpu.passos_executados);  
                break;
                case 10:
                    while (!cpu.halt_flag && cpu.pc.valor < cpu.mem_instrucoes.num_instrucoes) {
                        executa_proxima_instrucao(&cpu);
                    }
                    printf("\n--- Execução concluída ---\n");
                    mostrar_estado_processador(&cpu); // Mostra o estado final
                    break;
                        default:
                            printf("Opção inválida!\n");
                    }
                } else {
                    switch(option) {
                    case 1:
                        printf("Digite o nome do arquivo de instruções: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = '\0';
                        load_memory(&cpu.mem_instrucoes, filename, &cpu);
                        break;
                    case 2:
                        if (cpu.mem_instrucoes.num_instrucoes > 0) {
                            em_execucao = 1;
                            cpu.halt_flag = 0;
                            printf("Modo execução passo a passo ativado\n");
                        } else {
                            printf("Erro: Nenhuma instrução carregada\n");
                        }
                        break;
                    case 3:
                        mostrar_estado_processador(&cpu);
                        break;
                    case 4:
                        printf("Nome do arquivo para salvar estado: ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = '\0';
                        salvar_estado_para_arquivo(&cpu, filename);
                        break;
                    case 5:
                        sair = 1;
                        return 0;
                    default:
                        printf("Opção inválida!\n");
                }
            }
        } while (!sair);

    return 0;
}

// Implementação das funções

void inicializar_processador(struct estado_processador *cpu) {
    memset(cpu, 0, sizeof(struct estado_processador));
    cpu->pc.valor = 0;
    cpu->pc.prev_valor = -1;
    cpu->halt_flag = 1; // Começa parado
    cpu->passos_executados = 0;
    
    // Inicializa memória de dados
    for (int i = 0; i < DATA_SIZE; i++) {
        cpu->mem_dados.dados[i] = 0;
    }
    
    // Inicializa registradores
    for (int i = 0; i < REG_COUNT; i++) {
        cpu->registradores[i] = 0;
    }
}

void display_menu_principal() {
    printf("\n=== MENU PRINCIPAL ===\n");
    printf("1. Carregar programa\n");
    printf("2. Iniciar execução passo a passo\n");
    printf("3. Mostrar estado do processador\n");
    printf("4. Salvar estado do processador\n");
    printf("5. Sair\n");
    printf("======================\n");
    printf("Escolha uma opção: ");
}

void display_menu_execucao() {
    printf("\n=== MENU DE EXECUÇÃO ===\n");
    printf("1. Executar próxima instrução\n");
    printf("2. Mostrar registradores\n");
    printf("3. Mostrar memória de instruções\n");
    printf("4. Mostrar memória de dados\n");
    printf("5. Salvar instruções executadas em arquivo\n");
    printf("6. Salvar memória de dados em arquivo\n");
    printf("7. Voltar instrução anterior\n");
    printf("8. Voltar ao menu principal\n");
    printf("9. Mostrar total de instruções executadas\n");
    printf("10. Executar todas as instruções até o fim\n");
    printf("=========================\n");
    printf("Escolha uma opção: ");
}

void decodificar(const char *inst_str, struct instrucao *inst, struct estado_processador *cpu) {
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
    else if (inst->opcode == 2){
        inst->tipo = tipo_J;
        char addr_str[13];
        strncpy(addr_str, inst_str + 4, 12); addr_str[12] = '\0';
        inst->addr = strtol(addr_str, NULL, 2);
        inst->addr = (cpu->pc.valor & 0xF000) | (inst->addr & 0x0FFF);
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
    

    cpu->ultimoestado.tipo = inst->tipo;
    cpu->ultimoestado.opcode = inst->opcode;
    cpu->ultimoestado.rs = inst->rs;
    cpu->ultimoestado.rt = inst->rt;
    cpu->ultimoestado.rd = inst->rd;
    cpu->ultimoestado.funct = inst->funct;
    cpu->ultimoestado.imm = inst->imm;
    cpu->ultimoestado.addr = inst->addr;
}

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

void load_memory(Memory *memory, const char *filename, struct estado_processador *cpu) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return;
    }

    char line[INSTR_BITS+2];
    int i = 0;
    
    memset(memory->instr_mem, 0, sizeof(memory->instr_mem));
    memset(memory->decodificadas, 0, sizeof(memory->decodificadas));

    strncpy(cpu->nomearqsaida, filename, sizeof(cpu->nomearqsaida));
    char *ext = strrchr(cpu->nomearqsaida, '.');
    if (ext) *ext = '\0';
    strcat(cpu->nomearqsaida, ".asm");

    FILE *arqasm = fopen(cpu->nomearqsaida, "w");
    if (arqasm) fclose(arqasm);
    
    while (fgets(line, sizeof(line), file) && i < MEM_SIZE) {
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) continue; 

        
        
        if (strlen(line) == INSTR_BITS) {
            strncpy(memory->instr_mem[i], line, INSTR_BITS);
            memory->instr_mem[i][INSTR_BITS] = '\0';
            decodificar(line, &memory->decodificadas[i], cpu);
            i++;
        }
    }
    fclose(file);
    
    memory->num_instrucoes = i;
    printf("Arquivo '%s' carregado com sucesso!\n", filename);
    printf("%d instrucoes carregadas e decodificadas.\n", i);
}

void imprimir_memoria_instrucoes(Memory *mem) {
    printf("\n=== Memória de Instruções ===\n");
    printf("Endereço | Conteúdo Binário   | Decodificação\n");
    printf("---------|-------------------|--------------\n");
    
    for (int i = 0; i < mem->num_instrucoes; i++) {
        printf("[%03d]    | %-16s | ", i, mem->instr_mem[i]);
        
        printf("Tipo: ");
        switch (mem->decodificadas[i].tipo) {
            case tipo_R:
                printf("R | op=%d rs=%d rt=%d rd=%d funct=%d", 
                      mem->decodificadas[i].opcode, 
                      mem->decodificadas[i].rs,
                      mem->decodificadas[i].rt,
                      mem->decodificadas[i].rd,
                      mem->decodificadas[i].funct);
                break;
            case tipo_I:
                printf("I | op=%d rs=%d rt=%d imm=%d", 
                      mem->decodificadas[i].opcode,
                      mem->decodificadas[i].rs,
                      mem->decodificadas[i].rt,
                      mem->decodificadas[i].imm);
                break;
            case tipo_J:
                printf("J | op=%d addr=%d", 
                      mem->decodificadas[i].opcode,
                      mem->decodificadas[i].addr);
                break;
            default:
                printf("Inválido");
        }
        printf("\n");
    }
}

int ula(enum ops_ula operacao, int a, int b, int *flag) {
    *flag = NO_FLAG;
    int resultado;
    
    switch (operacao) {
        case ULA_ADD:
            resultado = a + b;
            // Detecção de overflow para soma
            if ((a > 0 && b > 0 && resultado < 0) || (a < 0 && b < 0 && resultado > 0)) {
                *flag = OVERFLOW_FLAG;
            }
            break;
            
        case ULA_SUB:
            resultado = a - b;
            // Detecção de overflow para subtração
            if ((a > 0 && b < 0 && resultado < 0) || (a < 0 && b > 0 && resultado > 0)) {
                *flag = OVERFLOW_FLAG;
            }
            break;
            
        case ULA_AND:
            resultado = a & b;
            break;
            
        case ULA_OR:
            resultado = a | b;
            break;
            
        default:
            resultado = 0;
    }
    
    
    if (operacao == ULA_SUB && *flag != OVERFLOW_FLAG) {
        if (resultado == 0) {
            *flag = BEQ_FLAG;
        }
    }
    
    return resultado;
}


void executa_instrucao(struct estado_processador *estado) {
    int pc_atual = estado->pc.valor;
    int flag = NO_FLAG;  // Declaração única da variável flag

    estado->ultimoestado.pc = estado->pc.valor;
    memcpy(estado->ultimoestado.reg, estado->registradores, sizeof(estado->registradores));
    memcpy(estado->ultimoestado.dados, estado->mem_dados.dados, sizeof(estado->mem_dados.dados));
    
    // Verifica se PC está dentro dos limites
    if (pc_atual < 0 || pc_atual >= estado->mem_instrucoes.num_instrucoes) {
        estado->halt_flag = 1;
        return;
    }

    struct instrucao *inst = &estado->mem_instrucoes.decodificadas[pc_atual];

    if (inst->tipo == tipo_R && inst->rd == 8) {  // $t0 = $8 
        estado->halt_flag = 1;
        printf("Fim do programa: instrução com rd=$t0 (registrador %d)\n", inst->rd);
        return;
    }

    estado->pc.prev_valor = pc_atual;
    estado->pc.valor++;

    switch (inst->tipo) {
    case tipo_R: {
        if (inst->rs == 0 && inst->rt == 0 && inst->rd == 0 && inst->funct == 0) {
            printf("Instrução NOP (000...0) - Ativando HALT\n");
            estado->halt_flag = 1;
            return;
        }
        
        int result = 0;  // Resultado das operações ULA
        
        switch (inst->funct) {
            case 0: // ADD
                result = ula(ULA_ADD, estado->registradores[inst->rs], estado->registradores[inst->rt], &flag);
                if (flag == OVERFLOW_FLAG) {
                    printf("Overflow na soma! Resultado: %d\n", result);
                    estado->registradores[inst->rd] = 0;  // Tratamento do overflow
                } else {
                    estado->registradores[inst->rd] = result;
                }
                break;
                
            case 2: // SUB
                result = ula(ULA_SUB, estado->registradores[inst->rs], estado->registradores[inst->rt], &flag);
                if (flag == OVERFLOW_FLAG) {
                    printf("Overflow na subtração! Resultado: %d\n", result);
                    estado->registradores[inst->rd] = 0;
                } else {
                    estado->registradores[inst->rd] = result;
                }
                break;
                
            case 4: // AND
                result = ula(ULA_AND, estado->registradores[inst->rs], estado->registradores[inst->rt], &flag);
                estado->registradores[inst->rd] = result;  // Operações lógicas não geram overflow
                break;
                
            case 5: // OR
                result = ula(ULA_OR, estado->registradores[inst->rs], estado->registradores[inst->rt], &flag);
                estado->registradores[inst->rd] = result;  // Operações lógicas não geram overflow
                break;
        }
        break;
    }
        case tipo_I: {
            switch (inst->opcode) {
                case 4: // ADDI
                    estado->registradores[inst->rt] = ula(ULA_ADD, estado->registradores[inst->rs], inst->imm, &flag);
                    if (flag == OVERFLOW_FLAG) {
                        printf("Overflow no ADDI! Registrador $%d não atualizado.\n", inst->rt);
                        estado->registradores[inst->rt] = 0;
                    }
                    break;
                case 8: // BEQ
                    ula(ULA_SUB, estado->registradores[inst->rs], estado->registradores[inst->rt], &flag);
                    if (flag == BEQ_FLAG) {
                        estado->pc.valor += inst->imm;
                    }
                    break;
                case 11: // LW
                    estado->registradores[inst->rt] = estado->mem_dados.dados[ula(ULA_ADD, estado->registradores[inst->rs], inst->imm, &flag)];
                    break;
                case 15: // SW
                    estado->mem_dados.dados[ula(ULA_ADD, estado->registradores[inst->rs], inst->imm, &flag)] = estado->registradores[inst->rt];
                    break;
            }
            break;
        }
        
        case tipo_J: {
            switch (inst->opcode) {
                case 2: // JMP
                    estado->pc.valor = inst->addr;

                    if (estado->pc.valor >= estado->mem_instrucoes.num_instrucoes) {
                        printf("Aviso: Jump para endereço fora do espaço de instruções (%d)\n", estado->pc.valor);
                        estado->halt_flag = 1;
                    }
                    break;
            }
            break;
        }
        
        default:
            // HALT será tratado como um opcode especial (255)
            if (inst->opcode == 15) {  // HALT
                estado->halt_flag = 1;
                printf("Instrução HALT encontrada no PC=%03d - programa concluído\n", pc_atual);
                estado->pc.valor++; // Avança para próxima instrução (mesmo com halt)
                return;
            }
    }
    criaasm(inst, estado->nomearqsaida);
}

void mostrar_registradores(int registradores[]) {
    printf("\n=== Registradores ===\n");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("$%d = %d\n", i, registradores[i]);
    }
}

void mostrar_memoria_dados(struct memoria_dados *mem) {
    printf("\n=== Memória de Dados (256 endereços) ===\n");
    
    // Exibe 8 valores por linha, com blocos de 8 endereços
    for (int i = 0; i < DATA_SIZE; i++) {  // DATA_SIZE = 256 (definido no início do código)
        if (i % 8 == 0) {
            printf("\n[%03d-%03d] ", i, i+7);  // Cabeçalho do bloco (ex: [000-007])
        }
        printf("%5d ", mem->dados[i]);
    }
    printf("\n");
}

void salvar_instrucoes_executadas(struct estado_processador *estado, Memory *mem, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erro ao criar arquivo");
        return;
    }
    
    for (int i = 0; i < estado->pc.valor && i < estado->mem_instrucoes.num_instrucoes; i++) {
        fprintf(file, "%s\n", estado->mem_instrucoes.instr_mem[i]);
    }
    
    fclose(file);
    printf("Instruções salvas em %s\n", filename);
}

void salvar_memoria_dados(struct memoria_dados *mem, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erro ao criar arquivo");
        return;
    }
    
    for (int i = 0; i < DATA_SIZE; i++) {
        fprintf(file, "%d\n", mem->dados[i]);
    }
    
    fclose(file);
    printf("Memória de dados salva em %s\n", filename);
}

void mostrar_estado_processador(struct estado_processador *estado) {
    printf("\n=== Estado do Processador ===\n");
    printf("PC: %d (anterior: %d)\n", estado->pc.valor, estado->pc.prev_valor);
    printf("Halt flag: %d\n", estado->halt_flag);
    printf("Passos executados: %d\n", estado->passos_executados);
    
    mostrar_registradores(estado->registradores);
    
    imprimir_memoria_instrucoes(&estado->mem_instrucoes);
    
    mostrar_memoria_dados(&estado->mem_dados);
    
    printf("\nPróxima instrução a executar:\n");
    if (estado->pc.valor < estado->mem_instrucoes.num_instrucoes) {
        print_instrucao(&estado->mem_instrucoes.decodificadas[estado->pc.valor]);
    } else {
        printf("Nenhuma (fim do programa alcançado)\n");
    }
}
void executa_proxima_instrucao(struct estado_processador *estado) {
    if (estado->halt_flag) {
        printf("Processador está parado (HALT)\n");
        return;
    }
    
    if (estado->pc.valor >= estado->mem_instrucoes.num_instrucoes) {
        printf("Fim do programa alcançado\n");
        estado->halt_flag = 1;
        return;
    }
    
    printf("\nExecutando instrução [PC=%03d]: %s\n", 
           estado->pc.valor, estado->mem_instrucoes.instr_mem[estado->pc.valor]);
    
    // Mostrar estado antes da execução
    printf("Registradores antes: ");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("$%d=%d ", i, estado->registradores[i]);
    }
    printf("\n");

    struct instrucao *inst = &estado->mem_instrucoes.decodificadas[estado->pc.valor];
    // Verificar se a próxima instrução após o destino do JMP é HALT
    if (inst->tipo == tipo_J && inst->opcode == 2) {
        int destino = inst->addr;
        if (destino + 1 < estado->mem_instrucoes.num_instrucoes &&
            strcmp(estado->mem_instrucoes.instr_mem[destino + 1], "0000000000000000") == 0) {
            
            printf("\nAVISO: Após o JMP para PC=%03d, a próxima instrução será HALT. Deseja continuar? (s/n): ", destino);
            char resposta;
            scanf(" %c", &resposta);
            getchar();

            if (resposta == 'n' || resposta == 'N') {
                estado->halt_flag = 1;
                printf("Execução interrompida pelo usuário.\n");
                return;
            }
        }
    }  
    executa_instrucao(estado);
    estado->passos_executados++;
    
    // Mostrar estado após a execução
    printf("Registradores após:  ");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("$%d=%d ", i, estado->registradores[i]);
    }
    printf("\n");
    
    printf("Instrução executada com sucesso. Passos totais: %d\n", estado->passos_executados);
    
    // Verificar se a próxima instrução é inválida ou fim do programa
    if (estado->pc.valor >= estado->mem_instrucoes.num_instrucoes) {
        printf("Fim do programa alcançado\n");
        estado->halt_flag = 1;
    }
}
void criaasm(struct instrucao *inst, const char *nomearq){

    FILE *arqasm = fopen(nomearq, "a");
    if(!arqasm){
        printf("Erro ao abrir arquivo de saída %s\n", nomearq);
        perror("Erro");
        return;
    }else{
        if(inst->tipo == tipo_R){
            if(inst->funct == 0){
                fprintf(arqasm, "add $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt);
            }
            if(inst->funct == 2){
                fprintf(arqasm, "sub $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt);
            }
            if(inst->funct == 4){
                fprintf(arqasm, "and $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt);
            }
            if(inst->funct == 5){
                fprintf(arqasm, "or $%d, $%d, $%d\n", inst->rd, inst->rs, inst->rt);
            }
        }
        
        if(inst->tipo == tipo_I){
            if(inst->opcode == 4){
                fprintf(arqasm, "addi $%d, $%d, %d\n", inst->rt, inst->rs, inst->imm);
            }
            if(inst->opcode == 11){
                fprintf(arqasm, "lw $%d, %d($%d)\n", inst->rt, inst->imm, inst->rs);
            }
            if(inst->opcode == 15){
                fprintf(arqasm, "sw $%d, %d($%d)\n", inst->rt, inst->imm, inst->rs);
            }
            if(inst->opcode == 8){
                fprintf(arqasm, "beq $%d, $%d, %d\n", inst->rt, inst->rs, inst->imm);
            }
        }
        
        if(inst->tipo == tipo_J){
            if(inst->opcode == 2){
            fprintf(arqasm, "j %d\n", inst->addr);
            }
        }
    }
    fclose(arqasm);
}
void back(struct estado_processador *cpu){
    printf("\n--- Reexecução da última instrução ---\n");

    memcpy(cpu->registradores, cpu->ultimoestado.reg, sizeof(cpu->registradores));
    memcpy(cpu->mem_dados.dados, cpu->ultimoestado.dados, sizeof(cpu->mem_dados.dados));
    cpu->pc.valor = cpu->ultimoestado.pc;
    
    struct instrucao inst = {
        .tipo = cpu->ultimoestado.tipo,
        .opcode = cpu->ultimoestado.opcode,
        .rs = cpu->ultimoestado.rs,
        .rt = cpu->ultimoestado.rt,
        .rd = cpu->ultimoestado.rd,
        .funct = cpu->ultimoestado.funct,
        .imm = cpu->ultimoestado.imm,
        .addr = cpu->ultimoestado.addr
    };

    printf("Estado restaurado para PC = %d\n", cpu->pc.valor);
}
void salvar_estado_para_arquivo(struct estado_processador *estado, const char *filename) {
    FILE *file = fopen(filename, "a");  
    if (!file) {
        perror("Erro ao abrir arquivo");
        return;
    }

    // Redireciona a saída para o arquivo
    fprintf(file, "\n=== Estado do Processador ===\n");
    fprintf(file, "PC: %d (anterior: %d)\n", estado->pc.valor, estado->pc.prev_valor);
    fprintf(file, "Halt flag: %d\n", estado->halt_flag);
    fprintf(file, "Passos executados: %d\n", estado->passos_executados);
    
    // Salva registradores
    fprintf(file, "\nRegistradores:\n");
    for (int i = 0; i < REG_COUNT; i++) {
        fprintf(file, "$%d = %d\n", i, estado->registradores[i]);
    }
    
    // Salva memória de instruções
    fprintf(file, "\nMemória de Instruções:\n");
    fprintf(file, "Endereço | Conteúdo Binário   | Decodificação\n");
    fprintf(file, "---------|-------------------|--------------\n");
    for (int i = 0; i < estado->mem_instrucoes.num_instrucoes; i++) {
        fprintf(file, "[%03d]    | %-16s | ", i, estado->mem_instrucoes.instr_mem[i]);
        switch (estado->mem_instrucoes.decodificadas[i].tipo) {
            case tipo_R:
                fprintf(file, "R | op=%d rs=%d rt=%d rd=%d funct=%d\n", 
                      estado->mem_instrucoes.decodificadas[i].opcode, 
                      estado->mem_instrucoes.decodificadas[i].rs,
                      estado->mem_instrucoes.decodificadas[i].rt,
                      estado->mem_instrucoes.decodificadas[i].rd,
                      estado->mem_instrucoes.decodificadas[i].funct);
                break;
            case tipo_I:
                fprintf(file, "I | op=%d rs=%d rt=%d imm=%d\n", 
                      estado->mem_instrucoes.decodificadas[i].opcode,
                      estado->mem_instrucoes.decodificadas[i].rs,
                      estado->mem_instrucoes.decodificadas[i].rt,
                      estado->mem_instrucoes.decodificadas[i].imm);
                break;
            case tipo_J:
                fprintf(file, "J | op=%d addr=%d\n", 
                      estado->mem_instrucoes.decodificadas[i].opcode,
                      estado->mem_instrucoes.decodificadas[i].addr);
                break;
            default:
                fprintf(file, "Inválido\n");
        }
    }
    
    // Salva memória de dados
    fprintf(file, "\nMemória de Dados (256 endereços):\n");
    for (int i = 0; i < DATA_SIZE; i++) {
        if (i % 8 == 0) fprintf(file, "\n[%03d-%03d] ", i, i+7);
        fprintf(file, "%5d ", estado->mem_dados.dados[i]);
    }
    
    // Próxima instrução
    fprintf(file, "\n\nPróxima instrução a executar:\n");
    if (estado->pc.valor < estado->mem_instrucoes.num_instrucoes) {
        fprintf(file, "Binário: %s\n", estado->mem_instrucoes.decodificadas[estado->pc.valor].binario);
        fprintf(file, "Opcode: %d | Tipo: ", estado->mem_instrucoes.decodificadas[estado->pc.valor].opcode);
        switch (estado->mem_instrucoes.decodificadas[estado->pc.valor].tipo) {
            case tipo_R:
                fprintf(file, "R | rs: %d, rt: %d, rd: %d, funct: %d\n", 
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].rs,
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].rt,
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].rd,
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].funct);
                break;
            case tipo_I:
                fprintf(file, "I | rs: %d, rt: %d, imm: %d\n", 
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].rs,
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].rt,
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].imm);
                break;
            case tipo_J:
                fprintf(file, "J | addr: %d\n", 
                      estado->mem_instrucoes.decodificadas[estado->pc.valor].addr);
                break;
            default:
                fprintf(file, "Inválido\n");
        }
    } else {
        fprintf(file, "Nenhuma (fim do programa alcançado)\n");
    }
    
    fclose(file);
    printf("Estado salvo em '%s' (formato completo)\n", filename);
}
int proxima_instrucao_eh_halt(struct estado_processador *estado) {
    if (estado->pc.valor + 1 >= estado->mem_instrucoes.num_instrucoes) {
        return 0; // Não há próxima instrução
    }
    return strcmp(estado->mem_instrucoes.instr_mem[estado->pc.valor + 1], "0000000000000000") == 0;
}