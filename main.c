/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#define MEM_SIZE 256
#define REG_COUNT 8

typedef struct memoria{
    int instr_mem[MEM_SIZE];
    int data_mem[MEM_SIZE];
} Memory;

typedef struct {
    int regs[REG_COUNT];
} Registers;

typedef struct xxxx; {
    Memory memory;
    Registers registers;
    ULA ula;
    int pc;
} MiniMIPS;

    void load_memory(*mem, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) exit(1);
    for (int i = 0; i < mem && fscanf(file, "%s", &memory->instr_mem[i]) != EOF; i++);
    fclose(file);
    }
int main(){
    char *filename[30];
    int op;
    Memory *mem;
    
    *filename=&arquivo;
    
    
    enum tipo_inst{
        classe_r,
        classe_i,
        classe_j
    }
    enum classe_inst{
        tipo_R,//OPCODE=0000
        tipo_I,//OPCODE=0100,1011,1111
        tipo_J,//OPCODE=0010
        tipo_OUTROS//OPCODE=
}; 
    struct instrucao{
        enum classe_inst tipo_inst;
        char inst_char[17];
        int opcode;
        int rs;
        int rt;
        int rd;
        int funct;
        int imm;
        int addr;
};
    
    printf("Digite o nome do arquivo para ser carregado na memoria!\n");
    scanf("%s", &arquivo);
    
    arquivo = fopen("arquivo.txt", "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }
    
    

    printf("**************Menu principal************\n");
    printf("1. Carregar memória de instruções (.mem)\n");
    printf("3. Imprimir memórias (instruções e dados)\n");
    printf("4. Imprimir banco de registradores\n");
    printf("5. Imprimir todo o simulador (registradores e memórias)\n");
    printf("6. Salvar .asm\n");
    printf("7. Salvar .dat\n");
    printf("8. Executa Programa (run)\n");
    printf("9. Executa uma instrução (Step)\n");
    printf("10. Volta uma instrução (Back)\n");
    printf("11.Ver as estatisticas do programa\n");
    printf("12. Encerrar o programa\n");
    scanf("%i", &op);
    
    while(op!=12){
    switch(op){
        case 1:
           load_memory();
            break;
            
        case 2:
            imprimimememoriadeinstruçoes();
            break;
            
        case 3:
            imprimirmemoria();// (Instruções e dados)
            break;
            
        case 4: 
            imprimiregistradores()
            break;
            
        case 5:
            imprimitodosimulador(); //Pôr a função imprimiregistradores() e imprimirmemoria() dentro da outra
            break;
            
        case 6:
            salvarasm();
            break;
            
        case 7:
            salvardat();
            break;
            
        case 8:
            executaprograma();
            break;
            
        case 9:
            executainstruçao();
            break;
            
        case 10:
            voltainstruçao();
            break;
            
        case 11:
            mostrarestatisticas();
            break;
            
        case 12:
            break;
        default:
            printf("Número invalido!\n");
            printf("Tente novamente!\n");
         }
}
    return 0;
}










