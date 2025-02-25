#pragma once
#include <windows.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <winternl.h>
#include <stdexcept>
#include <iostream>
#pragma comment(lib, "ntdll.lib")

// Исправленная сигнатура для 64-битных систем
extern "C" NTSTATUS NTAPI NtReadVirtualMemory(
    HANDLE  ProcessHandle,
    PVOID   BaseAddress,
    PVOID   Buffer,
    SIZE_T  NumberOfBytesToRead,
    PSIZE_T NumberOfBytesRead
);

namespace MemoryUtils {
    extern uintptr_t baseAddress;
    extern DWORD processId;
    extern HANDLE processHandle;

    namespace Internal {
        DWORD FindProcessId(const wchar_t* target);
        uintptr_t FindModuleBase(DWORD pid, const wchar_t* module);
        void RandomDelay();
    }

    template <typename T>
    T ReadData(uintptr_t address) {
        T data{};
        SIZE_T bytesRead = 0; // Заменяем ULONG на SIZE_T
        if (processHandle == nullptr || processHandle == INVALID_HANDLE_VALUE) {
            std::cout << "[!] Error: u must start dota 2, before Helper for safe work!" << std::endl;
            throw std::runtime_error("Invalid process handle");
            exit(10000);
        }
        NTSTATUS status = NtReadVirtualMemory(
            processHandle,
            reinterpret_cast<void*>(address),
            &data,
            sizeof(T),
            &bytesRead
        );
        if (status != 0 || bytesRead != sizeof(T)) {
            throw std::runtime_error("Failed to read memory");
        }
        return data;
    }

    template <typename T>
    bool WriteData(uintptr_t address, const T& value) {
        SIZE_T bytesWritten;
        return NT_SUCCESS(
            NtWriteVirtualMemory(
                processHandle,
                reinterpret_cast<void*>(address),
                &value,
                sizeof(T),
                &bytesWritten
            )
        );
    }

    std::string ReadString(uintptr_t address, size_t maxLen = 256);
    std::string ReadComplexString(uintptr_t address);
}
