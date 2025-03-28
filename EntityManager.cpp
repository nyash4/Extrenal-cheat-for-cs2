#include "EntityManager.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>

EntityManager::EntityManager() {
    entities.resize(256);
    running = true;
    updateThread = std::thread(&EntityManager::EntityUpdateThread, this);
}

EntityManager::~EntityManager() {
    running = false;
    if (updateThread.joinable()) {
        updateThread.join();
    }
}

std::vector<EntityData> EntityManager::GetEntities() const {
    std::lock_guard<std::mutex> lock(entitiesMutex);
    return entities;
}

int EntityManager::GetLocalTeam() const {
    return g_LocalTeam.load();
}

int EntityManager::GetLocalPlayerIndex() const {
    return g_localPlayerIndex.load();    
}

void EntityManager::EntityUpdateThread() {
    while (running) {
        uintptr_t localPlayer = 0;
        try {
            localPlayer = MemoryUtils::ReadData<uintptr_t>(MemoryUtils::baseAddress + offsets::dwLoclalPlayerPawn);
        }
        catch (...) {}

        if (localPlayer) {
            try {
                g_LocalTeam = MemoryUtils::ReadData<int>(localPlayer + offsets::m_iTeamNum);
            }
            catch (...) {}
        }

        std::vector<EntityData> newEntities(256);
        for (int i = 0; i < 256; i++) {
            uintptr_t ent = 0;
            try {
                ent = MemoryUtils::ReadData<uintptr_t>(MemoryUtils::baseAddress + offsets::dwEntityList + i * 0x08);
            }
            catch (...) {}

            if (ent == localPlayer) g_localPlayerIndex = i;

            if (!ent || ent == (uintptr_t)-1) {
                newEntities[i].isValid = false;
                continue;
            }

            try {
                int hp = MemoryUtils::ReadData<int>(ent + offsets::m_iHealth);
                if (hp <= 0 || hp > 100) {
                    newEntities[i].isValid = false;
                    continue;
                }

                newEntities[i].team = MemoryUtils::ReadData<int>(ent + offsets::m_iTeamNum);
                newEntities[i].position.x = MemoryUtils::ReadData<float>(ent + offsets::XPos);
                newEntities[i].position.y = MemoryUtils::ReadData<float>(ent + offsets::YPos);
				newEntities[i].position.z = MemoryUtils::ReadData<float>(ent + offsets::ZPos);
                newEntities[i].isValid = true;
            }
            catch (...) {
                newEntities[i].isValid = false;
            }
        }

        {
            std::lock_guard<std::mutex> lock(entitiesMutex);
            entities = std::move(newEntities);
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}