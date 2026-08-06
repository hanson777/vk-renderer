#include "vk_shader.h"
#include "vk_common.h"
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <array>
#include <cstdint>

namespace Shader {

    Slang::ComPtr<slang::IGlobalSession> g_slangGlobalSession = nullptr;

    bool InitSlangSession() {
        slang::createGlobalSession(g_slangGlobalSession.writeRef());

        slang::TargetDesc targetDesc{
            .format = SLANG_SPIRV,
            .profile = g_slangGlobalSession->findProfile("spirv_1_4"),
        };

        std::array<const char*, 1> searchPaths = { "src/shaders/" };

        slang::CompilerOptionEntry spirvOption{
            .name = slang::CompilerOptionName::EmitSpirvDirectly,
            .value = slang::CompilerOptionValueKind::Int,
        };

        std::array<slang::CompilerOptionEntry, 1> compilerOptionEntries = { spirvOption };

        slang::SessionDesc sessionDesc{
            .targets = &targetDesc,
            .targetCount = 1,
            .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
            .searchPaths = searchPaths.data(),
            .searchPathCount = static_cast<uint32_t>(searchPaths.size()),
            .compilerOptionEntries = compilerOptionEntries.data(),
            .compilerOptionEntryCount = static_cast<uint32_t>(compilerOptionEntries.size()),
        };

        Slang::ComPtr<slang::ISession> slangSession;
        g_slangGlobalSession->createSession(sessionDesc, slangSession.writeRef());
    }
}
