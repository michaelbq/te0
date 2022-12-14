#define SDL_MAIN_HANDLED
#include <SDL2/SDL_keycode.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/param.h>
#include <SDL2/SDL.h>

#include "editor.h"
#include "la.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define BMP_CHAR_MIN 32
#define BMP_CHAR_MAX 126
#define BMP_ROW 6
#define BMP_COL 16

float g_font_scale = 1.0f;

#define FPS 120
#define DELTA_TIME (1.0f/FPS)

Vec2f camera_pos = {0};
Vec2f camera_vel = {0};


teditor editor = {0};

void p_scp(const void *p, int lineno) {
    if(!p) {
        fprintf(stderr, "error: (line:%d) %s\n", lineno, SDL_GetError());
        exit(1);
    }
}
void p_scc(int c, int lineno) {
    if(0 != c) {
        fprintf(stderr, "error: (line:%d) %s\n", lineno, SDL_GetError());
        exit(1);
    }
}

#define M_SCP(p) p_scp(p, __LINE__)
#define M_SCC(c) p_scc(c, __LINE__)


#define OPENGL_RENDERER

#ifdef OPENGL_RENDERER

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <SDL2/SDL_opengl.h>

#include "shader.h"
#include "texture.h"

int g_bmp_pre_char_w = 208/BMP_COL;
int g_bmp_pre_char_h = 156/BMP_ROW;

struct Glyph
{
    Vec2f apos;
    Vec2f chpos;
};

struct Glyph g_glyph[1024];
int g_glyph_num = 0;


void drawText(const char* ptext, int len, int row, int col)
{
    int i = 0;
    for(i = 0; i < len; i++)
    {
        char c = ptext[i] - 32;
        g_glyph[g_glyph_num].apos.x =(col+i);
        g_glyph[g_glyph_num].apos.y = row;
        g_glyph[g_glyph_num].chpos.x = (float)(c%BMP_COL);
        g_glyph[g_glyph_num].chpos.y = (float)(c/BMP_COL);
        g_glyph_num++;
    }
}

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam)
{
    (void) source;
    (void) id;
    (void) length;
    (void) userParam;
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SDL_Window *window = SDL_CreateWindow("Text Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    M_SCP(window);
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        int major;
        int minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
        printf("GL version %d.%d\n", major, minor);
    }
    SDL_GLContext *pglcontext = SDL_GL_CreateContext(window);
    M_SCP(pglcontext);

    if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        fprintf(stderr, "gladLoadGLLoader error\n");
        exit(1);
    }

//    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//    M_SCP(renderer);

    glfwSwapInterval(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //if (GLAD_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
    } 

    unsigned int shaderProgram;
    if(!CreateProgram("vertex.shader", "fragment.shader", &shaderProgram))
    {
        fprintf(stderr, "CreateProgram failed\n");
        exit(1);
    }
    glUseProgram(shaderProgram);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_glyph), g_glyph, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct Glyph), (void*)offsetof(struct Glyph, apos));
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Glyph), (void*)offsetof(struct Glyph, chpos));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    unsigned int texture0;
    if(!LoadTexture("ascii.bmp", &texture0))
    {
        fprintf(stderr, "LoadTexture failed\n");
        exit(1);
    }
    glActiveTexture(GL_TEXTURE0);

    unsigned int scale_uniform = glGetUniformLocation(shaderProgram, "scale");
    glUniform1f(scale_uniform, g_font_scale);

    g_glyph_num = 0;
    const char text[] = "hello world!";
    drawText(text, strlen(text), 0, 0);
    const char text2[] = "abcdefg";
    drawText(text2, strlen(text2), 1, 0);

    glBufferSubData(GL_ARRAY_BUFFER, 0, g_glyph_num * sizeof(struct Glyph), g_glyph);

    bool bquit = false;
    while(!bquit)
    {
        SDL_Event event = {0};
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT: {
                bquit = true;
            }
            break;
            }
        }

        glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture0);
        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g_glyph_num);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &vao);

    return 0;
}

#else

int g_bmp_pre_char_w = 0;
int g_bmp_pre_char_h = 0;

void drawCursor(SDL_Renderer *renderer, Vec2f pos) {
    SDL_Rect rect = {.x = (int)floorf(pos.x), .y = (int)floorf(pos.y), .w = g_bmp_pre_char_w * g_font_scale, .h = g_bmp_pre_char_h * g_font_scale};
    M_SCC(SDL_SetRenderDrawColor(renderer, 155, 155, 155, 255));
    M_SCC(SDL_RenderFillRect(renderer, &rect));
}

