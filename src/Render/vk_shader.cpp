#include "vk_shader.h"
#include "vk_common.h"
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <array>
#include <cstdint>
#include <string>
#include <iostream>
#include <limits>

namespace Shader {

    Slang::ComPtr<slang::IGlobalSession> g_slangGlobalSession;
    Slang::ComPtr<slang::ISession> g_slangSession;

    bool InitSlangSession() {
        if (SLANG_FAILED(slang::createGlobalSession(g_slangGlobalSession.writeRef()))) {
            std::cerr << "[ERROR::SHADER] failed to create slang global session\n";
            return false;
        }

        slang::TargetDesc targetDesc{
            .format = SLANG_SPIRV,
            .profile = g_slangGlobalSession->findProfile("spirv_1_4"),
        };

        std::array<const char*, 1> searchPaths = { "Shaders/" };

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

        if (SLANG_FAILED(g_slangGlobalSession->createSession(sessionDesc, g_slangSession.writeRef()))) {
            std::cerr << "[ERROR::SHADER] failed to create slang session\n";
            return false;
        }

        return true;
    }
}

static void showDiagnostics(Slang::ComPtr<slang::IBlob>& diagnostics) {
    if (diagnostics != nullptr) {
        std::cerr << "[Slang Diagnostics] " << static_cast<const char*>(diagnostics->getBufferPointer()) << '\n';
    }
    diagnostics.setNull();
}

VkShader::VkShader(const std::string& filename, const std::string& entryPointName, const VkShaderStageFlagBits stage) : m_entryPoint(entryPointName), m_stage(stage) {
    LoadModule(filename, entryPointName);
}

void VkShader::LoadModule(const std::string& filename, const std::string& entryPointName) {
    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IModule> slangModule(Shader::g_slangSession->loadModule(filename.c_str(), diagnostics.writeRef()));
    showDiagnostics(diagnostics);
    if (slangModule == nullptr) {
        std::cerr << "[ERROR::VK_SHADER] failed to load slang module\n";
        return;
    }

    SlangResult result = std::numeric_limits<int32_t>::max();
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    result = slangModule->findEntryPointByName(entryPointName.c_str(), entryPoint.writeRef());
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::SHADER] entry point not found: " << entryPointName << "\n";
        return;
    }

    std::array<slang::IComponentType*, 2> components = { slangModule, entryPoint };
    Slang::ComPtr<slang::IComponentType> program;
    result = Shader::g_slangSession->createCompositeComponentType(components.data(), components.size(), program.writeRef(), diagnostics.writeRef());
    showDiagnostics(diagnostics);
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::VK_SHADER] failed to create composite component type\n";
        return;
    }

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    result = program->link(linkedProgram.writeRef(), diagnostics.writeRef());
    showDiagnostics(diagnostics);
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::VK_SHADER] failed to link program\n";
        return;
    }

    int entryPointIndex = 0;
    int targetIndex = 0;
    Slang::ComPtr<slang::IBlob> spirv;
    result = linkedProgram->getEntryPointCode(entryPointIndex, targetIndex, spirv.writeRef(), diagnostics.writeRef());
    showDiagnostics(diagnostics);
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::VK_SHADER] failed to get entry point code\n";
        return;
    }

    m_moduleCreateInfo.codeSize = spirv->getBufferSize();
    m_moduleCreateInfo.pCode = static_cast<const uint32_t*>(spirv->getBufferPointer());
}