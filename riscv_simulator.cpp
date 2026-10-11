#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>

class RISCVSimulator {
private:
    // Registers
    uint32_t regs[32];
    uint32_t pc;
    
    // Memory
    std::vector<uint8_t> memory;
    
    // Pipeline registers for 5-stage pipeline
    struct IF_ID {
        uint32_t instruction;
        uint32_t pc_plus_4;
    } if_id;
    
    struct ID_EX {
        uint32_t rs1_val;
        uint32_t rs2_val;
        uint32_t imm;
        uint32_t rd;
        uint32_t pc_plus_4;
        bool reg_write;
        bool mem_read;
        bool mem_write;
        bool mem_to_reg;
        bool alu_src;
        uint32_t alu_op;
        uint32_t funct3;
        uint32_t funct7;
    } id_ex;
    
    struct EX_MEM {
        uint32_t alu_result;
        uint32_t rs2_val;
        uint32_t rd;
        bool reg_write;
        bool mem_read;
        bool mem_write;
        bool mem_to_reg;
    } ex_mem;
    
    struct MEM_WB {
        uint32_t mem_data;
        uint32_t alu_result;
        uint32_t rd;
        bool reg_write;
        bool mem_to_reg;
    } mem_wb;
    
public:
    RISCVSimulator() : pc(0) {
        // Initialize registers to 0
        for (int i = 0; i < 32; i++) {
            regs[i] = 0;
        }
        // Initialize pipeline registers
        memset(&if_id, 0, sizeof(if_id));
        memset(&id_ex, 0, sizeof(id_ex));
        memset(&ex_mem, 0, sizeof(ex_mem));
        memset(&mem_wb, 0, sizeof(mem_wb));
    }
    
