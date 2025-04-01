#include <stdio.h>

// Definição do tamanho da memória de dados
#define DATA_MEM_ROWS 256  // Número de linhas da memória
#define DATA_MEM_COLS 4    // Número de colunas (palavras de 4 bytes)

// Estrutura para representar a memória de dados
typedef struct data_memory{
    int data_mem[DATA_MEM_ROWS][DATA_MEM_COLS]; // Matriz que representa a memória de dados
}DataMemory;

// Estrutura para representar o Program Counter (PC)
typedef struct pc{
    int pc;      // Endereço da instrução atual
    int prev_pc; // Endereço da instrução anterior
}ProgramCounter;

// Função para inicializar a memória de dados com zeros
void initialize_data_memory(DataMemory *dmem){
    for (int i = 0; i < DATA_MEM_ROWS; i++){
        for (int j = 0; j < DATA_MEM_COLS; j++){
            dmem->data_mem[i][j] = 0; // Define todos os valores como 0
        }
    }
}

// Função para armazenar um valor na memória de dados
void store_data(DataMemory *dmem, int row, int col, int value){
    // Verifica se os índices estão dentro dos limites da memória
    if (row >= 0 && row < DATA_MEM_ROWS && col >= 0 && col < DATA_MEM_COLS){
        dmem->data_mem[row][col] = value; // Armazena o valor na posição especificada
    } else {
        printf("Erro: Endereço de memória inválido (%d, %d)\n", row, col);
    }
}

// Função para carregar um valor da memória de dados
int load_data(DataMemory *dmem, int row, int col){
    // Verifica se os índices estão dentro dos limites da memória
    if (row >= 0 && row < DATA_MEM_ROWS && col >= 0 && col < DATA_MEM_COLS) {
        return dmem->data_mem[row][col]; // Retorna o valor armazenado
    } else {
        printf("Erro: Endereço de memória inválido (%d, %d)\n", row, col);
        return -1; // Retorna um valor de erro caso o acesso seja inválido
    }
}

// Função para atualizar o Program Counter (PC)
void update_pc(ProgramCounter *pc, int new_address){
    pc->prev_pc = pc->pc; // Guarda o endereço anterior
    pc->pc = new_address; // Atualiza o PC com o novo endereço
}

// Função principal para testar as funcionalidades
int main(){
    DataMemory dmem; // Declara a memória de dados
    ProgramCounter pc = {0, -1}; // Inicializa o PC (inicia em 0, prev_pc em -1 indicando inexistência)

    initialize_data_memory(&dmem); // Inicializa a memória de dados

    // Teste 1: Armazenando valores na memória
    printf("Armazenando valores...\n");
    store_data(&dmem, 10, 2, 42);
    store_data(&dmem, 15, 1, 99);
    
    // Teste 2: Lendo valores armazenados na memória
    printf("Valor na posição (10,2): %d\n", load_data(&dmem, 10, 2)); 
    printf("Valor na posição (15,1): %d\n", load_data(&dmem, 15, 1)); 
    
    // Teste 3: Tentativa de acesso inválido
    printf("Tentando acessar posição inválida:\n");
    load_data(&dmem, 300, 2); // Deve exibir erro

    // Teste 4: Atualizando o Program Counter (PC)
    printf("\nAtualizando o PC...\n");
    update_pc(&pc, 100);
    printf("PC atual: %d, PC anterior: %d\n", pc.pc, pc.prev_pc);

    update_pc(&pc, 200);
    printf("PC atual: %d, PC anterior: %d\n", pc.pc, pc.prev_pc);

    return 0;
}