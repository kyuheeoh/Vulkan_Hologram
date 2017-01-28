#pragma once

#ifndef SIMULATION_H
#define SIMULATION_H

#include <memory>
#include <random>
#include <vector>

#include <glm/glm.hpp>

#include "Meshes.h"

class Animation {
public:
    Animation(unsigned rng_seed, float scale);

    glm::mat4 transformation(float t);
    float transparency();

private:
    struct Data {
        glm::vec3 axis;
        float speed;
        float scale;

        glm::mat4 matrix;

        float alpha;
        float alpha_inc;
    };

    std::mt19937 rng_;
    std::uniform_real_distribution<float> dir_;
    std::uniform_real_distribution<float> speed_;

    Data current_;
};

class Curve;

class Path {
public:
    Path(unsigned rng_seed);

    glm::vec3 position(float t);

private:
    struct Subpath {
        glm::vec3 origin;
        float start;
        float end;
        float now;

        std::shared_ptr<Curve> curve;
    };

    void generate_subpath();

    std::mt19937 rng_;
    std::uniform_int_distribution<> type_;
    std::uniform_real_distribution<float> duration_;

    Subpath current_;
};

class Simulation {
public:
    Simulation(int object_count);

    struct Object {
        Meshes::Type mesh;
        glm::vec3 light_pos;
        glm::vec3 light_color;

        Animation animation;
        Path path;

        uint32_t frame_data_offset;

        glm::mat4 model;
        float alpha;
    };

    const std::vector<Object> &objects() const { return objects_; }

    unsigned int rng_seed() { return random_dev_(); }

    void set_frame_data_size(uint32_t size);
    void update(float time, int begin, int end);

private:
    std::random_device random_dev_;
    std::vector<Object> objects_;
};

#endif // SIMULATION_H
