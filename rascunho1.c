#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEM_SIZE 256// Tamanho da memória (256 bytes)
#define REG_COUNT 8// Número de registradores (8 registradores de 8 bits)

uint8_t memory[MEM_SIZE]; // Memória principal
uint8_t registers[REG_COUNT]; // Registradores
uint8_t pc = 0; // Program Counter

// Carrega o arquivo .mem na memória
void load_memory(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }
    
    int addr = 0;
    while (fscanf(file, "%hhx", &memory[addr]) != EOF && addr < MEM_SIZE) {
        addr++;
    }
    fclose(file);
}

// Executa uma instrução
void execute_instruction(uint8_t instruction) {
    uint8_t opcode = (instruction >> 4) & 0x0F;  // 4 bits para opcode
    uint8_t reg1 = (instruction >> 2) & 0x03;    // 2 bits para reg1
    uint8_t reg2_imm = instruction & 0x03;      // 2 bits para reg2 ou parte do imediato
    int8_t imm = (instruction & 0x3F);          // Imediato de 6 bits
    if (imm & 0x20) imm |= 0xC0; // Extensão de sinal

    switch (opcode) {
        case 0b0000:  // ADD R1, R2
            registers[reg1] += registers[reg2_imm];
            break;
        case 0b0001:  // SUB R1, R2
            registers[reg1] -= registers[reg2_imm];
            break;
        case 0b0010:  // LOAD R1, [endereco]
            registers[reg1] = memory[reg2_imm];
            break;
        case 0b0011:  // STORE R1, [endereco]
            memory[reg2_imm] = registers[reg1];
            break;
        case 0b0100:  // JMP endereco
            pc = reg2_imm;
            return;
        case 0b0101:  // JZ R1, endereco
            if (registers[reg1] == 0) pc = reg2_imm;
            return;
        case 0b0110:  // AND R1, R2
            registers[reg1] &= registers[reg2_imm];
            break;
        case 0b0111:  // OR R1, R2
            registers[reg1] |= registers[reg2_imm];
            break;
        case 0b1000:  // ADDI R1, IMM
            registers[reg1] += imm;
            break;
        case 0b1001:  // SUBI R1, IMM
            registers[reg1] -= imm;
            break;
    }
    pc++;  // Incrementa o Program Counter
}

// Executa as instruções no modo step-by-step
void run() {
    char command;
    while (pc < MEM_SIZE) {
        printf("\nPC: %02X | Regs: ", pc);
        for (int i = 0; i < REG_COUNT; i++)
            printf("R%d:%02X ", i, registers[i]);
        printf("\nDigite 's' para step, 'q' para sair: ");
        
        scanf(" %c", &command);
        if (command == 'q') break;

        execute_instruction(memory[pc]);
    }
}

// Salva a memória final em um arquivo .mem
void save_memory(const char *filename) {
    FILE *file = fopen(filename, "w");
    for (int i = 0; i < MEM_SIZE; i++)
        fprintf(file, "%02X\n", memory[i]);
    fclose(file);
}

// Salva o código traduzido em um arquivo .asm
void save_assembly(const char *filename) {
    FILE *file = fopen(filename, "w");
    for (int i = 0; i < MEM_SIZE; i++) {
        uint8_t instr = memory[i];
        fprintf(file, "Addr %02X: ", i);
        switch ((instr >> 4) & 0x0F) {
            case 0b0000: fprintf(file, "ADD R%d, R%d\n", (instr >> 2) & 0x03, instr & 0x03); break;
            case 0b0001: fprintf(file, "SUB R%d, R%d\n", (instr >> 2) & 0x03, instr & 0x03); break;
            case 0b0010: fprintf(file, "LOAD R%d, [%d]\n", (instr >> 2) & 0x03, instr & 0x03); break;
            case 0b0011: fprintf(file, "STORE R%d, [%d]\n", (instr >> 2) & 0x03, instr & 0x03); break;
            case 0b0100: fprintf(file, "JMP %d\n", instr & 0x03); break;
            case 0b0101: fprintf(file, "JZ R%d, %d\n", (instr >> 2) & 0x03, instr & 0x03); break;
            case 0b0110: fprintf(file, "AND R%d, R%d\n", (instr >> 2) & 0x03, instr & 0x03); break;
            case 0b0111: fprintf(file, "OR R%d, R%d\n", (instr >> 2) & 0x03, instr & 0x03); break;
            case 0b1000: fprintf(file, "ADDI R%d, %d\n", (instr >> 2) & 0x03, instr & 0x3F); break;
            case 0b1001: fprintf(file, "SUBI R%d, %d\n", (instr >> 2) & 0x03, instr & 0x3F); break;
        }
    }
    fclose(file);
}

// Função principal
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.mem>\n", argv[0]);
        return 1;
    }

    load_memory(argv[1]);
    run();
    save_memory("saida.mem");
    save_assembly("saida.asm");

    printf("\nExecução finalizada. Arquivos gerados: saida.mem e saida.asm\n");
    return 0;
}
