#include "Shader.h"
#include "Render/vk_common.h"
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <array>
#include <cstdint>
#include <string>
#include <iostream>
#include <limits>

namespace slang_context {

    Slang::ComPtr<slang::IGlobalSession> g_slang_global_session;
    Slang::ComPtr<slang::ISession> g_slang_session;

    bool Init() {
        if (!InitSlangSession()) return false;
        return true;
    }

    bool InitSlangSession() {
        if (SLANG_FAILED(slang::createGlobalSession(g_slang_global_session.writeRef()))) {
            std::cerr << "[ERROR::SHADER] failed to create slang global session\n";
            return false;
        }

        slang::TargetDesc target_desc{
            .format = SLANG_SPIRV,
            .profile = g_slang_global_session->findProfile("spirv_1_4"),
        };

        std::array<const char*, 1> search_paths = { "Shaders/" };

        slang::CompilerOptionEntry spirv_option{
            .name = slang::CompilerOptionName::EmitSpirvDirectly,
            .value = slang::CompilerOptionValueKind::Int,
        };

        std::array<slang::CompilerOptionEntry, 1> compiler_option_entries = { spirv_option };

        slang::SessionDesc session_desc{
            .targets = &target_desc,
            .targetCount = 1,
            .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
            .searchPaths = search_paths.data(),
            .searchPathCount = static_cast<uint32_t>(search_paths.size()),
            .compilerOptionEntries = compiler_option_entries.data(),
            .compilerOptionEntryCount = static_cast<uint32_t>(compiler_option_entries.size()),
        };

        if (SLANG_FAILED(g_slang_global_session->createSession(session_desc, g_slang_session.writeRef()))) {
            std::cerr << "[ERROR::SHADER] failed to create slang session\n";
            return false;
        }

        return true;
    }
}

static void showDiagnostics(Slang::ComPtr<slang::IBlob>& diagnostics);

Shader::Shader(const std::string& filename, const std::string& entry_point_name, const ShaderStage stage) : m_entry_point(entry_point_name) {
    set_shader_stage(stage);
    LoadModule(filename, entry_point_name);
}

void Shader::LoadModule(const std::string& filename, const std::string& entry_point_name) {
    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IModule> slang_module(slang_context::g_slang_session->loadModule(filename.c_str(), diagnostics.writeRef()));
    showDiagnostics(diagnostics);
    if (slang_module == nullptr) {
        std::cerr << "[ERROR::VK_SHADER] failed to load slang module\n";
        return;
    }

    SlangResult result = std::numeric_limits<int32_t>::max();
    Slang::ComPtr<slang::IEntryPoint> entry_point;
    result = slang_module->findEntryPointByName(entry_point_name.c_str(), entry_point.writeRef());
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::SHADER] entry point not found: " << entry_point_name << "\n";
        return;
    }

    std::array<slang::IComponentType*, 2> components = { slang_module, entry_point };
    Slang::ComPtr<slang::IComponentType> program;
    result = slang_context::g_slang_session->createCompositeComponentType(components.data(), components.size(), program.writeRef(), diagnostics.writeRef());
    showDiagnostics(diagnostics);
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::VK_SHADER] failed to create composite component type\n";
        return;
    }

    Slang::ComPtr<slang::IComponentType> linked_program;
    result = program->link(linked_program.writeRef(), diagnostics.writeRef());
    showDiagnostics(diagnostics);
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::VK_SHADER] failed to link program\n";
        return;
    }

    int entry_point_index = 0;
    int target_index = 0;
    result = linked_program->getEntryPointCode(entry_point_index, target_index, m_spirv.writeRef(), diagnostics.writeRef());
    showDiagnostics(diagnostics);
    if (SLANG_FAILED(result)) {
        std::cerr << "[ERROR::VK_SHADER] failed to get entry point code\n";
        return;
    }

    m_module_create_info.codeSize = m_spirv->getBufferSize();
    m_module_create_info.pCode = static_cast<const uint32_t*>(m_spirv->getBufferPointer());
    std::cout << "Shader module successfully read for file " << filename << '\n';
}

static void showDiagnostics(Slang::ComPtr<slang::IBlob>& diagnostics) {
    if (diagnostics != nullptr) {
        std::cerr << "[Slang Diagnostics] " << static_cast<const char*>(diagnostics->getBufferPointer()) << '\n';
    }
    diagnostics.setNull();
}

void Shader::set_shader_stage(const ShaderStage stage) {
    switch (stage) {
    case ShaderStage::vertex:
        Shader::m_stage = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case ShaderStage::fragment:
        Shader::m_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    default:
        m_stage = VK_SHADER_STAGE_ALL;
    }
}
