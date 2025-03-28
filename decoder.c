#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEM_SIZE 256       // Tamanho total da memória
#define INSTR_BITS 16      // Tamanho de cada instrução em bits

enum classe_inst{
    tipo_I, tipo_J, tipo_R, tipo_OUTROS
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

typedef struct {
    struct instrucao instr_mem[MEM_SIZE]; // Matriz para armazenar as instruções
} Memory;

void decodificador(struct instrucao *inst){
    
    char opcode_str[5];
    strncpy(opcode_str, (*inst).inst_char, 4);
    opcode_str[4]= '\0';
    (*inst).opcode = strtol(opcode_str, NULL, 2);
    
    if ((*inst).opcode == 0) { // Tipo R
        (*inst).tipo_inst = tipo_R;
        char rs_str[4], rt_str[4], rd_str[4], funct_str[4];
        strncpy(rs_str, (*inst).inst_char + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, (*inst).inst_char + 7, 3); rt_str[3] = '\0';
        strncpy(rd_str, (*inst).inst_char + 10, 3); rd_str[3] = '\0';
        strncpy(funct_str, (*inst).inst_char + 13, 3); funct_str[3] = '\0';
        
        (*inst).rs = strtol(rs_str, NULL, 2);
        (*inst).rt = strtol(rt_str, NULL, 2);
        (*inst).rd = strtol(rd_str, NULL, 2);
        (*inst).funct = strtol(funct_str, NULL, 2);
        
    } 
    else if ((*inst).opcode == 2 || (*inst).opcode == 3) { // Tipo J
        (*inst).tipo_inst = tipo_J;
        char address_str[13];
        strncpy(address_str, (*inst).inst_char + 4, 12); address_str[12] = '\0';
        (*inst).addr = strtol(address_str, NULL, 2);
        
    } 
    else { // Tipo I
        (*inst).tipo_inst = tipo_I;
        char rs_str[4], rt_str[4], imm_str[7];
        strncpy(rs_str, (*inst).inst_char + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, (*inst).inst_char + 7, 3); rt_str[3] = '\0';
        strncpy(imm_str, (*inst).inst_char + 10, 6); imm_str[6] = '\0';
        
        (*inst).rs = strtol(rs_str, NULL, 2);
        (*inst).rt = strtol(rt_str, NULL, 2);
        (*inst).imm = strtol(imm_str, NULL, 2);
        
        printf("Tipo I -> opcode: %d, rs: %d, rt: %d, immediate: %d\n", 
               (*inst).opcode, (*inst).rs, (*inst).rt, (*inst).imm);
    }
}

int main() {
    Memory mem = {
        {
            {tipo_R, "0000000100010000"},  // Exemplo de instrução tipo R
            {tipo_I, "0100000100100101"},  // Exemplo de instrução tipo I
            {tipo_J, "0010000000110100"}   // Exemplo de instrução tipo J
        }
    };

    for (int i = 0; i < 3; i++) {
        decodificador(&mem.instr_mem[i]);
    }
    
    return 0;
}