void drawChar(SDL_Renderer* renderer, SDL_Texture *texture, Vec2f pos, char c) {
    if(c >= BMP_CHAR_MIN && c <= BMP_CHAR_MAX) {
        size_t i = c - BMP_CHAR_MIN;
        size_t brow = i / BMP_COL;
        size_t bcol = i % BMP_COL;
        SDL_Rect srcrect = {.x = bcol * g_bmp_pre_char_w, .y = brow * g_bmp_pre_char_h, .w = g_bmp_pre_char_w, .h = g_bmp_pre_char_h};
        SDL_Rect dstrect = {.x = (int)floorf(pos.x), .y = (int)floorf(pos.y), .w = g_bmp_pre_char_w * g_font_scale, .h = g_bmp_pre_char_h * g_font_scale};
        M_SCC(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
    } else {
        fprintf(stderr, "char %d is out of range\n", c);
    }
}

void drawText(SDL_Renderer *renderer, SDL_Texture *texture, Vec2f pos, const char *text, size_t len) {
    for(size_t i = 0; i < len; i++) {
        drawChar(renderer, texture, pos, text[i]);
        pos.x += g_bmp_pre_char_w * g_font_scale;
    }
}

int main(int argc, char *argv[]) 
{

    // editor_init(&editor);
    if(argc > 1)
    {
        editor_load_from_file(&editor, argv[1]);
    }
    else
    {
        editor_create_first_new_line(&editor);
    }

    M_SCC(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window = SDL_CreateWindow("Text Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    M_SCP(window);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    M_SCP(renderer);

    SDL_Surface *surface_chars = SDL_LoadBMP("ascii.bmp");
    M_SCP(surface_chars);
    g_bmp_pre_char_w = surface_chars->clip_rect.w / BMP_COL;
    g_bmp_pre_char_h = surface_chars->clip_rect.h / BMP_ROW;
    SDL_Texture *texture_chars = SDL_CreateTextureFromSurface(renderer, surface_chars);
    M_SCP(texture_chars);
    SDL_FreeSurface(surface_chars);

    bool bquit = false;
    SDL_Event event;
    while(!bquit) {
        const Uint32 start = SDL_GetTicks();

        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderClear(renderer);

        SDL_PollEvent(&event);
        switch(event.type) {
            case SDL_QUIT: {
                bquit = true;
            }
            break;
            case SDL_KEYDOWN: {
                switch(event.key.keysym.sym) {
                    case SDLK_BACKSPACE: {
                        editor_delete_back_text(&editor, 1);
                    }
                    break;
                    case SDLK_DELETE:
                    {
                        editor_delete_after_text(&editor, 1);
                    }
                    break;
                    case SDLK_LEFT: {
                        if(editor_cursorCanLeft(&editor)) editor.cursor_col--;
                    }
                    break;
                    case SDLK_RIGHT: {
                        if(editor_cursorCanRight(&editor)) editor.cursor_col++;
                    }
                    break;
                    case SDLK_RETURN: {
                        editor_insert_newline(&editor);
                    }
                    break;
                    case SDLK_UP:
                    {
                        if(editor.cursor_row > 0) editor.cursor_row--;
                    }
                    break;
                    case SDLK_DOWN:
                    {
                        if(editor.cursor_row + 1< editor.size) editor.cursor_row++;
                    }
                    break;
                    case SDLK_F1:
                    {
                        editor_save_to_file(&editor, "output.txt");
                    }
                    break;
                    default: break;
                }
            }
            break;
            case SDL_TEXTINPUT: {
                size_t len = strlen(event.text.text);
                editor_insert_text(&editor, event.text.text, len);
            }
            break;
            default: {
            }
            break;
        }
        if(bquit) continue;

        {
            const Vec2f cursor_pos = vec2f((float)editor.cursor_col * g_bmp_pre_char_w * g_font_scale, 
                                           (float)editor.cursor_row * g_bmp_pre_char_h * g_font_scale);
            camera_vel = vec2f_mul(vec2f_sub(cursor_pos, camera_pos), vec2fs(2.0f));
            camera_pos = vec2f_add(camera_pos, vec2f_mul(camera_vel, vec2fs(DELTA_TIME)));
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
        for(size_t i = 0; i < editor.size; i++) {
            const Vec2f pos = vec2f_add(vec2f_sub(vec2f_mul(vec2f(0,i), vec2f(g_bmp_pre_char_w * g_font_scale, g_bmp_pre_char_h * g_font_scale)), camera_pos), vec2f(400, 300));
            drawText(renderer,texture_chars, pos, editor.lines[i].buf, editor.lines[i].len);
        }
        {
            const Vec2f cursor_pos = vec2f_add(vec2f((float)editor.cursor_col * g_bmp_pre_char_w * g_font_scale, 
                                           (float)editor.cursor_row * g_bmp_pre_char_h * g_font_scale),
                                       vec2f(400, 300));
            drawCursor(renderer, vec2f_sub(cursor_pos, camera_pos));
        }
        SDL_RenderPresent(renderer);

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 detla_time = 1000 / FPS;
        if(duration < detla_time)
        {
            SDL_Delay(detla_time - duration);
        }
    }

    SDL_DestroyTexture(texture_chars);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    editor_deinit(&editor);

    return 0;
}
#endif

