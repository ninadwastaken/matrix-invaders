#include "Scene.h"

class LevelE : public Scene {
public:
    // ����� STATIC ATTRIBUTES ����� //
    int ENEMY_COUNT = 6;
    int PROJECTILE_COUNT = 6;

    // ����� DESTRUCTOR ����� //
    ~LevelE();

    // ����� METHODS ����� //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
