#include "Entities.hpp"

#include "../data/dynamic_util.hpp"
#include "../assets/Assets.hpp"
#include "../world/Level.hpp"
#include "../physics/Hitbox.hpp"
#include "../physics/PhysicsSolver.hpp"
#include "../graphics/render/ModelBatch.hpp"
#include "../graphics/core/LineBatch.hpp"
#include "../graphics/core/Model.hpp"
#include "../maths/FrustumCulling.hpp"
#include "../objects/EntityDef.hpp"
#include "../objects/rigging.hpp"
#include "../logic/scripting/scripting.hpp"

#include <glm/ext/matrix_transform.hpp>

void Transform::refresh() {
    combined = glm::mat4(1.0f);
    combined = glm::translate(combined, pos);
    combined = glm::scale(combined, size);
    combined = combined * glm::mat4(rot);
    dirty = false;
}

void Entity::destroy() {
    if (isValid()){
        entities.despawn(id);
    }
}

rigging::Rig& Entity::getModeltree() const {
    return registry.get<rigging::Rig>(entity);
}

void Entity::setRig(rigging::RigConfig* rigConfig) {
    auto& rig = registry.get<rigging::Rig>(entity);
    rig.config = rigConfig;
    rig.pose.matrices.resize(rigConfig->getNodes().size(), glm::mat4(1.0f));
    rig.calculated.matrices.resize(rigConfig->getNodes().size(), glm::mat4(1.0f));
}

Entities::Entities(Level* level) : level(level) {
}

template<void(*callback)(const Entity&, size_t, entityid_t)>
static triggercallback create_trigger_callback(Entities* entities) {
    return [=](auto entityid, auto index, auto otherid) {
        if (auto entity = entities->get(entityid)) {
            if (entity->isValid()) {
                callback(*entity, index, otherid);
            }
        }
    };
}

