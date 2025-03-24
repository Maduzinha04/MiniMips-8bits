# README - Simulador MiniMIPS

## 📌 Visão Geral

Este projeto implementa um simulador didático da arquitetura MIPS simplificada, capaz de executar instruções em linguagem de máquina a partir de um arquivo de texto contendo código binário.

## 🛠️ Funcionalidades

- **Conjunto de instruções suportado**:
  - Aritméticas: ADD, SUB, ADDI
  - Lógicas: AND, OR
  - Acesso à memória: LW (Load Word), SW (Store Word)
  - Controle de fluxo: BEQ (Branch if Equal), J (Jump)

- **Componentes simulados**:
  - 8 registradores de propósito geral (R0-R7)
  - Memória de instruções (16x256 bits)
  - Memória de dados (256 bytes)
  - ULA (Unidade Lógica Aritmética)

## 📋 Pré-requisitos

- Compilador GCC
- Sistema operacional Linux/Unix (ou WSL no Windows)

## 🚀 Como Executar

1. **Compilação**:
   ```bash
   gcc -o minimips minimips.c
   ```

2. **Execução**:
   ```bash
   ./minimips instrucoes.txt
   ```

3. **Arquivo de entrada**:
   - Crie um arquivo `instrucoes.txt` contendo uma instrução por linha
   - Cada linha deve ter exatamente 16 caracteres (0s e 1s)

## 📝 Formato das Instruções

Exemplo de arquivo `instrucoes.txt`:
```
0100000001000101  # ADDI R2, 5
0100000001000011  # ADDI R1, 3
0000010001011000  # ADD R3, R2, R1
```
```

## 📚 Referência de Instruções

| Opcode | Funct | Mnemônico | Formato        | Operação               |
|--------|-------|-----------|----------------|------------------------|
| 0000   | 000   | ADD       | R-type         | Rd = Rs + Rt           |
| 0000   | 010   | SUB       | R-type         | Rd = Rs - Rt           |
| 0000   | 100   | AND       | R-type         | Rd = Rs AND Rt         |
| 0000   | 101   | OR        | R-type         | Rd = Rs OR Rt          |
| 0100   | -     | ADDI      | I-type         | Rt = Rs + imm          |
| 1011   | -     | LW        | I-type         | Rt = MEM[Rs + imm]     |
| 1111   | -     | SW        | I-type         | MEM[Rs + imm] = Rt     |
| 1000   | -     | BEQ       | I-type         | if (Rs == Rt) PC+=imm  |
| 0010   | -     | J         | J-type         | PC = addr              |

## 🐛 Reportando Problemas

Encontrou um bug? Por favor, abra uma issue no repositório do projeto.

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo LICENSE para detalhes.
