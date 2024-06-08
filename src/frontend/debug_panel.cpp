#include "../audio/audio.hpp"
#include "../delegates.hpp"
#include "../engine.hpp"
#include "../settings.hpp"
#include "../graphics/core/Mesh.hpp"
#include "../graphics/ui/elements/CheckBox.hpp"
#include "../graphics/ui/elements/TextBox.hpp"
#include "../graphics/ui/elements/TrackBar.hpp"
#include "../graphics/ui/elements/InputBindBox.hpp"
#include "../graphics/render/WorldRenderer.hpp"
#include "../objects/Player.hpp"
#include "../physics/Hitbox.hpp"
#include "../util/stringutil.hpp"
#include "../voxels/Block.hpp"
#include "../voxels/Chunk.hpp"
#include "../voxels/Chunks.hpp"
#include "../world/Level.hpp"
#include "../world/World.hpp"

#include <string>
#include <memory>
#include <sstream>
#include <bitset>
#include <utility>

using namespace gui;

static std::shared_ptr<Label> create_label(wstringsupplier supplier) {
    auto label = std::make_shared<Label>(L"-");
    label->textSupplier(std::move(supplier));
    return label;
}

std::shared_ptr<UINode> create_debug_panel(
    Engine* engine, 
    Level* level, 
    Player* player
) {
    auto panel = std::make_shared<Panel>(glm::vec2(250, 200), glm::vec4(5.0f), 2.0f);
    panel->setId("hud.debug-panel");
    panel->setPos(glm::vec2(10, 10));

    static int fps = 0;
    static int fpsMin = fps;
    static int fpsMax = fps;
    static std::wstring fpsString = L"";

    panel->listenInterval(0.016f, [engine]() {
        fps = 1.0f / engine->getDelta();
        fpsMin = std::min(fps, fpsMin);
        fpsMax = std::max(fps, fpsMax);
    });

    panel->listenInterval(0.5f, []() {
        fpsString = std::to_wstring(fpsMax)+L" / "+std::to_wstring(fpsMin);
        fpsMin = fps;
        fpsMax = fps;
    });
    panel->add(create_label([](){ return L"fps: "+fpsString;}));
   
    panel->add(create_label([](){
        return L"meshes: " + std::to_wstring(Mesh::meshesCount);
    }));
    panel->add(create_label([](){
        return L"speakers: " + std::to_wstring(audio::count_speakers())+
               L" streams: " + std::to_wstring(audio::count_streams());
    }));
    panel->add(create_label([=](){
        auto& settings = engine->getSettings();
        bool culling = settings.graphics.frustumCulling.get();
        return L"frustum-culling: "+std::wstring(culling ? L"on" : L"off");
    }));
    panel->add(create_label([=]() {
        return L"chunks: "+std::to_wstring(level->chunks->chunksCount)+
               L" visible: "+std::to_wstring(level->chunks->visible);
    }));
    panel->add(create_label([=](){
        std::wstringstream stream;
        stream << "r:" << player->selectedVoxel.state.rotation << " s:"
                << std::bitset<3>(player->selectedVoxel.state.segment) << " u:"
                << std::bitset<8>(player->selectedVoxel.state.userbits);
        if (player->selectedVoxel.id == BLOCK_VOID) {
            return std::wstring {L"block: -"};
        } else {
            return L"block: "+std::to_wstring(player->selectedVoxel.id)+
                   L" "+stream.str();
        }
    }));
    panel->add(create_label([=](){
        auto* indices = level->content->getIndices();
        if (auto def = indices->getBlockDef(player->selectedVoxel.id)) {
            return L"name: " + util::str2wstr_utf8(def->name);
        } else {
            return std::wstring {L"name: void"};
        }
    }));
    panel->add(create_label([=](){
        return L"seed: "+std::to_wstring(level->getWorld()->getSeed());
    }));

    for (int ax = 0; ax < 3; ax++) {
        auto sub = std::make_shared<Container>(glm::vec2(250, 27));

        std::wstring str = L"x: ";
        str[0] += ax;
        auto label = std::make_shared<Label>(str);
        label->setMargin(glm::vec4(2, 3, 2, 3));
        label->setSize(glm::vec2(20, 27));
        sub->add(label);
        sub->setColor(glm::vec4(0.0f));

        // Coord input
        auto box = std::make_shared<TextBox>(L"");
        auto boxRef = box.get();
        box->setTextSupplier([=]() {
            Hitbox* hitbox = player->hitbox.get();
            return util::to_wstring(hitbox->position[ax], 2);
        });
        box->setTextConsumer([=](const std::wstring& text) {
            try {
                glm::vec3 position = player->hitbox->position;
                position[ax] = std::stoi(text);
                player->teleport(position);
            } catch (std::invalid_argument& _){
            } catch (std::out_of_range & _) {
            }
        });
        box->setOnEditStart([=](){
            Hitbox* hitbox = player->hitbox.get();
            boxRef->setText(std::to_wstring(int(hitbox->position[ax])));
        });
        box->setSize(glm::vec2(230, 27));

        sub->add(box, glm::vec2(20, 0));
        panel->add(sub);
    }
    panel->add(create_label([=](){
        int hour, minute, second;
        timeutil::from_value(level->getWorld()->daytime, hour, minute, second);

        std::wstring timeString = 
                util::lfill(std::to_wstring(hour), 2, L'0') + L":" +
                util::lfill(std::to_wstring(minute), 2, L'0');
        return L"time: "+timeString;
    }));
    {
        auto bar = std::make_shared<TrackBar>(0.0f, 1.0f, 1.0f, 0.005f, 8);
        bar->setSupplier([=]() {return level->getWorld()->daytime;});
        bar->setConsumer([=](double val) {level->getWorld()->daytime = val;});
        panel->add(bar);
    }
    {
        auto bar = std::make_shared<TrackBar>(0.0f, 1.0f, 0.0f, 0.005f, 8);
        bar->setSupplier([=]() {return level->getWorld()->fog;});
        bar->setConsumer([=](double val) {level->getWorld()->fog = val;});
        panel->add(bar);
    }
    {
        auto checkbox = std::make_shared<FullCheckBox>(
            L"Show Chunk Borders", glm::vec2(400, 24)
        );
        checkbox->setSupplier([=]() {
            return WorldRenderer::showChunkBorders;
        });
        checkbox->setConsumer([=](bool checked) {
            WorldRenderer::showChunkBorders = checked;
        });
        panel->add(checkbox);
    }
    panel->refresh();
    return panel;
}
