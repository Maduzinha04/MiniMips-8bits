#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256
#define REG_COUNT 8
#define INSTR_BITS 16

typedef struct {
    uint8_t data_mem[MEM_SIZE];
    char instr_mem[MEM_SIZE][INSTR_BITS+1]; // +1 para o '\0'
} Memory;

typedef struct {
    uint8_t regs[REG_COUNT];
} Registers;

typedef struct {
    uint8_t (*execute)(uint8_t, uint8_t, uint8_t);
} ULA;

typedef struct {
    Memory memory;
    Registers registers;
    ULA ula;
    uint16_t pc; // Alterado para 16 bits
} MiniMIPS;

uint8_t ula_execute(uint8_t a, uint8_t b, uint8_t opcode) {
    switch(opcode) {
        case 0: return a + b;  // ADD
        case 2: return a - b;  // SUB
        case 4: return a & b;  // AND
        case 5: return a | b;  // OR
        default: return 0;
    }
}

void load_memory(Memory *memory, const char *filename) {
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

uint16_t binary_to_uint16(const char *binary) {
    uint16_t result = 0;
    for (int i = 0; i < INSTR_BITS; i++) {
        if (binary[i] == '1') {
            result |= (1 << (INSTR_BITS - 1 - i));
        }
    }
    return result;
}

void execute_instruction(MiniMIPS *cpu) {
    if (cpu->pc >= MEM_SIZE) return;

    uint16_t instr = binary_to_uint16(cpu->memory.instr_mem[cpu->pc]);
    uint8_t opcode = (instr >> 12) & 0xF;
    uint8_t rs = (instr >> 9) & 0x7;
    uint8_t rt = (instr >> 6) & 0x7;
    uint8_t rd = (instr >> 3) & 0x7;
    uint8_t funct = instr & 0x7;
    uint8_t imm = instr & 0x3F;
    uint16_t addr = instr & 0xFFF;

    switch(opcode) {
        case 0: // Operações tipo R
            switch(funct) {
                case 0: // ADD
                    cpu->registers.regs[rd] = cpu->ula.execute(cpu->registers.regs[rs], cpu->registers.regs[rt], 0);
                    break;
                case 2: // SUB
                    cpu->registers.regs[rd] = cpu->ula.execute(cpu->registers.regs[rs], cpu->registers.regs[rt], 2);
                    break;
                case 4: // AND
                    cpu->registers.regs[rd] = cpu->ula.execute(cpu->registers.regs[rs], cpu->registers.regs[rt], 4);
                    break;
                case 5: // OR
                    cpu->registers.regs[rd] = cpu->ula.execute(cpu->registers.regs[rs], cpu->registers.regs[rt], 5);
                    break;
            }
            cpu->pc++;
            break;
            
        case 4: // ADDI
            cpu->registers.regs[rt] = cpu->registers.regs[rs] + imm;
            cpu->pc++;
            break;
            
        case 11: // LW
            cpu->registers.regs[rt] = cpu->memory.data_mem[imm];
            cpu->pc++;
            break;
            
        case 15: // SW
            cpu->memory.data_mem[imm] = cpu->registers.regs[rt];
            cpu->pc++;
            break;
            
        case 8: // BEQ
            if (cpu->registers.regs[rs] == cpu->registers.regs[rt]) {
                cpu->pc += imm + 1;
            } else {
                cpu->pc++;
            }
            break;
            
        case 2: // J
            cpu->pc = addr;
            break;
            
        default:
            printf("Instrução não reconhecida: %04X\n", instr);
            cpu->pc++;
            break;
    }
}

void run(MiniMIPS *cpu) {
    while (cpu->pc < MEM_SIZE) {
        printf("PC: %03X | Instrução: %s | ", cpu->pc, cpu->memory.instr_mem[cpu->pc]);
        for (int i = 0; i < REG_COUNT; i++) {
            printf("R%d:%02X ", i, cpu->registers.regs[i]);
        }
        printf("\n");
        
        execute_instruction(cpu);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_instrucoes.txt>\n", argv[0]);
        return 1;
    }

    MiniMIPS cpu = {0};
    cpu.ula.execute = ula_execute;
    load_memory(&cpu.memory, argv[1]);
    run(&cpu);

    return 0;
}