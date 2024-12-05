#include "Scene.h"

class LevelD : public Scene {
public:
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 3;
    int PROJECTILE_COUNT = 3;

    // ————— DESTRUCTOR ————— //
    ~LevelD();

    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
