#pragma once

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

class Memory{
    private:
        std::uintptr_t processID = 0;
        void* processHandle = nullptr;

    public:
        // Memory constructor to find the processID and opening the handle to the process
        Memory(const std::string processName){
            ::PROCESSENTRY32 processEntry = { };
            processEntry.dwSize = sizeof(::PROCESSENTRY32);

            const HANDLE processSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

            while(::Process32Next(processSnapshot, &processEntry)){
                if(!processName.compare(processEntry.szExeFile)){
                    processID = processEntry.th32ProcessID;
                    processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
                    break;
                }
            }

            if (processSnapshot){
                ::CloseHandle(processSnapshot);
            }
        }
        
        // Memory Deconstructor to cleanup the closing process handle to the game memory
        ~Memory(){
            if(processHandle){
                ::CloseHandle(processHandle);
            }
        }

        const std::uintptr_t getModuleBaseAddress(const std::string moduleName){
           ::MODULEENTRY32 moduleEntry = { };
           moduleEntry.dwSize = sizeof(::MODULEENTRY32);

           const auto processSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
           std::uintptr_t moduleBaseAddress = 0;
           while(::Module32Next(processSnapshot, &moduleEntry)){
               if(!moduleName.compare(moduleEntry.szModule)){
                   moduleBaseAddress = reinterpret_cast<std::uintptr_t>(moduleEntry.modBaseAddr);
                   break;
               }
           }
           
           if(processSnapshot){
               ::CloseHandle(processSnapshot);
           }

           return moduleBaseAddress;
        }


        // Read process memory template
        template <typename T>
        constexpr const T Read(const std::uintptr_t& address){
            T value = {};
            ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL);
            return value;
        }

        // Write process memory template
        template <typename T>
        constexpr void Write(const std::uintptr_t& address, const T& value){
            ::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL);
        }
};
