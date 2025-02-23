#include <stdio.h>
#include <SDL.h>

#include "stream.h"
#include "thread.h"
#include "fifo.h"

const int WINDOW_W = 800;
const int WINDOW_H = 480;

static int running = 1;
static int ret = EXIT_SUCCESS;

int inRegion(Sint32 x, Sint32 y, Sint32 tlx, Sint32 tly, Sint32 blx, Sint32 bly);
int setup();
int cleanup();
void processEvent(SDL_Event *event);
void handleLeftButton();
void handleRightButton();

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

        // button texture
        char *buttonPath = NULL;
        SDL_asprintf(&buttonPath, "%sbutton.bmp", SDL_GetBasePath());
        SDL_Surface *buttonSurface = SDL_LoadBMP(buttonPath);
        SDL_free(buttonPath);
        if (buttonSurface == NULL) {
                SDL_Log("Could not load button bitmap: %s\n", SDL_GetError());
                ret = EXIT_FAILURE;
                goto destroy_button_surface;
        }

        SDL_Texture *buttonTexture = SDL_CreateTextureFromSurface(renderer, buttonSurface);
        if (buttonTexture == NULL) {
                SDL_Log("Could not create static texture: %s\n", SDL_GetError());
                ret = EXIT_FAILURE;
                goto destroy_button_texture;
        }

        int button_width = buttonSurface->w;
        int button_height = buttonSurface->h;

        SDL_FreeSurface(buttonSurface);

        // streaming texture for gstreamer side
        SDL_Texture *streamTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_W, WINDOW_H);
        if (streamTexture == NULL) {
                SDL_Log("Failed to create texture: %s\n", SDL_GetError());
                ret = EXIT_FAILURE;
                goto destroy_stream_texture;
        }

        gst_init(&argc, &argv);
        g_print("gstreamer initialized.\n");
        Stream stream;
        if (initStream(&stream)) {
                ret = EXIT_FAILURE;
                goto free_stream;
        }

        ThreadArg threadArg;
        initThreadArg(&threadArg, &stream, WINDOW_W, WINDOW_H, 32, 5);

        pthread_t streamThread;
        pthread_create(&streamThread, NULL, startStream, &threadArg);

        void *rawPixels;
        int rawPitch;
        while (running) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                        processEvent(&event);
                }
                
                SDL_LockTexture(streamTexture, NULL, &rawPixels, &rawPitch); 
                if (rawPixels) {
                        popFifo(&threadArg.buffer, rawPixels);
                }
                SDL_UnlockTexture(streamTexture);
                SDL_RenderCopy(renderer, streamTexture, NULL, NULL);

                SDL_Rect destRect;
                // too dumb to figure out the scaling / translations
                // in a sophisticated way. so... magic numbers.
                // the screen will be always 800x480 anyways since that's the
                // touchscreen's resolution
                destRect.x = (float) (WINDOW_W - button_width) - (10 * 8);
                destRect.y = (float) (WINDOW_H - button_height) / 2;
                destRect.w = (float) button_width * 6;
                destRect.h = (float) button_height * 4;
                SDL_RenderCopy(renderer, buttonTexture, NULL, &destRect);

                destRect.x = (float) (button_width) + (10 * 2);
                destRect.y = (float) (WINDOW_H - button_height) / 2;
                destRect.w = (float) button_width * 6;
                destRect.h = (float) button_height * 4;
                SDL_RenderCopy(renderer, buttonTexture, NULL, &destRect);

                SDL_RenderPresent(renderer);
        }

kill_thread:
        threadArg.running = 0;
        pthread_join(streamThread, NULL);
        freeThreadArg(&threadArg);
free_stream:
        freeStream(&stream);
destroy_stream_texture:
        SDL_DestroyTexture(streamTexture);
destroy_button_texture:
        SDL_DestroyTexture(buttonTexture);
destroy_button_surface:
        SDL_FreeSurface(buttonSurface);
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

int inRegion(Sint32 x, Sint32 y, Sint32 tlx, Sint32 tly, Sint32 blx, Sint32 bly) 
{
        int inX = (tlx <= x && x <= blx);
        int inY = (tly <= y && y <= bly);
        return inX && inY;
}

void handleLeftButton()
{
        printf("Left button\n");
}

void handleRightButton()
{
        printf("Right button\n");
}

void processEvent(SDL_Event *event) 
{
        switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
                //printf("x: %d, y: %d\n", event->button.x, event->button.y);
                // left button:
                // top left corner: x: 35 y: 240
                // bottom right corner: x: 101 y: 277
                if (inRegion(event->button.x, event->button.y, 35, 240, 101, 277)) {
                        handleLeftButton();
                }
                // right button:
                // top left corner: x: 710 y: 240
                // bottom right corner: x: 777 y: 277
                if (inRegion(event->button.x, event->button.y, 710, 240, 777, 277)) {
                        handleRightButton();
                }
                break;
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
