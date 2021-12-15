#include "rpm_provider.hpp"

#include <psapi.h>
#include <tlhelp32.h>

#include <hex/helpers/fmt.hpp>
#include <regex>

namespace hex::plugin::rpm {
    RPMProvider::RPMProvider() : Provider() {}

    RPMProvider::~RPMProvider() { this->close(); }

    bool RPMProvider::isAvailable() const { return handle_ != INVALID_HANDLE_VALUE; }

    bool RPMProvider::isReadable() const { return this->isAvailable(); }

    bool RPMProvider::isWritable() const { return this->isAvailable() && open_write_; }

    bool RPMProvider::isResizable() const { return false; }

    bool RPMProvider::isSavable() const { return false; }

    void RPMProvider::readRaw(u64 offset, void *buffer, size_t size) {
        if (!this->isReadable()) return;
        if (buffer == nullptr || size == 0 || (offset + size) > this->getActualSize()) return;

        ReadProcessMemory(handle_, (void *)(std::uintptr_t)offset, buffer, size, nullptr);
    }

    void RPMProvider::writeRaw(u64 offset, const void *buffer, size_t size) {
        if (!this->isWritable()) return;
        if (buffer == nullptr || size == 0 || (offset + size) > this->getActualSize()) return;

        WriteProcessMemory(handle_, (void *)(std::uintptr_t)offset, buffer, size, nullptr);
    }

    size_t RPMProvider::getActualSize() const { return (u64)1 << 48; }

    std::string RPMProvider::getName() const { return this->open_info_.label; }

    std::vector<std::pair<std::string, std::string>> RPMProvider::getDataInformation() const {
        return {
            {"hex.rpm.provider.base"_lang, hex::format("{:016X}", this->base_)},
            {"hex.rpm.provider.pid"_lang, hex::format("{}", this->open_info_.pid)},
            {"hex.rpm.provider.name"_lang, hex::format("{}", this->open_info_.name)},
        };
    }

    bool RPMProvider::open() {
        this->close();
        if (this->select_info_.label.empty()) return false;

        DWORD const base_access = PROCESS_VM_READ | PROCESS_QUERY_INFORMATION;
        DWORD const write_access = this->select_write_ ? PROCESS_VM_WRITE : 0;
        HANDLE const handle = OpenProcess(base_access | write_access, false, this->select_info_.pid);
        if (handle == INVALID_HANDLE_VALUE) return false;

        HMODULE module = {};
        DWORD size = {};
        EnumProcessModules(handle, &module, sizeof(module), &size);

        this->open_info_ = this->select_info_;
        this->open_write_ = this->select_write_;
        this->base_ = (u64)(uintptr_t)module;
        this->handle_ = handle;

        this->setCurrentPage(this->base_ / PageSize);

        return true;
    }

    void RPMProvider::close() {
        if (auto const handle = std::exchange(this->handle_, INVALID_HANDLE_VALUE); handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
            this->base_ = 0;
            this->open_write_ = false;
            this->open_info_ = {};
        }
    }

    [[nodiscard]] bool RPMProvider::hasLoadInterface() const { return true; }

    void RPMProvider::drawLoadInterface() {
        ImGui::Checkbox("hex.rpm.provider.select.write"_lang, &this->select_write_);

        ImGui::SameLine();

        bool do_refresh = ImGui::InputText("hex.rpm.provider.select.filter"_lang,
                                           this->select_filter_.data(),
                                           this->select_filter_.capacity(),
                                           ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize,
                                           ImGui::UpdateStringSizeCallback,
                                           &this->select_filter_);

        ImGui::SameLine();

        if (ImGui::Button("hex.rpm.provider.select.reload"_lang)) do_refresh = true;

        if (do_refresh) this->refreshSelectList();

        ImGui::NewLine();

        if (ImGui::BeginListBox("##hex.rpm.provider.select.list", {500.0f, 0.0f})) {
            for (const auto &info : this->select_list_) {
                if (ImGui::Selectable(info.label.c_str(), info.label == this->select_info_.label)) {
                    this->select_info_ = info;
                }
            }
            ImGui::EndListBox();
        }
    }

    void RPMProvider::refreshSelectList() {
        this->select_list_.clear();

        HANDLE const snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return;

        PROCESSENTRY32 entry = {.dwSize = sizeof(PROCESSENTRY32)};
        for (bool i = Process32First(snapshot, &entry); i; i = Process32Next(snapshot, &entry)) {
            this->select_list_.emplace_back(ProcessInfo{
                hex::format("{: 10} | {}", (int)entry.th32ProcessID, entry.szExeFile),
                entry.szExeFile,
                entry.th32ProcessID,
            });
        }
        if (!this->select_filter_.empty()) {
            try {
                constexpr auto flags = std::regex::optimize | std::regex::icase;
                auto const filter = [re = std::regex(this->select_filter_, flags)](ProcessInfo const &info) -> bool {
                    return !std::regex_search(info.label, re);
                };
                std::erase_if(this->select_list_, filter);
            } catch (std::regex_error const &) {
            }
        }
        CloseHandle(snapshot);
    }

}  // namespace hex::plugin::rpm
