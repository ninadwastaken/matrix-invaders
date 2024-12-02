#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/neo.png",
PLATFORM_FILEPATH[] = "assets/platformPack_tile027.png",
ENEMY_FILEPATH[] = "assets/smith.png",
PROJECTILE_FILEPATH[] = "assets/projectile.png";

unsigned int LEVELB_DATA[] =
{
    3, 1, 3, 2, 1, 3, 2, 4, 2, 1, 2, 0, 0, 0,
    3, 2, 3, 2, 1, 3, 2, 4, 2, 2, 2, 0, 0, 0,
    3, 2, 3, 2, 1, 3, 2, 4, 2, 2, 2, 0, 0, 0,
    3, 2, 3, 2, 1, 3, 2, 4, 2, 2, 2, 0, 0, 0,
    3, 2, 3, 2, 1, 3, 2, 3, 2, 1, 2, 0, 0, 0,
    3, 2, 3, 2, 1, 3, 2, 3, 2, 1, 1, 1, 1, 1,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

LevelB::~LevelB()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{

    m_game_state.next_scene_id = -1;
    GLuint map_texture_id = Utility::load_texture("assets/Textures-16.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 30, 32);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    GLuint projectile_texture_id = Utility::load_texture(PROJECTILE_FILEPATH);

    int player_walking_animation[4][4] =
    {
        { 1, 5, 9, 13 },  // for George to move to the left,
        { 3, 7, 11, 15 }, // for George to move to the right,
        { 2, 6, 10, 14 }, // for George to move upwards,
        { 0, 4, 8, 12 }   // for George to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        5.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        1,                         // animation column amount
        1,                         // animation row amount
        1.0f,                      // width
        1.0f,                       // height
        PLAYER
    );

    m_game_state.player->set_position(glm::vec3(5.0f, -7.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(5.0f);

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT + PROJECTILE_COUNT];

    /*for (int i = 0; i < ENEMY_COUNT; i++)
    {
    m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    }*/
    auto e1 = new Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    m_game_state.enemies[0] = *e1;
    m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

    auto p1 = new Entity(projectile_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, PROJECTILE, IDLE);
    m_game_state.enemies[1] = *p1;
    m_game_state.enemies[1].set_position(glm::vec3(9.0f, 0.0f, 0.0f));
    m_game_state.enemies[1].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME);

    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT + PROJECTILE_COUNT, m_game_state.map);

    /*for (int i = 0; i < ENEMY_COUNT + PROJECTILE_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    }*/

    m_game_state.enemies[0].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    m_game_state.enemies[1].update(delta_time, &(m_game_state.enemies[0]), NULL, NULL, m_game_state.map);



    if (m_game_state.player->get_position().y < -10.0f) {
        m_game_state.next_scene_id = 2;
    }

}


void LevelB::render(ShaderProgram* g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < ENEMY_COUNT + PROJECTILE_COUNT; i++)
        m_game_state.enemies[i].render(g_shader_program);

    /*Utility::draw_text(g_shader_program, "time left: " + std::to_string(1), 0.2f, 0.001f,
        glm::vec3(2.0f, -0.5f, 0.0f));*/

}