entityid_t Entities::spawn(
    Assets* assets,
    EntityDef& def,
    glm::vec3 pos,
    dynamic::Value args) 
{
    auto rig = assets->get<rigging::RigConfig>(def.rigName);
    if (rig == nullptr) {
        throw std::runtime_error("rig "+def.rigName+" not found");
    }
    auto entity = registry.create();
    glm::vec3 size(1);
    auto id = nextID++;
    registry.emplace<EntityId>(entity, static_cast<entityid_t>(id), def);
    registry.emplace<Transform>(entity, pos, size, glm::mat3(1.0f));
    auto& body = registry.emplace<Rigidbody>(
        entity, true, Hitbox {pos, def.hitbox}, std::vector<Trigger>{});

    body.triggers.resize(def.radialTriggers.size() + def.boxTriggers.size());
    for (auto& [i, box] : def.boxTriggers) {
        TriggerParams params {};
        params.aabb = box;
        body.triggers[i] = Trigger {
            true, TriggerType::AABB, i, id, params, params, {}, {},
            create_trigger_callback<scripting::on_trigger_enter>(this),
            create_trigger_callback<scripting::on_trigger_exit>(this)};
    }
    for (auto& [i, radius] : def.radialTriggers) {
        TriggerParams params {};
        params.radial = glm::vec4(radius);
        body.triggers[i] = Trigger {
            true, TriggerType::RADIUS, i, id, params, params, {}, {},
            create_trigger_callback<scripting::on_trigger_enter>(this),
            create_trigger_callback<scripting::on_trigger_exit>(this)};
    }
    auto& scripting = registry.emplace<ScriptComponents>(entity);
    entities[id] = entity;
    uids[entity] = id;
    registry.emplace<rigging::Rig>(entity, rig->instance());
    for (auto& componentName : def.components) {
        auto component = std::make_unique<UserComponent>(
            componentName, entity_funcs_set {}, nullptr);
        scripting.components.emplace_back(std::move(component));
    }
    scripting::on_entity_spawn(def, id, scripting.components, std::move(args));
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

void Entities::onSave(const Entity& entity) {
    scripting::on_entity_save(entity);
}

dynamic::Value Entities::serialize(const Entity& entity) {
    auto root = dynamic::create_map();
    auto& eid = entity.getID();
    auto& def = eid.def;
    root->put("def", def.name);
    root->put("uid", eid.uid);
    {
        auto& transform = entity.getTransform();
        auto& tsfmap = root->putMap("transform");
        tsfmap.put("pos", dynamic::to_value(transform.pos));
        if (transform.size != glm::vec3(1.0f)) {
            tsfmap.put("size", dynamic::to_value(transform.size));
        }
        tsfmap.put("rot", dynamic::to_value(transform.rot));
    }
    {
        auto& rigidbody = entity.getRigidbody();
        auto& bodymap = root->putMap("rigidbody");
        if (!rigidbody.enabled) {
            bodymap.put("enabled", rigidbody.enabled);
        }
        bodymap.put("vel", dynamic::to_value(rigidbody.hitbox.velocity));
        bodymap.put("damping", rigidbody.hitbox.linearDamping);
    }
    auto& rig = entity.getModeltree();
    if (rig.config->getName() != def.rigName) {
        root->put("rig", rig.config->getName());
    }
    if (def.save.rig.pose || def.save.rig.textures) {
        auto& rigmap = root->putMap("rig");
        if (def.save.rig.textures) {
            auto& map = rigmap.putMap("textures");
            for (auto& entry : rig.textures) {
                map.put(entry.first, entry.second);
            }
        }
        if (def.save.rig.pose) {
            auto& list = rigmap.putList("pose");
            for (auto& mat : rig.pose.matrices) {
                list.put(dynamic::to_value(mat));
            }
        }
    }
    auto& scripts = entity.getScripting();
    if (!scripts.components.empty()) {
        auto& compsMap = root->putMap("comps");
        for (auto& comp : scripts.components) {
            auto data = scripting::get_component_value(comp->env, "SAVED_DATA");
            compsMap.put(comp->name, data);
        }
    }
    return root;
}

void Entities::clean() {
    for (auto it = entities.begin(); it != entities.end();) {
        if (!registry.get<EntityId>(it->second).destroyFlag) {
            ++it;
        } else {
            uids.erase(it->second);
            registry.destroy(it->second);
            it = entities.erase(it);
        }
    }
}

void Entities::preparePhysics() {
    static uint64_t frameid = 0;
    frameid++;
    auto view = registry.view<EntityId, Transform, Rigidbody>();
    auto physics = level->physics.get();
    std::vector<Trigger*> triggers;
    for (auto [entity, eid, transform, rigidbody] : view.each()) {
        if (!rigidbody.enabled) {
            continue;
        }
        // TODO: temporary optimization until threaded solution
        if ((eid.uid + frameid) % 3 != 0) {
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

            switch (trigger.type) {
                case TriggerType::AABB:
                    trigger.calculated.aabb = trigger.params.aabb;
                    trigger.calculated.aabb.transform(transform.combined);
                    break;
                case TriggerType::RADIUS:
                    trigger.calculated.radial = glm::vec4(
                        rigidbody.hitbox.position.x,
                        rigidbody.hitbox.position.y,
                        rigidbody.hitbox.position.z,
                        trigger.params.radial.w*
                        trigger.params.radial.w);
                    break;
            }
            triggers.push_back(&trigger);
        }
    }
    physics->setTriggers(std::move(triggers));
}

void Entities::updatePhysics(float delta) {
    preparePhysics();

    auto view = registry.view<EntityId, Transform, Rigidbody>();
    auto physics = level->physics.get();
    for (auto [entity, eid, transform, rigidbody] : view.each()) {
        if (!rigidbody.enabled) {
            continue;
        }
        auto& hitbox = rigidbody.hitbox;
        auto prevVel = hitbox.velocity;
        bool grounded = hitbox.grounded;

        float vel = glm::length(prevVel);
        int substeps = static_cast<int>(delta * vel * 20);
        substeps = std::min(100, std::max(2, substeps));
        physics->step(
            level->chunks.get(),
            &hitbox,
            delta,
            substeps,
            false,
            1.0f,
            true,
            eid.uid
        );
        hitbox.linearDamping = hitbox.grounded * 24;
        transform.setPos(hitbox.position);
        if (hitbox.grounded && !grounded) {
            scripting::on_entity_grounded(
                *get(eid.uid), glm::length(prevVel-hitbox.velocity));
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
        if (transform.dirty) {
            transform.refresh();
        }
    }
}

void Entities::renderDebug(LineBatch& batch, const Frustum& frustum) {
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
            if (trigger.type != TriggerType::AABB)
                continue;
            batch.box(
                trigger.calculated.aabb.center(), 
                trigger.calculated.aabb.size(), 
                glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        }
    }
}

void Entities::render(Assets* assets, ModelBatch& batch, const Frustum& frustum) {
    auto view = registry.view<Transform, rigging::Rig>();
    for (auto [entity, transform, rig] : view.each()) {
        const auto& pos = transform.pos;
        const auto& size = transform.size;
        if (frustum.isBoxVisible(pos-size, pos+size)) {
            const auto* rigConfig = rig.config;
            rigConfig->render(assets, batch, rig, transform.combined);
        }
    }
}

std::vector<Entity> Entities::getAllInside(AABB aabb) {
    std::vector<Entity> collected;
    auto view = registry.view<Transform>();
    for (auto [entity, transform] : view.each()) {
        if (aabb.contains(transform.pos)) {
            const auto& found = uids.find(entity);
            if (found == uids.end()) {
                continue;
            }
            if (auto entity = get(found->second)) {
                collected.push_back(*entity);
            }
        }
    }
    return collected;
}
