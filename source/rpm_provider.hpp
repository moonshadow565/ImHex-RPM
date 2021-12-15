#pragma once

#include <hex/providers/provider.hpp>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace hex::plugin::rpm {
    class RPMProvider final : public hex::prv::Provider {
    public:
        explicit RPMProvider();
        ~RPMProvider() override;

        [[nodiscard]] bool isAvailable() const override;
        [[nodiscard]] bool isReadable() const override;
        [[nodiscard]] bool isWritable() const override;
        [[nodiscard]] bool isResizable() const override;
        [[nodiscard]] bool isSavable() const override;

        void readRaw(u64 offset, void *buffer, size_t size) override;
        void writeRaw(u64 offset, const void *buffer, size_t size) override;
        [[nodiscard]] size_t getActualSize() const override;

        [[nodiscard]] std::string getName() const override;
        [[nodiscard]] std::vector<std::pair<std::string, std::string>> getDataInformation() const override;

        [[nodiscard]] bool open() override;
        void close() override;

        [[nodiscard]] bool hasLoadInterface() const override;
        void drawLoadInterface() override;

        inline u64 getBase() const { return base_; }

    private:
        struct ProcessInfo {
            std::string label = {};
            std::string name = {};
            DWORD pid = {};
        };

        HANDLE handle_ = INVALID_HANDLE_VALUE;
        u64 base_ = {};
        bool open_write_ = false;
        ProcessInfo open_info_ = {};

        bool select_write_ = false;
        ProcessInfo select_info_ = {};
        std::string select_filter_ = {};
        std::vector<ProcessInfo> select_list_ = {};

        void refreshSelectList();
    };
}  // namespace hex::plugin::rpm
