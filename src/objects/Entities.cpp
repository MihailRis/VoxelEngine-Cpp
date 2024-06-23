#include "Entities.hpp"

#include "../assets/Assets.hpp"
#include "../world/Level.hpp"
#include "../physics/Hitbox.hpp"
#include "../physics/PhysicsSolver.hpp"
#include "../graphics/render/ModelBatch.hpp"
#include "../graphics/core/Model.hpp"

#include <glm/ext/matrix_transform.hpp>

void Transform::refresh() {
    combined = glm::mat4(1.0f);
    combined = glm::translate(combined, pos);
}

Entities::Entities(Level* level) : level(level) {
    auto entity = registry.create();
    glm::vec3 pos(0.5f, 170, 0.5f);
    glm::vec3 size(1);
    registry.emplace<EntityId>(entity, 1);
    registry.emplace<Transform>(entity, pos, size, glm::mat3(1.0f));
    registry.emplace<Hitbox>(entity, pos, size/20.0f);
}

void Entities::updatePhysics(float delta){
    auto view = registry.view<Transform, Hitbox>();
    auto physics = level->physics.get();
    for (auto [entity, transform, hitbox] : view.each()) {
        physics->step(
            level->chunks.get(),
            &hitbox,
            delta,
            10,
            false,
            1.0f,
            true
        );
        transform.pos = hitbox.position;
        if (hitbox.grounded) {
            hitbox.velocity.y = 10;
        }
    }
}

void Entities::render(Assets* assets, ModelBatch& batch) {
    auto view = registry.view<Transform>();
    auto model = assets->get<model::Model>("dingus");
    for (auto [entity, transform] : view.each()) {
        transform.refresh();
        batch.pushMatrix(transform.combined);
        batch.draw(model);
        batch.popMatrix();
    }
}
