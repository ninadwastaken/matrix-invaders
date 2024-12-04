/**
* Author: Ninad
* Assignment: Matrix Invaders
* Date due: 06-12-2024 ideally, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "Start.h"
#include "LevelB.h"
#include "LevelC.h"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640 * 1.6,
          WINDOW_HEIGHT = 480 * 1.6;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ————— GLOBAL VARIABLES ————— //
Scene *g_current_scene;
LevelA* g_level_a;
LevelB *g_level_b;
LevelC* g_level_c;
Start* g_start_screen;
Start* g_win_screen;
Start* g_lose_screen;

float g_level_time_left[3] = { 5.0f, 25.0f, 5.0f };
int g_curr_level_time_left_index = 0;
float g_curr_level_time_left_value = 15.0f;

Scene* g_levels[6];

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();

}

void initialise();
void process_input();
void update();
void render();
void shutdown();


void initialise()
{
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    auto bgm = Mix_LoadMUS("assets/bgm.wav");
    //Mix_PlayMusic(bgm, -1);
    Mix_VolumeMusic(0);

    g_display_window = SDL_CreateWindow("matrix platformer",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    
    // ————— LEVEL A SETUP ————— //
    g_start_screen = new Start();
    g_level_a = new LevelA();
    g_level_b = new LevelB();
    g_level_c = new LevelC();
    g_win_screen = new Start();
    g_lose_screen = new Start();

    g_levels[0] = g_start_screen;
    g_levels[1] = g_level_a;
    g_levels[2] = g_level_b;
    g_levels[3] = g_level_c;
    g_levels[4] = g_win_screen;
    g_levels[5] = g_lose_screen;


    switch_to_scene(g_levels[0]);

    

    
    
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // ————— KEYSTROKES ————— //
        switch (event.type) {
            // ————— END GAME ————— //
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        // ————— JUMPING ————— //
                        if (g_current_scene->get_state().player->get_collided_bottom())
                        {
                            //g_current_scene->get_state().player->jump();
                            //Mix_PlayChannel(-1,  g_current_scene->get_state().jump_sfx, 0);
                        }
                         break;

                    case SDLK_RETURN:
                        if (g_current_scene == g_start_screen || g_current_scene == g_win_screen || g_current_scene == g_lose_screen) {
                            switch_to_scene(g_level_a);
                            g_curr_level_time_left_index = 0;
                            g_curr_level_time_left_value = g_level_time_left[g_curr_level_time_left_index];
                     
                        }
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    // ————— KEY HOLD ————— //
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])        g_current_scene->get_state().player->move_left();
    else if (key_state[SDL_SCANCODE_RIGHT])  g_current_scene->get_state().player->move_right();
     
    if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
        g_current_scene->get_state().player->normalise_movement();
 
}

void update()
{
    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    g_curr_level_time_left_value -= delta_time;

    if (g_curr_level_time_left_value < 0) {
        if (g_curr_level_time_left_index == 2) {
            g_curr_level_time_left_index = 0;
            g_curr_level_time_left_value = 100000000.0f;
            switch_to_scene(g_win_screen);
        }
        else {
            g_curr_level_time_left_value = g_level_time_left[++g_curr_level_time_left_index];
            switch_to_scene(g_levels[g_curr_level_time_left_index + 1]);
        }
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;

    if (!g_current_scene->m_game_state.player->get_is_active()) {
        switch_to_scene(g_lose_screen);
        g_curr_level_time_left_index = 0;
        g_curr_level_time_left_value = 100000000.0f;
    }
    
    
    // ————— PLAYER CAMERA ————— //
    g_view_matrix = glm::mat4(1.0f);
    
    /*if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
    } else {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
    }*/
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));

}
std::string text_to_write;
void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
    if (g_current_scene == g_levels[4]) {
        text_to_write = "you win wowie";
    }
    else if (g_current_scene == g_levels[5]) {
        text_to_write = "you lose lmao";
    }
    else {
        text_to_write = "time left: " + std::to_string((int)g_curr_level_time_left_value);
    }
    
    if (g_current_scene != g_start_screen) {
        Utility::draw_text(&g_shader_program, text_to_write,
            0.2f, 0.001f,
            glm::vec3(2.0f, -0.5f, 0.0f));
        
    }
    if (g_current_scene != g_start_screen && g_current_scene != g_win_screen && g_current_scene != g_lose_screen) {
        Utility::draw_text(&g_shader_program, "level " + std::to_string(g_curr_level_time_left_index + 1),
            0.2f, 0.001f,
            glm::vec3(2.0f, -1.5f, 0.0f));
    }
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{    
    SDL_Quit();
    
    // ————— DELETING LEVEL A DATA (i.e. map, character, enemies...) ————— //
    delete g_level_a;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();

        if (g_current_scene->m_game_state.next_scene_id >= 0) {
            g_curr_level_time_left_index = g_current_scene->m_game_state.next_scene_id;
            g_curr_level_time_left_value = g_level_time_left[g_curr_level_time_left_index];
            switch_to_scene(g_levels[g_current_scene->m_game_state.next_scene_id]);

        }

        render();
    }

    
    shutdown();
    return 0;
}
