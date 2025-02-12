#pragma once

struct Vec3 {
    double x, y, z;
};

struct EntityData {
    Vec3 position; // Мировые координаты
    bool isValid;
    int team;
};