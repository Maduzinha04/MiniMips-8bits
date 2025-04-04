typedef struct {
    uint16_t instrucao;  // Última instrução executada (16 bits)
    uint16_t pc_anterior; // Endereço do PC antes da execução da última instrução
} BackState;

void executarInstrucao(uint16_t instrucao, uint16_t *pc, BackState *back) {
    // Salvar o estado anterior antes da execução
    back->instrucao = instrucao;
    back->pc_anterior = *pc;

    // Decodificar e executar a instrução (simulação)
    // Aqui você colocaria a lógica de execução real do MIPS-16
    *pc += 2; // Exemplo: PC avança para a próxima instrução
}

void back(uint16_t *pc, BackState *back) {
    if (back->instrucao == 0) {
        printf("Nenhuma instrução anterior para executar novamente.\n");
        return;
    }

    // Restaurar o PC para a posição antes da última execução
    *pc = back->pc_anterior;

    // Reexecutar a última instrução
    printf("Reexecutando a instrução: 0x%04X\n", back->instrucao);
    executarInstrucao(back->instrucao, pc, back);
}


#include <stdio.h>
#include <stdint.h>

// Definições das funções e estrutura BackState aqui...

int main() {
    uint16_t pc = 0x0000; // PC inicial
    BackState back = {0, 0}; // Estado inicial do back

    // Simulação de execução de instruções
    printf("Executando instrução 1...\n");
    executarInstrucao(0x1234, &pc, &back);
    
    printf("Executando instrução 2...\n");
    executarInstrucao(0x5678, &pc, &back);

    // Agora chamamos a função back para reexecutar a última instrução
    printf("Chamando back...\n");
    back(&pc, &back);

    return 0;
}