    // Load program from input
    void loadProgram() {
        std::string line;
        memory.clear();
        
        // Read binary data from stdin
        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            uint32_t value;
            if (iss >> std::hex >> value) {
                // Store in memory (little-endian)
                memory.push_back(value & 0xFF);
                memory.push_back((value >> 8) & 0xFF);
                memory.push_back((value >> 16) & 0xFF);
                memory.push_back((value >> 24) & 0xFF);
            }
        }
    }
    
    // Fetch stage
    void fetch() {
        if (pc < memory.size()) {
            // Read instruction from memory (little-endian)
            uint32_t instruction = memory[pc] |
                                 (memory[pc+1] << 8) |
                                 (memory[pc+2] << 16) |
                                 (memory[pc+3] << 24);
            
            if_id.instruction = instruction;
            if_id.pc_plus_4 = pc + 4;
            pc += 4;
        }
    }
    
    // Decode stage
    void decode() {
        uint32_t instr = if_id.instruction;
        uint32_t opcode = instr & 0x7F;
        uint32_t rd = (instr >> 7) & 0x1F;
        uint32_t funct3 = (instr >> 12) & 0x7;
        uint32_t rs1 = (instr >> 15) & 0x1F;
        uint32_t rs2 = (instr >> 20) & 0x1F;
        uint32_t funct7 = (instr >> 25) & 0x7F;
        
        // Get register values
        id_ex.rs1_val = regs[rs1];
        id_ex.rs2_val = regs[rs2];
        id_ex.rd = rd;
        id_ex.pc_plus_4 = if_id.pc_plus_4;
        
        // Control signals (simplified)
        id_ex.reg_write = true;
        id_ex.mem_read = false;
        id_ex.mem_write = false;
        id_ex.mem_to_reg = false;
        id_ex.alu_src = false;
        id_ex.funct3 = funct3;
        id_ex.funct7 = funct7;
        
        // Immediate generation
        if (opcode == 0x03 || opcode == 0x13 || opcode == 0x67) {
            // I-type
            int32_t imm = (int32_t)instr >> 20;
            id_ex.imm = imm;
        } else if (opcode == 0x23) {
            // S-type
            int32_t imm = ((int32_t)instr >> 25) | ((instr >> 7) & 0x1F);
            id_ex.imm = imm;
        } else if (opcode == 0x63) {
            // B-type
            int32_t imm = ((int32_t)instr >> 31) | ((instr >> 25) & 0x3F) | 
                         ((instr >> 8) & 0xF) | ((instr >> 7) & 0x1) << 11;
            id_ex.imm = imm;
        } else if (opcode == 0x37) {
            // U-type
            int32_t imm = (int32_t)instr >> 12;
            id_ex.imm = imm;
        } else if (opcode == 0x6F) {
            // J-type
            int32_t imm = ((int32_t)instr >> 31) | ((instr >> 21) & 0x3FF) |
                         ((instr >> 20) & 0x1) << 11 | ((instr >> 12) & 0xFF) << 12;
            id_ex.imm = imm;
        }
        
        // ALU operation
        id_ex.alu_op = opcode;
    }
    
    // Execute stage
    void execute() {
        uint32_t alu_input2 = id_ex.alu_src ? id_ex.imm : id_ex.rs2_val;
        
        // Simple ALU implementation
        switch (id_ex.alu_op) {
            case 0x33: // R-type
                if (id_ex.funct3 == 0x0) {
                    if (id_ex.funct7 == 0x00) {
                        ex_mem.alu_result = id_ex.rs1_val + alu_input2; // ADD
                    } else if (id_ex.funct7 == 0x20) {
                        ex_mem.alu_result = id_ex.rs1_val - alu_input2; // SUB
                    }
                }
                break;
            case 0x13: // I-type arithmetic
                if (id_ex.funct3 == 0x0) {
                    ex_mem.alu_result = id_ex.rs1_val + id_ex.imm; // ADDI
                }
                break;
            default:
                ex_mem.alu_result = 0;
                break;
        }
        
        ex_mem.rs2_val = id_ex.rs2_val;
        ex_mem.rd = id_ex.rd;
        ex_mem.reg_write = id_ex.reg_write;
        ex_mem.mem_read = id_ex.mem_read;
        ex_mem.mem_write = id_ex.mem_write;
        ex_mem.mem_to_reg = id_ex.mem_to_reg;
    }
    
    // Memory stage
    void memoryStage() {
        if (ex_mem.mem_read) {
            // Load from memory
            if (ex_mem.alu_result < memory.size()) {
                mem_wb.mem_data = memory[ex_mem.alu_result];
            }
        }
        
        if (ex_mem.mem_write) {
            // Store to memory
            if (ex_mem.alu_result < memory.size()) {
                memory[ex_mem.alu_result] = ex_mem.rs2_val & 0xFF;
            }
        }
        
        mem_wb.alu_result = ex_mem.alu_result;
        mem_wb.rd = ex_mem.rd;
        mem_wb.reg_write = ex_mem.reg_write;
        mem_wb.mem_to_reg = ex_mem.mem_to_reg;
    }
    
    // Writeback stage
    void writeback() {
        if (mem_wb.reg_write && mem_wb.rd != 0) {
            uint32_t write_data = mem_wb.mem_to_reg ? mem_wb.mem_data : mem_wb.alu_result;
            regs[mem_wb.rd] = write_data;
        }
    }
    
    // Run simulation
    void run() {
        loadProgram();
        
        // Simple simulation loop
        int cycles = 0;
        const int max_cycles = 1000; // Prevent infinite loops
        
        while (pc < memory.size() && cycles < max_cycles) {
            fetch();
            decode();
            execute();
            memoryStage();
            writeback();
            cycles++;
        }
        
        // Output result - this needs to be determined based on problem requirements
        // For now, output register 0 (which should always be 0 in RISC-V)
        std::cout << regs[0] << std::endl;
    }
};

int main() {
    RISCVSimulator simulator;
    simulator.run();
    return 0;
}