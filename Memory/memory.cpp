#include "memory.h"
#include <ntstatus.h>
#include <winternl.h>
#include <random>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

#pragma comment(lib, "ntdll.lib")

typedef CLIENT_ID* PCLIENT_ID; // Add this line to define PCLIENT_ID

extern "C" {
    NTSTATUS NTAPI NtOpenProcess(
        PHANDLE ProcessHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        PCLIENT_ID ClientId
    );
}

namespace MemoryUtils {
    namespace Internal {
        void RandomDelay() {
            static std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<> dist(1, 15);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
        }

        DWORD FindProcessId(const wchar_t* target) {
            std::cout << "Preparing, wait..." << std::endl;
            DWORD pid = 0;
            PROCESSENTRY32W pe = { sizeof(pe) };

            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snap == INVALID_HANDLE_VALUE) return 0;

            if (Process32FirstW(snap, &pe)) {
                do {
                    if (_wcsicmp(pe.szExeFile, target) == 0) {
                        pid = pe.th32ProcessID;
                        break;
                    }
                    RandomDelay();
                } while (Process32NextW(snap, &pe));
            }
            CloseHandle(snap);
            return pid;
        }

        uintptr_t FindModuleBase(DWORD pid, const wchar_t* module) {
            uintptr_t base = 0;
            MODULEENTRY32W me = { sizeof(me) };

            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
            if (snap == INVALID_HANDLE_VALUE) return 0;

            if (Module32FirstW(snap, &me)) {
                do {
                    if (_wcsicmp(me.szModule, module) == 0) {
                        base = reinterpret_cast<uintptr_t>(me.modBaseAddr);
                        break;
                    }
                    RandomDelay();
                } while (Module32NextW(snap, &me));
            }
            CloseHandle(snap);
            //std::cout << "Preparing, wait..." << std::endl;
            return base;
        }
    }

    DWORD processId = Internal::FindProcessId(L"cs2.exe");
    uintptr_t baseAddress = Internal::FindModuleBase(processId, L"client.dll");
    HANDLE processHandle = []() {
        if (processId == 0) return HANDLE(0);

        OBJECT_ATTRIBUTES oa;
        InitializeObjectAttributes(&oa, nullptr, 0, nullptr, nullptr);

        CLIENT_ID cid = { reinterpret_cast<void*>(processId), nullptr };
        HANDLE hProcess = nullptr;

        NtOpenProcess(&hProcess, PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, &oa, &cid);
        return hProcess;
        }();

    std::string ReadString(uintptr_t address, size_t maxLen) {
        std::vector<char> buffer(maxLen);
        SIZE_T bytesRead; // Change ULONG to SIZE_T
        if (NT_SUCCESS(NtReadVirtualMemory(processHandle, reinterpret_cast<void*>(address), buffer.data(), buffer.size() - 1, &bytesRead))) {
            buffer[bytesRead] = '\0';
            return std::string(buffer.data());
        }
        return "";
    }



    std::string ReadComplexString(uintptr_t address) {
        struct StringData {
            uintptr_t buffer;
            size_t size;
            size_t capacity;
        } strData;

        if (!NT_SUCCESS(NtReadVirtualMemory(processHandle, reinterpret_cast<void*>(address), &strData, sizeof(strData), nullptr))) {
            return "";
        }

        if (strData.size == 0 || strData.buffer == 0) return "";

        std::vector<char> buffer(strData.size + 1);
        if (NT_SUCCESS(NtReadVirtualMemory(processHandle, reinterpret_cast<void*>(strData.buffer), buffer.data(), strData.size, nullptr))) {
            buffer[strData.size] = '\0';
            return std::string(buffer.data());
        }
        return "";
    }
}
