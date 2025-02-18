#include <SDL.h>
#include <SDL_mixer.h>
#include <emscripten/emscripten.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 60
#define BALL_SIZE 10

// Variables globales para SDL
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Variables de juego
int leftPaddleY = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
int rightPaddleY = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
int ballX = SCREEN_WIDTH / 2 - BALL_SIZE/2;
int ballY = SCREEN_HEIGHT / 2 - BALL_SIZE/2;
// Pelota más lenta
int ballVelX = -2;
int ballVelY = 2;
// Paleta del jugador se moverá más rápido
const int paddleSpeed = 20;

bool running = true;

// Variable global para el sonido de impacto
Mix_Chunk *hitSound = NULL;

void game_loop() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            running = false;
        } else if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_UP) {
                rightPaddleY -= paddleSpeed;
                if(rightPaddleY < 0) rightPaddleY = 0;
            } else if(event.key.keysym.sym == SDLK_DOWN) {
                rightPaddleY += paddleSpeed;
                if(rightPaddleY > SCREEN_HEIGHT - PADDLE_HEIGHT)
                    rightPaddleY = SCREEN_HEIGHT - PADDLE_HEIGHT;
            }
        }
    }

    // Control simple de la paleta izquierda (IA): sigue la posición de la bola
    if(ballY < leftPaddleY) {
        leftPaddleY -= 3;
        if(leftPaddleY < 0) leftPaddleY = 0;
    } else if(ballY > leftPaddleY + PADDLE_HEIGHT) {
        leftPaddleY += 3;
        if(leftPaddleY > SCREEN_HEIGHT - PADDLE_HEIGHT)
            leftPaddleY = SCREEN_HEIGHT - PADDLE_HEIGHT;
    }

    // Mover la pelota
    ballX += ballVelX;
    ballY += ballVelY;

    // Rebotar en los bordes superior e inferior
    if(ballY <= 0 || ballY >= SCREEN_HEIGHT - BALL_SIZE)
        ballVelY = -ballVelY;

    // Colisión con la paleta derecha (jugador)
    if(ballX + BALL_SIZE >= SCREEN_WIDTH - PADDLE_WIDTH) {
        if(ballY + BALL_SIZE >= rightPaddleY && ballY <= rightPaddleY + PADDLE_HEIGHT) {
            ballVelX = -ballVelX;
            ballX = SCREEN_WIDTH - PADDLE_WIDTH - BALL_SIZE;
            // Reproducir sonido de impacto
            if(hitSound) {
                Mix_PlayChannel(-1, hitSound, 0);
            }
        } else {
            // Reiniciar la pelota si falla la colisión
            ballX = SCREEN_WIDTH / 2 - BALL_SIZE/2;
            ballY = SCREEN_HEIGHT / 2 - BALL_SIZE/2;
        }
    }

    // Colisión con la paleta izquierda (IA)
    if(ballX <= PADDLE_WIDTH) {
        if(ballY + BALL_SIZE >= leftPaddleY && ballY <= leftPaddleY + PADDLE_HEIGHT) {
            ballVelX = -ballVelX;
            ballX = PADDLE_WIDTH;
            // Reproducir sonido de impacto
            if(hitSound) {
                Mix_PlayChannel(-1, hitSound, 0);
            }
        } else {
            // Reiniciar la pelota
            ballX = SCREEN_WIDTH / 2 - BALL_SIZE/2;
            ballY = SCREEN_HEIGHT / 2 - BALL_SIZE/2;
        }
    }

    // Renderizado
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Dibujar la pelota (blanca)
    SDL_Rect ballRect = { ballX, ballY, BALL_SIZE, BALL_SIZE };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ballRect);

    // Dibujar la paleta izquierda
    SDL_Rect leftPaddle = { 0, leftPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_RenderFillRect(renderer, &leftPaddle);

    // Dibujar la paleta derecha
    SDL_Rect rightPaddle = { SCREEN_WIDTH - PADDLE_WIDTH, rightPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_RenderFillRect(renderer, &rightPaddle);

    SDL_RenderPresent(renderer);

    // Si se cierra la ventana, finaliza el bucle
    if(!running) {
        emscripten_cancel_main_loop();
        Mix_FreeChunk(hitSound);
        Mix_CloseAudio();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

int main(int argc, char* argv[]) {
    // Inicializar SDL (video y audio)
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    
    window = SDL_CreateWindow("Pong Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if(!window) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) {
        printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Inicializar SDL_mixer
    if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }
    
    // Cargar el sonido de impacto (hit.wav)
    hitSound = Mix_LoadWAV("hit.wav");
    if(hitSound == NULL) {
        printf("Failed to load hit sound! SDL_mixer Error: %s\n", Mix_GetError());
        // Puedes continuar sin sonido o salir
    }
    
    // Configurar el bucle principal de Emscripten
    emscripten_set_main_loop(game_loop, 0, 1);
    
    return 0;
}
