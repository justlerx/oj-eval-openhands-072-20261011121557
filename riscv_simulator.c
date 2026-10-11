#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// RISC-V Registers
uint32_t regs[32];
uint32_t pc;

// Memory (simplified)
uint8_t memory[1024 * 1024]; // 1MB memory
size_t memory_size = 0;

// Initialize simulator
void init_simulator() {
    memset(regs, 0, sizeof(regs));
    pc = 0;
    memset(memory, 0, sizeof(memory));
    memory_size = 0;
}

// Load program from input
void load_program() {
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        // Parse hex values from input
        uint32_t value;
        if (sscanf(line, "%x", &value) == 1) {
            // Store in memory (little-endian)
            if (memory_size + 4 < sizeof(memory)) {
                memory[memory_size++] = value & 0xFF;
                memory[memory_size++] = (value >> 8) & 0xFF;
                memory[memory_size++] = (value >> 16) & 0xFF;
                memory[memory_size++] = (value >> 24) & 0xFF;
            }
        }
    }
}

// Fetch instruction
uint32_t fetch() {
    if (pc + 4 <= memory_size) {
        uint32_t instruction = memory[pc] |
                              (memory[pc+1] << 8) |
                              (memory[pc+2] << 16) |
                              (memory[pc+3] << 24);
        pc += 4;
        return instruction;
    }
    return 0;
}

// Decode and execute instruction (simplified)
void execute(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;
    
    // Simple instruction execution
    switch (opcode) {
        case 0x33: // R-type
            if (funct3 == 0x0) {
                if (funct7 == 0x00) {
                    // ADD
                    regs[rd] = regs[rs1] + regs[rs2];
                } else if (funct7 == 0x20) {
                    // SUB
                    regs[rd] = regs[rs1] - regs[rs2];
                }
            }
            break;
            
        case 0x13: // I-type
            if (funct3 == 0x0) {
                // ADDI
                int32_t imm = (int32_t)instruction >> 20;
                regs[rd] = regs[rs1] + imm;
            }
            break;
            
        case 0x03: // Load
            if (funct3 == 0x0) {
                // LB
                int32_t imm = (int32_t)instruction >> 20;
                uint32_t addr = regs[rs1] + imm;
                if (addr < memory_size) {
                    regs[rd] = (int8_t)memory[addr];
                }
            }
            break;
            
        case 0x23: // Store
            if (funct3 == 0x0) {
                // SB
                int32_t imm = ((int32_t)instruction >> 25) | ((instruction >> 7) & 0x1F);
                uint32_t addr = regs[rs1] + imm;
                if (addr < memory_size) {
                    memory[addr] = regs[rs2] & 0xFF;
                }
            }
            break;
    }
    
    // Ensure x0 is always 0
    regs[0] = 0;
}

// Run simulation
void run_simulation() {
    init_simulator();
    load_program();
    
    // Reset PC to start of program
    pc = 0;
    
    // Execute instructions
    int max_cycles = 10000; // Prevent infinite loops
    int cycles = 0;
    
    while (pc < memory_size && cycles < max_cycles) {
        uint32_t instruction = fetch();
        if (instruction == 0) break;
        execute(instruction);
        cycles++;
    }
    
    // Output result - this needs to be determined based on problem requirements
    // For now, output register 0 (which should always be 0 in RISC-V)
    printf("%u\n", regs[0]);
}

int main() {
    run_simulation();
    return 0;
}