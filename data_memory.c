#include <stdio.h>
#include <stdint.h>  // Para tipos de dados precisos (uint8_t, uint16_t)
#include <string.h>  // Para funções de manipulação de strings (memset, strncpy)

// Tamanho da memória de dados em bytes
#define DATA_SIZE 256
// Tamanho de meia palavra (16 bits) em bytes
#define HALF_WORD_SIZE 2
#define MEM_SIZE 256

// Estrutura para representar a memória de dados
typedef struct {
    uint8_t data_mem[DATA_SIZE];  // Memória de dados (cada célula = 1 byte)
}DataMemory;

// Estrutura para representar o Program Counter (PC)
typedef struct pc{
    uint16_t pc;       // Contador de programa (endereço atual)
    uint16_t prev_pc;  // Endereço anterior 
} ProgramCounter;

// Inicializa toda a memória de dados com zeros
void init_data_memory(DataMemory *dmem) {
    memset(dmem->data_mem, 0, DATA_SIZE);
}

// Função para armazenar um valor na memória de dados
void store_halfword(DataMemory *dmem, uint8_t address, uint16_t value) {
    if (address % HALF_WORD_SIZE != 0) {  // Verifica alinhamento
        printf("Erro: Endereço %d não alinhado para 16 bits!\n", address);
        return;
    }
    // Divide o valor em 2 bytes e armazena
    dmem->data_mem[address] = (value >> 8) & 0xFF;   // Byte mais significativo
    dmem->data_mem[address + 1] = value & 0xFF;      // Byte menos significativo
}

// Lê uma meia-palavra (16 bits) de endereço par
uint16_t load_halfword(DataMemory *dmem, uint8_t address) {
    if (address % HALF_WORD_SIZE != 0) {  // Verifica alinhamento
        printf("Erro: Endereço %d não alinhado para 16 bits!\n", address);
        return 0;
    }
    // Combina 2 bytes para formar um valor de 16 bits
    return (dmem->data_mem[address] << 8) | dmem->data_mem[address + 1];
}

// Inicializa o PC com zero
void pc_init(ProgramCounter *pc) {
    pc->pc = 0;
    pc->prev_pc = 0;
}

// Atualiza o PC para um novo endereço
void pc_update(ProgramCounter *pc, uint16_t new_pc) {
    pc->prev_pc = pc->pc;  // Guarda o endereço atual
    pc->pc = new_pc;       // Atualiza para o novo endereço
}

// Executa um desvio (branch) somando um offset ao PC atual
void pc_branch(ProgramCounter *pc, uint16_t offset) {
    pc_update(pc, pc->pc + offset);
}

// Função principal para testar as funcionalidades
int main(){
    DataMemory dmem; // Declara a memória de dados
    ProgramCounter pc; // Declara o contador de programa
    pc_init(&pc); // Inicializa o PC

    init_data_memory(&dmem); // Inicializa a memória de dados


    while (1) {
        // 1. Busca da instrução
        const char *current_instr = imem.instr_mem[pc.pc];
        printf("Executando PC=%d: %s\n", pc.pc, current_instr);

        // 2. Determina próximo PC (incremento padrão)
        uint16_t next_pc = pc.pc + 1;
        
        // Exemplo simplificado de branch (substitua pela sua lógica real)
        if (current_instr[0] == '1' && current_instr[1] == '1') {
            uint16_t offset = 5;  // Valor do branch (exemplo)
            pc_branch(&pc, offset);
        } else {
            pc_update(&pc, next_pc);  // Avança para próxima instrução
        }

        // Exemplo de acesso à memória de dados
        if (pc.pc == 10) {
            store_halfword(&dmem, 0x00, 0xABCD);  // Armazena valor
            uint16_t val = load_halfword(&dmem, 0x00);  // Lê valor
            printf("Dado lido: 0x%04X\n", val);
        }

        // Condição de parada (fim da memória)
        if (pc.pc >= MEM_SIZE) break;
    }

    return 0;
}