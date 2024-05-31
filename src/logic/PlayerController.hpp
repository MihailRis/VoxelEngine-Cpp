#ifndef PLAYER_CONTROL_HPP_
#define PLAYER_CONTROL_HPP_

#include "../objects/Player.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

class Camera;
class Level;
class Block;
class Chunks;
class BlocksController;
struct CameraSettings;

class CameraControl {
    std::shared_ptr<Player> player;
    std::shared_ptr<Camera> camera;
    const CameraSettings& settings;
    glm::vec3 offset;
    float shake = 0.0f;
    float shakeTimer = 0.0f;
    glm::vec3 interpVel {0.0f};

    /// @brief Update shaking timer and calculate camera offset
    /// @param delta delta time
    /// @return camera offset
    glm::vec3 updateCameraShaking(float delta);

    /// @brief Update field-of-view effects
    /// @param input player inputs
    /// @param delta delta time
    void updateFovEffects(const PlayerInput& input, float delta);

    /// @brief Switch active player camera
    void switchCamera();
public:
    CameraControl(std::shared_ptr<Player> player, const CameraSettings& settings);
    void updateMouse(PlayerInput& input);
    void update(const PlayerInput& input, float delta, Chunks* chunks);
    void refresh();
};

enum class BlockInteraction {
    step,
    destruction,
    placing
};

using on_block_interaction = std::function<void(
    Player*, glm::ivec3, const Block*, BlockInteraction type
)>;

class PlayerController {
    Level* level;
    std::shared_ptr<Player> player;
    PlayerInput input {};
    CameraControl camControl;
    BlocksController* blocksController;

    std::vector<on_block_interaction> blockInteractionCallbacks;

    void updateKeyboard();
    void updateCamera(float delta, bool movement);
    void resetKeyboard();
    void updateControls(float delta);
    void updateInteraction();
    void onBlockInteraction(
        glm::ivec3 pos,
        const Block* def,
        BlockInteraction type
    );

    float stepsTimer = 0.0f;
    void onFootstep();
    void updateFootsteps(float delta);
public:
    static glm::ivec3 selectedBlockNormal;
    static glm::vec3 selectedPointPosition;
    static int selectedBlockId;
    static int selectedBlockRotation;

    PlayerController(
        Level* level, 
        const EngineSettings& settings,
        BlocksController* blocksController
    );
    void update(float delta, bool input, bool pause);

    Player* getPlayer();

    void listenBlockInteraction(on_block_interaction callback);
};

#endif /* PLAYER_CONTROL_HPP_ */
