#include "Entities.hpp"

#include "../assets/Assets.hpp"
#include "../world/Level.hpp"
#include "../physics/Hitbox.hpp"
#include "../physics/PhysicsSolver.hpp"
#include "../graphics/render/ModelBatch.hpp"
#include "../graphics/core/LineBatch.hpp"
#include "../graphics/core/Model.hpp"
#include "../maths/FrustumCulling.hpp"
#include "../objects/EntityDef.hpp"
#include "../logic/scripting/scripting.hpp"

#include <glm/ext/matrix_transform.hpp>

void Transform::refresh() {
    combined = glm::mat4(1.0f);
    combined = glm::translate(combined, pos);
    combined = glm::scale(combined, size);
    combined = combined * glm::mat4(rot);
}

void Entity::destroy() {
    if (isValid()){
        entities.despawn(id);
    }
}

Entities::Entities(Level* level) : level(level) {
}

entityid_t Entities::spawn(EntityDef& def, glm::vec3 pos) {
    auto entity = registry.create();
    glm::vec3 size(1);
    auto id = nextID++;
    registry.emplace<EntityId>(entity, static_cast<entityid_t>(id), def);
    registry.emplace<Transform>(entity, pos, size/4.0f, glm::mat3(1.0f));
    registry.emplace<Rigidbody>(entity, true, Hitbox {pos, def.hitbox}, std::vector<Trigger>{
        {true, id, AABB {glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{1.0f, 1.0f, 1.0f}}, {}, {}, {},
        [=](auto entityid, auto index, auto otherid) {
            if (auto entity = get(entityid)) {
                if (entity->isValid()) {
                    scripting::on_trigger_enter(*entity, index, otherid);
                }
            }
        }, [=](auto entityid, auto index, auto otherid) {
            if (auto entity = get(entityid)) {
                if (entity->isValid()) {
                    scripting::on_trigger_exit(*entity, index, otherid);
                }
            }
        }}
    });
    auto& scripting = registry.emplace<Scripting>(entity, entity_funcs_set {}, nullptr);
    entities[id] = entity;
    scripting.env = scripting::on_entity_spawn(def, id, scripting.funcsset);
    return id;
}

void Entities::despawn(entityid_t id) {
    if (auto entity = get(id)) {
        auto& eid = entity->getID();
        if (!eid.destroyFlag) {
            eid.destroyFlag = true;
            scripting::on_entity_despawn(entity->getDef(), *entity);
        }
    }
}

void Entities::clean() {
    for (auto it = entities.begin(); it != entities.end();) {
        if (!registry.get<EntityId>(it->second).destroyFlag) {
            ++it;
        } else {
            registry.destroy(it->second);
            it = entities.erase(it);
        }
    }
}
#include "../util/timeutil.hpp"
void Entities::updatePhysics(float delta){
    auto view = registry.view<EntityId, Transform, Rigidbody>();
    auto physics = level->physics.get();
    {
        std::vector<Trigger*> triggers;
        for (auto [entity, eid, transform, rigidbody] : view.each()) {
            if (!rigidbody.enabled) {
                continue;
            }
            for (size_t i = 0; i < rigidbody.triggers.size(); i++) {
                auto& trigger = rigidbody.triggers[i];
                for (auto oid : trigger.prevEntered) {
                    if (trigger.nextEntered.find(oid) == trigger.nextEntered.end()) {
                        trigger.exitCallback(trigger.entity, i, oid);
                    }
                }
                trigger.prevEntered = trigger.nextEntered;
                trigger.nextEntered.clear();
                trigger.calculated = trigger.aabb;
                trigger.calculated.transform(transform.combined);
                triggers.push_back(&trigger);
            }
        }
        physics->setTriggers(std::move(triggers));
    }
    for (auto [entity, eid, transform, rigidbody] : view.each()) {
        if (!rigidbody.enabled) {
            continue;
        }
        auto& hitbox = rigidbody.hitbox;
        auto prevVel = hitbox.velocity;
        bool grounded = hitbox.grounded;
        physics->step(
            level->chunks.get(),
            &hitbox,
            delta,
            10,
            false,
            1.0f,
            true,
            eid.uid
        );
        hitbox.linearDamping = hitbox.grounded * 24;
        transform.pos = hitbox.position;
        if (hitbox.grounded && !grounded) {
            scripting::on_entity_grounded(*get(eid.uid), glm::length(prevVel-hitbox.velocity));
        }
        if (!hitbox.grounded && grounded) {
            scripting::on_entity_fall(*get(eid.uid));
        }
    }
}

void Entities::update() {
    scripting::on_entities_update();
    auto view = registry.view<Transform>();
    for (auto [entity, transform] : view.each()) {
        transform.refresh();
    }
}

void Entities::renderDebug(LineBatch& batch, Frustum& frustum) {
    batch.lineWidth(1.0f);
    auto view = registry.view<Transform, Rigidbody>();
    for (auto [entity, transform, rigidbody] : view.each()) {
        const auto& hitbox = rigidbody.hitbox;
        const auto& pos = transform.pos;
        const auto& size = transform.size;
        if (!frustum.isBoxVisible(pos-size, pos+size)) {
            continue;
        }
        batch.box(hitbox.position, hitbox.halfsize * 2.0f, glm::vec4(1.0f));

        for (auto& trigger : rigidbody.triggers) {
            batch.box(trigger.calculated.center(), trigger.calculated.size(), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        }
    }
}

void Entities::render(Assets* assets, ModelBatch& batch, Frustum& frustum) {
    auto view = registry.view<Transform>();
    auto model = assets->get<model::Model>("cube");
    for (auto [entity, transform] : view.each()) {
        const auto& pos = transform.pos;
        const auto& size = transform.size;
        if (frustum.isBoxVisible(pos-size, pos+size)) {
            batch.pushMatrix(transform.combined);
            batch.draw(model);
            batch.popMatrix();
        }
    }
}
