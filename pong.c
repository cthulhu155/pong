#include <SDL.h>
#include <emscripten/emscripten.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 60
#define BALL_SIZE 10

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int leftPaddleY = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
int rightPaddleY = (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2;
int ballX = SCREEN_WIDTH / 2 - BALL_SIZE/2;
int ballY = SCREEN_HEIGHT / 2 - BALL_SIZE/2;
int ballVelX = -4;
int ballVelY = 4;

bool running = true;

void game_loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            running = false;
        } else if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_UP) {
                rightPaddleY -= 10;
                if(rightPaddleY < 0) rightPaddleY = 0;
            } else if(event.key.keysym.sym == SDLK_DOWN) {
                rightPaddleY += 10;
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

    // Mover la bola
    ballX += ballVelX;
    ballY += ballVelY;

    // Rebotar la bola en el tope y fondo
    if(ballY <= 0 || ballY >= SCREEN_HEIGHT - BALL_SIZE)
        ballVelY = -ballVelY;

    // Colisión con la paleta derecha
    if(ballX + BALL_SIZE >= SCREEN_WIDTH - PADDLE_WIDTH) {
        if(ballY + BALL_SIZE >= rightPaddleY && ballY <= rightPaddleY + PADDLE_HEIGHT) {
            ballVelX = -ballVelX;
            ballX = SCREEN_WIDTH - PADDLE_WIDTH - BALL_SIZE;
        } else {
            // Si falla, reinicia la bola
            ballX = SCREEN_WIDTH / 2 - BALL_SIZE/2;
            ballY = SCREEN_HEIGHT / 2 - BALL_SIZE/2;
        }
    }

    // Colisión con la paleta izquierda
    if(ballX <= PADDLE_WIDTH) {
        if(ballY + BALL_SIZE >= leftPaddleY && ballY <= leftPaddleY + PADDLE_HEIGHT) {
            ballVelX = -ballVelX;
            ballX = PADDLE_WIDTH;
        } else {
            // Si falla, reinicia la bola
            ballX = SCREEN_WIDTH / 2 - BALL_SIZE/2;
            ballY = SCREEN_HEIGHT / 2 - BALL_SIZE/2;
        }
    }

    // Render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Dibuja la bola (blanca)
    SDL_Rect ballRect = { ballX, ballY, BALL_SIZE, BALL_SIZE };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ballRect);

    // Dibuja la paleta izquierda
    SDL_Rect leftPaddle = { 0, leftPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_RenderFillRect(renderer, &leftPaddle);

    // Dibuja la paleta derecha
    SDL_Rect rightPaddle = { SCREEN_WIDTH - PADDLE_WIDTH, rightPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_RenderFillRect(renderer, &rightPaddle);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    
    window = SDL_CreateWindow("Pong Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if(!window) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) {
        printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
        return 1;
    }
    
    // Configura el bucle principal de Emscripten
    emscripten_set_main_loop(game_loop, 0, 1);
    return 0;
}
