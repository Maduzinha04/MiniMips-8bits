#include <stdio.h>
#include <string.h>

#define DATA_SIZE 256
#define INSTR_SIZE 16
#define HALF_WORD_SIZE 2
#define MEM_SIZE 256

// Estrutura da memória de dados
typedef struct datamem{
    int data_mem[DATA_SIZE];  // Cada posição = 1 byte (8 bits)
} DataMemory;

// Estrutura do Program Counter
typedef struct pc{
    int pc;          // Endereço atual
    int prev_pc;     // Endereço anterior (para branches)
} ProgramCounter;

// Estrutura da instrução decodificada 
typedef struct inst{
    int opcode;      // 4 bits
    int rs, rt, rd;  // Registradores
    int imm;         // Imediato (12 bits)
    int addr;        // Endereço (para jumps)
    char type;       // 'R', 'I', 'J'
    char binary[INSTR_SIZE+1]; // Instrução em binário
} Instruction;

// Função de decodificação 
void decodificar(const char *inst_str, Instruction *inst) {
    strncpy(inst->binary, inst_str, INSTR_SIZE);
    inst->binary[INSTR_SIZE] = '\0';

    // Extrai opcode (primeiros 4 bits)
    char opcode_str[5];
    strncpy(opcode_str, inst_str, 4);
    opcode_str[4] = '\0';
    inst->opcode = strtol(opcode_str, NULL, 2);

    if (inst->opcode == 0) { // Tipo R
        inst->type = 'R';
        char rs_str[4], rt_str[4], rd_str[4], funct_str[4];
        strncpy(rs_str, inst_str + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst_str + 7, 3); rt_str[3] = '\0';
        strncpy(rd_str, inst_str + 10, 3); rd_str[3] = '\0';
        strncpy(funct_str, inst_str + 13, 3); funct_str[3] = '\0';
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->rd = strtol(rd_str, NULL, 2);
    } 
    else if (inst->opcode == 2 || inst->opcode == 3) { // Tipo J
        inst->type = 'J';
        char addr_str[13];
        strncpy(addr_str, inst_str + 4, 12); addr_str[12] = '\0';
        inst->addr = strtol(addr_str, NULL, 2);
    } 
    else { // Tipo I
        inst->type = 'I';
        char rs_str[4], rt_str[4], imm_str[7];
        strncpy(rs_str, inst_str + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst_str + 7, 3); rt_str[3] = '\0';
        strncpy(imm_str, inst_str + 10, 6); imm_str[6] = '\0';
        inst->rs = strtol(rs_str, NULL, 2);
        inst->rt = strtol(rt_str, NULL, 2);
        inst->imm = strtol(imm_str, NULL, 2);
    }
}

// Funções de memória 
void store_halfword(DataMemory *dmem, int addr, int value) {
    dmem->data_mem[addr] = value >> 8;    // Byte alto
    dmem->data_mem[addr + 1] = value;     // Byte baixo
}

int load_halfword(DataMemory *dmem, int addr) {
    return (dmem->data_mem[addr] << 8) | dmem->data_mem[addr + 1];
}

void init_memory(DataMemory *dmem) {
    memset(dmem->data_mem, 0, sizeof(dmem->data_mem));
}

// Simulação do ciclo fetch-decode-execute
void run_simulation(const char *program[], int program_size) {
    DataMemory dmem;
    ProgramCounter pc = {0, -1};
    Instruction inst;
    init_memory(&dmem);

    while (pc.pc < program_size) {
        // 1. FETCH: Busca instrução
        const char *current_instr = program[pc.pc];
        
        // 2. DECODE: Decodifica
        decodificar(current_instr, &inst);
        
        // 3. EXECUTE (exemplo simplificado)
        printf("PC=%02d | Instrução: %s | Tipo: %c\n", pc.pc, inst.binary, inst.type);
        
        // Operações de memória (exemplo para load/store)
        if (inst.type == 'I' && inst.opcode == 4) { // Load
            inst.rt = load_halfword(&dmem, inst.imm);
            printf("  LOAD: mem[%d] -> R%d (Valor: 0x%04X)\n", inst.imm, inst.rt, inst.rt);
        } 
        else if (inst.type == 'I' && inst.opcode == 5) { // Store
            store_halfword(&dmem, inst.imm, inst.rt);
            printf("  STORE: R%d -> mem[%d]\n", inst.rt, inst.imm);
        }
        
        // Atualiza PC (sequencial ou branch)
        pc.prev_pc = pc.pc;
        pc.pc++;
    }
}

// Programa de teste (instruções em binário)
const char *test_program[] = {
    "0100001100101010",  // LOAD  R2 <- mem[42] (op=4, rt=2, imm=42)
    "0101001100101010",  // STORE R2 -> mem[42] (op=5, rt=2, imm=42)
    "0000000000000000",  // NOP (op=0)
    "0000000000000000"   // NOP (op=0)
};

int main() {
    // Inicializa memória de dados
    DataMemory dmem;
    init_memory(&dmem);
    
    // Pré-carrega um valor na memória
    store_halfword(&dmem, 42, 0xABCD);
    printf("Valor inicial em mem[42]: 0x%04X\n", load_halfword(&dmem, 42));
    
    // Executa o programa
    run_simulation(test_program, 4);
    
    return 0;
}