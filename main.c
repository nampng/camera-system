#include <stdio.h>
#include <SDL.h>

#include "stream.h"
#include "thread.h"
#include "fifo.h"

const int WINDOW_W = 800;
const int WINDOW_H = 480;

static int running = 1;
static int ret = EXIT_SUCCESS;

void processEvent(SDL_Event *event);

int main(int argc, char *argv[])
{
        // init SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
                printf("Failed to initialize SDL: %s\n", SDL_GetError());
                ret = EXIT_FAILURE;
                goto quit;
        }
        SDL_Log("SDL initialized.");

        // create window
        SDL_Window *window = SDL_CreateWindow("Camera System", 
                        SDL_WINDOWPOS_CENTERED, 
                        SDL_WINDOWPOS_CENTERED, 
                        WINDOW_W, 
                        WINDOW_H, 
                        SDL_WINDOW_OPENGL);

        if (window == NULL) {
                SDL_Log("Failed to create window: %s\n", SDL_GetError());
                ret = EXIT_FAILURE;
                goto destroy_window;
        }

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
        if (renderer == NULL) {
                SDL_Log("Failed to create renderer: %s\n", SDL_GetError());
                ret = EXIT_FAILURE;
                goto destroy_renderer;
        }

        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_W, WINDOW_H);
        if (texture == NULL) {
                SDL_Log("Failed to create texture: %s\n", SDL_GetError());
                ret = EXIT_FAILURE;
                goto destroy_texture;
        }

        gst_init(&argc, &argv);
        g_print("gstreamer initialized.\n");
        Stream stream;
        if (initStream(&stream)) {
                goto free_stream;
        }

        ThreadArg threadArg;
        initThreadArg(&threadArg, &stream, WINDOW_W, WINDOW_H, 32, 10);

        pthread_t streamThread;
        pthread_create(&streamThread, NULL, startStream, &threadArg);

        void *rawPixels;
        int rawPitch;

        while (running) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                        processEvent(&event);
                }
                
                SDL_LockTexture(texture, NULL, &rawPixels, &rawPitch); 
                if (rawPixels) {
                        popFifo(&threadArg.buffer, rawPixels);
                }
                SDL_UnlockTexture(texture);
                
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
        }

kill_thread:
        threadArg.running = 0;
        pthread_join(streamThread, NULL);
        freeThreadArg(&threadArg);
free_stream:
        freeStream(&stream);
destroy_texture:
        SDL_DestroyTexture(texture);
destroy_renderer:
        SDL_DestroyRenderer(renderer);
destroy_window:
        SDL_Log("Destroying window...");
        SDL_DestroyWindow(window);
quit:
        SDL_Log("Quitting SDL...");
        SDL_Quit();
        exit(ret);
}

void processEvent(SDL_Event *event) 
{
        switch (event->type) {
        case SDL_KEYDOWN:
                if (event->key.keysym.sym == SDLK_q) {
                        running = 0;
                }
                break;
        case SDL_QUIT:
                running = 0;
                break;
        }
}
