#include <stdio.h>
#include <SDL.h>

#include "stream.h"
#include "thread.h"
#include "fifo.h"

const int WINDOW_W = 480;
const int WINDOW_H = 800;

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
        #ifdef ON_TARGET
                printf("TARGET build.\n");
        #else
                printf("HOST build.\n");
        #endif

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
                goto destroy_renderer;
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
                destRect.x = 0.0;
                destRect.y = 0.0;
                destRect.w = (float) button_width;
                destRect.h = (float) button_height;
                SDL_RenderCopy(renderer, buttonTexture, NULL, &destRect);

                destRect.x = (float) 0.0;
                destRect.y = (float) (WINDOW_H - button_height);
                destRect.w = (float) button_width;
                destRect.h = (float) button_height;
                SDL_RenderCopyEx(renderer, buttonTexture, NULL, &destRect, 0, NULL, SDL_FLIP_VERTICAL);

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
        case SDL_FINGERDOWN:
                Sint32 x = (Sint32) event->tfinger.x * WINDOW_W;
                Sint32 y = (Sint32) event->tfinger.y * WINDOW_H; 
                printf("touch: x: %d, y: %d\n", event->tfinger.x, event->tfinger.y);
                printf("processed: x: %d, y: %d\n", x, y);

                if (inRegion(x, y, 0, 0, 100, 200)) {
                        handleLeftButton();
                }
                if (inRegion(x, y, 0, 600, 100, 800)) {
                        handleRightButton();
                }
                break;
        case SDL_MOUSEBUTTONDOWN:
                // left button:
                // top left corner: x: 0 y: 0
                // bottom right corner: x: 100 y: 200
                if (inRegion(event->button.x, event->button.y, 0, 0, 100, 200)) {
                        handleLeftButton();
                }
                // right button:
                // top left corner: x: 0 y: 600
                // bottom right corner: x: 100 y: 800
                if (inRegion(event->button.x, event->button.y, 0, 600, 100, 800)) {
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
