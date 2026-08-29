#include "capstonebackend.h"
#include "parser/lief_parser.h"

#include <capstone/capstone.h>

namespace {
    DisasmInstruction toDisasmInstruction(const cs_insn& insn) {
        DisasmInstruction out;
        out.address = QStringLiteral("0x%1").arg(insn.address, 0, 16);
        out.mnemonic = QString::fromLatin1(insn.mnemonic);
        out.operands = QString::fromLatin1(insn.op_str);
        out.fileOffset = -1;
        out.size = insn.size;

        QString bytes;
        bytes.reserve(insn.size * 3);
        for (uint16_t i = 0; i < insn.size; ++i) {
            if (i)
                bytes += QLatin1Char(' ');
            bytes += QStringLiteral("%1").arg(insn.bytes[i], 2, 16, QLatin1Char('0'));
        }
        out.bytes = bytes;

        return out;
    }

} // namespace

CapstoneBackend::Result CapstoneBackend::disassembleFile(const QString& filePath,
                                                         const Options& opt,
                                                         std::atomic<bool>* m_cancelled) {
    Result result;

    // парсинг через LIEF
    ParsedBinary parsed = LiefParser::parse(filePath);
    if (!parsed.ok()) {
        result.error = parsed.error;
        return result;
    }

    // инициализация
    csh handle;
    if (cs_open(parsed.arch, parsed.mode, &handle) != CS_ERR_OK) {
        result.error = QStringLiteral("cs_open failed for detected architecture");
        return result;
    }

    if (opt.asmSyntax == 1)
        cs_option(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);
    // по умолчанию intel

    // дизассемблирование каждой секции
    for (const ParsedBinary::Section& section: parsed.sections) {
        if (m_cancelled && m_cancelled->load())
            break;

        DisasmSection disasmSection;
        disasmSection.name = section.name;
        disasmSection.vaddr = section.vaddr;
        disasmSection.fileOffset = section.fileOffset;
        disasmSection.size = section.bytes.size();
        disasmSection.hasFileMapping = true;

        cs_insn* insn = nullptr;
        const size_t count = cs_disasm(handle,
                                       section.bytes.data(), section.bytes.size(),
                                       section.vaddr, 0, &insn);

        disasmSection.instructions.reserve(static_cast<int>(count));
        for (size_t i = 0; i < count; ++i) {
            if (m_cancelled && m_cancelled->load())
                break;

            DisasmInstruction instr = toDisasmInstruction(insn[i]);

            instr.fileOffset = static_cast<qint64>(section.fileOffset)
                               + static_cast<qint64>(insn[i].address - section.vaddr);
            disasmSection.instructions.push_back(std::move(instr));
        }

        if (count > 0)
            cs_free(insn, count);

        result.sections.push_back(std::move(disasmSection));
    }

    cs_close(&handle);

    return result;
}