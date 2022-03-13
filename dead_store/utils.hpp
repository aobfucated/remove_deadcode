#pragma once
#include<fstream>
#include<vector>

#include<Zydis/Utils.h>
#include<Zydis/Zydis.h>

//static const std::vector<ZydisMnemonic> blacklist = {
    //ZYDIS_MNEMONIC_CLC, ZYDIS_MNEMONIC_BT,  ZYDIS_MNEMONIC_TEST,
    //ZYDIS_MNEMONIC_CMP, ZYDIS_MNEMONIC_CMC, ZYDIS_MNEMONIC_STC };

//static const std::vector<ZydisMnemonic> whitelist = {
    //ZYDIS_MNEMONIC_PUSH, ZYDIS_MNEMONIC_POP, ZYDIS_MNEMONIC_CALL,
    //ZYDIS_MNEMONIC_DIV };

struct ZYDIS_INSN_INFO {
    ZydisDecodedInstruction instr;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];
    std::uintptr_t addr;
    ZYDIS_INSN_INFO(ZydisDecodedInstruction instr, ZydisDecodedOperand* operands, uintptr_t addr)
    {
        this->instr = instr;
        this->addr = addr;
        memcpy(this->operands, operands, sizeof(this->operands));
    }
};
using ZYDIS_ROUTINUE = std::vector<ZYDIS_INSN_INFO>;

namespace utils {

    inline thread_local std::shared_ptr<ZydisDecoder> g_decoder = nullptr;
    inline thread_local std::shared_ptr<ZydisFormatter> g_formatter = nullptr;
    
    inline void init() {
        if (!g_decoder && !g_formatter) {
            g_decoder = std::make_shared<ZydisDecoder>();
            g_formatter = std::make_shared<ZydisFormatter>();

            ZydisDecoderInit(g_decoder.get(), ZYDIS_MACHINE_MODE_LONG_64,
                ZYDIS_STACK_WIDTH_64);

            ZydisFormatterInit(g_formatter.get(),
                ZYDIS_FORMATTER_STYLE_INTEL);
        }
    }


    inline bool open_binary_file(const std::string& file,
        std::vector<uint8_t>& data) {
        std::ifstream fstr(file, std::ios::binary);
        if (!fstr.is_open()) return false;

        fstr.unsetf(std::ios::skipws);
        fstr.seekg(0, std::ios::end);

        const auto file_size = fstr.tellg();

        fstr.seekg(NULL, std::ios::beg);
        data.reserve(static_cast<uint32_t>(file_size));
        data.insert(data.begin(), std::istream_iterator<uint8_t>(fstr),
            std::istream_iterator<uint8_t>());
        return true;
    }

    void print(const ZydisDecodedInstruction& instr) {
        char buffer[256];
        ZydisFormatterFormatInstruction(g_formatter.get(), &instr, 0,0,buffer,
            sizeof(buffer), 0u);
        std::puts(buffer);
    }

    void print(ZYDIS_ROUTINUE& routine) {
        char buffer[256];
        for (auto [instr,operands,addr] : routine) {
            ZydisFormatterFormatInstruction(g_formatter.get(), &instr, operands, instr.operand_count_visible,buffer,
                sizeof(buffer), addr);
            std::printf("> %p %s\n", addr, buffer);
        }
    }

    bool is_jcc(const ZydisDecodedInstruction& instr) {
        return instr.mnemonic >= ZYDIS_MNEMONIC_JB &&
            instr.mnemonic <= ZYDIS_MNEMONIC_JZ;
    }

    bool is_jmp(const ZydisDecodedInstruction& instr)
    {
        return instr.mnemonic == ZYDIS_MNEMONIC_JMP;
    }

    /// <summary>
    /// 判断第一次写的寄存器和第二次的是否冲突(覆盖)
    /// </summary>
    /// <param name="reg_write_first">第一次写的寄存器</param>
    /// <param name="reg_write_second">第二次写的寄存器</param>
    /// <returns></returns>
    bool reg_written_compare(ZydisRegister reg_write_first, ZydisRegister reg_write_second)
    {
        return ((ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, reg_write_first) == ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, reg_write_second))&&
            (ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64,reg_write_first) <= ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, reg_write_second)) || ((ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, reg_write_first) == 64) && ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, reg_write_second) == 32));
    }

    namespace optimize {
        
        //https://docs.microsoft.com/zh-cn/windows-hardware/drivers/debugger/x64-architecture
        /// <summary>
        /// 基本思路是在两次写寄存器中间判断有没有读的操作
        ///
        /// <summary>
        inline void remove_dead_store(ZYDIS_ROUTINUE block)
        {











        }
    }

}