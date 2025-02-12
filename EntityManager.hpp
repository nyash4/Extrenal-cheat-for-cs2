#pragma once
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include "EntityData.hpp"
#include "../ImguiMenu/Memory/memory.h"
#include "Offsets.h"

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    std::vector<EntityData> GetEntities() const;
    int GetLocalTeam() const;
    int GetLocalPlayerIndex() const;

private:
    void EntityUpdateThread();

    mutable std::mutex entitiesMutex;
    std::vector<EntityData> entities;
    std::atomic<int> g_LocalTeam{ 0 };
    std::atomic<int> g_localPlayerIndex{ 0 };
    std::atomic<bool> running{ false };
    std::thread updateThread;
};