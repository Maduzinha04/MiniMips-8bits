#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 256

// Estrutura da memória de dados (agora armazena apenas inteiros)
struct memoria_dados {
    int *dados;      // Array de inteiros (32 bits cada)
    int tamanho;     // Tamanho total da memória
};

// Função para inicializar a memória
void init_memoria(struct memoria_dados *mem, int tamanho) {
    mem->tamanho = tamanho;
    mem->dados = (int *)malloc(tamanho * sizeof(int));
    memset(mem->dados, 0, tamanho * sizeof(int));
}

// Função simplificada para armazenar um valor inteiro
void store_value(struct memoria_dados *mem, int addr, int value) {
    if (addr < 0 || addr >= mem->tamanho) {
        printf("Erro: Endereço de memória inválido\n");
        return;
    }
    mem->dados[addr] = value;
}

// Função simplificada para carregar um valor inteiro
int load_value(struct memoria_dados *mem, int addr) {
    if (addr < 0 || addr >= mem->tamanho) {
        printf("Erro: Endereço de memória inválido\n");
        return 0;
    }
    return mem->dados[addr];
}

// Função principal para teste
int main() {
    struct memoria_dados mem;
    init_memoria(&mem, DATA_SIZE);  // Inicializa com 256 posições

    // Armazena um valor
    store_value(&mem, 42, 0x12345678);
    printf("Armazenado 0x%08X no endereço 42\n", 0x12345678);

    // Lê o valor
    int valor = load_value(&mem, 42);
    printf("Valor lido do endereço 42: 0x%08X\n", valor);

    // Libera a memória alocada
    free(mem.dados);
    return 0;
}