//
// Created by chelovek on 11/20/23.
//

#ifndef UNIFORM_VALUES_H
#define UNIFORM_VALUES_H

#include <string>
#include <unordered_map>
#include <variant>
#include <glm/glm.hpp>

#include "Tools.h"
#include "uniforms/BackgroundUniform.h"
#include "uniforms/FogUniform.h"
#include "uniforms/LightUniform.h"
#include "uniforms/ProjectionViewUniform.h"
#include "uniforms/SkyboxUniform.h"
#include "uniforms/StateUniform.h"

namespace vulkan {
    using UniformValue = std::variant<std::monostate,
        float, int,
        glm::vec2, glm::vec3, glm::vec4,
        glm::mat4,
        std::vector<glm::vec3>>;

    template<typename T>
    inline constexpr bool is_uniform_type_v = tools::is_same_of_types_v<T, float, int, glm::vec2, glm::vec3, glm::vec4, glm::mat4, std::vector<glm::vec3>>;

    // this class use for generate vulkan uniforms
    // Vulkan shaders doesn't support OpenGL uniform: uniform mat4 name
    class UniformValues {
        std::unordered_map<std::string, UniformValue> m_uniformValues;
    public:
        UniformValues() = default;

        template<typename T>
        void addOrUpdate(const std::string &name, const T &value) {
            static_assert(is_uniform_type_v<T>, "Value type can be: float, int, glm::vec2, glm::vec3, glm::vec4, glm::mat4");
            m_uniformValues[name] = value;
        }

        template<typename T, size_t N>
        void addOrUpdate(const std::string &name, const std::array<T, N> &value) {
            static_assert(is_uniform_type_v<T>, "Value type can be: float, int, glm::vec2, glm::vec3, glm::vec4, glm::mat4");
            std::vector<T> res;
            std::copy(value.begin(), value.end(), std::back_inserter(res));
            m_uniformValues[name] = res;
        }

        template<typename T>
        void addOrUpdate(const std::string &name, const std::vector<T> &value) {
            static_assert(is_uniform_type_v<T>, "Value type can be: float, int, glm::vec2, glm::vec3, glm::vec4, glm::mat4");
            std::vector<T> res;
            std::copy(value.begin(), value.end(), std::back_inserter(res));
            m_uniformValues[name] = res;
        }

        template<typename T>
        const T &getUniformValue(const std::string &name) const {
            static_assert(is_uniform_type_v<T>, "Type can be: float, int, glm::vec2, glm::vec3, glm::vec4, glm::mat4");
            static T temp{};
            if (m_uniformValues.find(name) == m_uniformValues.end()) {
                return temp;
            }
            return std::get<T>(m_uniformValues.at(name));
        }

        inline StateUniform getStateUniform() const {
            StateUniform uniform{};
            uniform.model = getUniformValue<glm::mat4>("u_model");
            uniform.projection = getUniformValue<glm::mat4>("u_proj");
            uniform.view = getUniformValue<glm::mat4>("u_view");
            uniform.skyLightColor = getUniformValue<glm::vec3>("u_skyLightColor");
            uniform.cameraPos = getUniformValue<glm::vec3>("u_cameraPos");
            uniform.gamma = getUniformValue<float>("u_gamma");

            return uniform;
        }

        inline LightUniform getLightUniform() const {
            LightUniform uniform{};

            uniform.torchlightColor = getUniformValue<glm::vec3>("u_torchlightColor");
            uniform.torchlightDistance = getUniformValue<float>("u_torchlightDistance");

            return uniform;
        }

        inline FogUniform getFogUniform() const {
            FogUniform uniform{};

            uniform.fogColor = getUniformValue<glm::vec3>("u_fogColor");
            uniform.fogFactor = getUniformValue<float>("u_fogFactor");
            uniform.fogCurve = getUniformValue<float>("u_fogCurve");

            return uniform;
        }

        inline ProjectionViewUniform getProjectionView() const {
            ProjectionViewUniform uniform{};

            uniform.projectionView = getUniformValue<glm::mat4>("u_projview");

            return uniform;
        }

        inline BackgroundUniform getBackgroundUniform() const {
            BackgroundUniform uniform{};

            uniform.view = getUniformValue<glm::mat4>("u_view");
            uniform.ar = getUniformValue<float>("u_ar");
            uniform.zoom = getUniformValue<float>("zoom");

            return uniform;
        }

        inline SkyboxUniform getSkyboxUniform() const {
            SkyboxUniform uniform{};

            auto &xaxis = getUniformValue<std::vector<glm::vec3>>("u_xaxis");
            auto &yaxis = getUniformValue<std::vector<glm::vec3>>("u_yaxis");
            auto &zaxis = getUniformValue<std::vector<glm::vec3>>("u_zaxis");

            std::copy(xaxis.cbegin(), xaxis.cend(), std::begin(uniform.xaxis));
            std::copy(yaxis.cbegin(), xaxis.cend(), std::begin(uniform.yaxis));
            std::copy(zaxis.cbegin(), xaxis.cend(), std::begin(uniform.zaxis));

            uniform.lightDir = getUniformValue<glm::vec3>("u_lightDir");
            uniform.quality = getUniformValue<int>("u_quality");
            uniform.mie = getUniformValue<float>("u_mie");

            return uniform;
        }
    };

} // vulkan

#endif //UNIFORM_VALUES_H
