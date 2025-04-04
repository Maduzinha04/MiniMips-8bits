#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define REG_COUNT 32

// Enumeração para tipos de instrução 
enum classe_inst {
    tipo_I, tipo_J, tipo_R, tipo_OUTROS
};

// Estrutura da instrução 
struct instrucao {
    enum classe_inst tipo_inst;
    char inst_char[INSTR_SIZE+1];
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
};

// Estrutura da memória de dados
struct memoria_dados {
    int dados[DATA_SIZE];  // Memória como array de inteiros
};

// Estrutura do Program Counter
struct pc {
    int pc;
    int prev_pc;
};

// Estrutura do simulador completo
struct simulador {
    struct memoria_dados dmem;
    struct pc pc;
    int reg[REG_COUNT];      // Banco de registradores
    struct instrucao *programa;
    int prog_size;
};

// Função decodificador 
void decodificador(struct instrucao *inst) {
    strncpy(inst->inst_char, inst->inst_char, INSTR_SIZE);
    inst->inst_char[INSTR_SIZE] = '\0';

    char opcode_str[5];
    strncpy(opcode_str, inst->inst_char, 4);
    opcode_str[4] = '\0';
    inst->opcode = strtol(opcode_str, NULL, 2);
    
    if (inst->opcode == 0) { // Tipo R
        inst->tipo_inst = tipo_R;
        char rs_str[4], rt_str[4], rd_str[4], funct_str[4];
        strncpy(rs_str, inst->inst_char + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst->inst_char + 7, 3); rt_str[3] = '\0';
        strncpy(rd_str, inst->inst_char + 10, 3); rd_str[3] = '\0';
        strncpy(funct_str, inst->inst_char + 13, 3); funct_str[3] = '\0';
        
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->rd = strtol(rd_str, NULL, 2);
        inst->funct = strtol(funct_str, NULL, 2);
        
    } 
    else if (inst->opcode == 2 || inst->opcode == 3) { // Tipo J
        inst->tipo_inst = tipo_J;
        char address_str[13];
        strncpy(address_str, inst->inst_char + 4, 12); address_str[12] = '\0';
        inst->addr = strtol(address_str, NULL, 2);
        
    } 
    else { // Tipo I
        inst->tipo_inst = tipo_I;
        char rs_str[4], rt_str[4], imm_str[7];
        strncpy(rs_str, inst->inst_char + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst->inst_char + 7, 3); rt_str[3] = '\0';
        strncpy(imm_str, inst->inst_char + 10, 6); imm_str[6] = '\0';
        
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->imm = strtol(imm_str, NULL, 2);
    }
}

// Função para executar instruções
void executar_instrucao(struct simulador *sim, struct instrucao *inst) {
    switch(inst->tipo_inst) {
        case tipo_R:
            if (inst->funct == 0) { // ADD
                sim->reg[inst->rd] = sim->reg[inst->rs] + sim->reg[inst->rt];
                printf("ADD R%d, R%d, R%d\n", inst->rd, inst->rs, inst->rt);
            }
            break;
            
        case tipo_I:
            if (inst->opcode == 4) { // LW
                sim->reg[inst->rt] = sim->dmem.dados[sim->reg[inst->rs] + inst->imm];
                printf("LW R%d, %d(R%d)\n", inst->rt, inst->imm, inst->rs);
            } 
            else if (inst->opcode == 5) { // SW
                sim->dmem.dados[sim->reg[inst->rs] + inst->imm] = sim->reg[inst->rt];
                printf("SW R%d, %d(R%d)\n", inst->rt, inst->imm, inst->rs);
            }
            break;
            
        case tipo_J:
            printf("J %d\n", inst->addr);
            sim->pc.pc = inst->addr;
            break;
            
        default:
            printf("Instrução não implementada (Opcode: %d)\n", inst->opcode);
    }
}

// Ciclo principal de execução
void run_simulation(struct simulador *sim) {
    while (sim->pc.pc < sim->prog_size) {
        struct instrucao *current = &sim->programa[sim->pc.pc];
        
        // Atualiza PC anterior
        sim->pc.prev_pc = sim->pc.pc;
        
        // Decodifica
        decodificador(current);
        
        // Executa
        executar_instrucao(sim, current);
        
        // Incrementa PC (a menos que seja jump)
        if (current->tipo_inst != tipo_J) {
            sim->pc.pc++;
        }
    }
}

int main() {
    // Programa de teste (agora usando struct instrucao)
    struct instrucao programa_teste[] = {
        {tipo_I, "0100000100101010"},  // LW R2, 42(R1)
        {tipo_I, "0101000101001010"},  // SW R4, 42(R1)
        {tipo_R, "0000001001100000"},  // ADD R3, R1, R2
        {tipo_J, "0010000000000100"}   // J 4
    };

    // Configuração do simulador
    struct simulador sim = {
        .dmem = {0},
        .pc = {0, -1},
        .reg = {0},
        .programa = programa_teste,
        .prog_size = 4
    };

    // Pré-carrega alguns valores
    sim.reg[1] = 0;       // R1 = 0 (base)
    sim.reg[2] = 0xABCD;   // Valor teste
    sim.dmem.dados[42] = 1234;

    // Executa
    run_simulation(&sim);

    return 0;
}
