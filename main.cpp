#include <SDL2/SDL.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SIZE = 4;
const int CARD_WIDTH = 150;
const int CARD_HEIGHT = 120;
const int CARD_MARGIN = 20;
const int GRID_OFFSET_X = (WINDOW_WIDTH - (GRID_SIZE * (CARD_WIDTH + CARD_MARGIN) - CARD_MARGIN)) / 2;
const int GRID_OFFSET_Y = (WINDOW_HEIGHT - (GRID_SIZE * (CARD_HEIGHT + CARD_MARGIN) - CARD_MARGIN)) / 2;

enum CardState { HIDDEN, VISIBLE, MATCHED };

struct Card {
    int value;
    CardState state;
    SDL_Rect rect;
};

// 8 colores distintos para los 8 pares
SDL_Color cardColors[8] = {
    {255, 87, 87, 255},    // Rojo
    {87, 255, 87, 255},    // Verde
    {87, 87, 255, 255},    // Azul
    {255, 255, 87, 255},   // Amarillo
    {255, 87, 255, 255},   // Magenta
    {87, 255, 255, 255},   // Cyan
    {255, 165, 0, 255},    // Naranja
    {148, 87, 235, 255}    // Púrpura
};

Card cards[16];
int firstSelected = -1;
int secondSelected = -1;
int attempts = 0;
int matchedPairs = 0;
bool waitingToHide = false;
Uint32 hideTime = 0;

void initCards() {
    int values[16];
    for (int i = 0; i < 8; i++) {
        values[i * 2] = i;
        values[i * 2 + 1] = i;
    }

    // Fisher-Yates shuffle
    for (int i = 15; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(values[i], values[j]);
    }

    for (int i = 0; i < 16; i++) {
        int row = i / GRID_SIZE;
        int col = i % GRID_SIZE;

        cards[i].value = values[i];
        cards[i].state = HIDDEN;
        cards[i].rect = {
            GRID_OFFSET_X + col * (CARD_WIDTH + CARD_MARGIN),
            GRID_OFFSET_Y + row * (CARD_HEIGHT + CARD_MARGIN),
            CARD_WIDTH,
            CARD_HEIGHT
        };
    }
}

int getCardAtPosition(int x, int y) {
    for (int i = 0; i < 16; i++) {
        if (x >= cards[i].rect.x && x < cards[i].rect.x + cards[i].rect.w &&
            y >= cards[i].rect.y && y < cards[i].rect.y + cards[i].rect.h) {
            return i;
        }
    }
    return -1;
}

void drawCard(SDL_Renderer* renderer, int index) {
    Card& card = cards[index];

    if (card.state == HIDDEN) {
        // Carta oculta - gris oscuro
        SDL_SetRenderDrawColor(renderer, 70, 70, 90, 255);
        SDL_RenderFillRect(renderer, &card.rect);

        // Borde
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        SDL_RenderDrawRect(renderer, &card.rect);
    } else {
        // Carta visible o emparejada
        SDL_Color color = cardColors[card.value];

        if (card.state == MATCHED) {
            // Hacer el color más claro para cartas emparejadas
            color.r = (color.r + 255) / 2;
            color.g = (color.g + 255) / 2;
            color.b = (color.b + 255) / 2;
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &card.rect);

        // Dibujar un símbolo simple (círculo interno)
        int centerX = card.rect.x + CARD_WIDTH / 2;
        int centerY = card.rect.y + CARD_HEIGHT / 2;
        int radius = 30;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
                }
            }
        }

        // Borde blanco
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &card.rect);
    }
}

void drawGame(SDL_Renderer* renderer) {
    // Fondo
    SDL_SetRenderDrawColor(renderer, 30, 30, 50, 255);
    SDL_RenderClear(renderer);

    // Dibujar todas las cartas
    for (int i = 0; i < 16; i++) {
        drawCard(renderer, i);
    }

    SDL_RenderPresent(renderer);
}

void handleClick(int x, int y) {
    if (waitingToHide) return;

    int clicked = getCardAtPosition(x, y);
    if (clicked == -1) return;
    if (cards[clicked].state != HIDDEN) return;

    if (firstSelected == -1) {
        firstSelected = clicked;
        cards[clicked].state = VISIBLE;
    } else if (secondSelected == -1 && clicked != firstSelected) {
        secondSelected = clicked;
        cards[clicked].state = VISIBLE;
        attempts++;

        // Verificar coincidencia
        if (cards[firstSelected].value == cards[secondSelected].value) {
            cards[firstSelected].state = MATCHED;
            cards[secondSelected].state = MATCHED;
            matchedPairs++;
            firstSelected = -1;
            secondSelected = -1;
        } else {
            waitingToHide = true;
            hideTime = SDL_GetTicks() + 1000;
        }
    }
}

void update() {
    if (waitingToHide && SDL_GetTicks() >= hideTime) {
        cards[firstSelected].state = HIDDEN;
        cards[secondSelected].state = HIDDEN;
        firstSelected = -1;
        secondSelected = -1;
        waitingToHide = false;
    }
}

void showVictoryScreen(SDL_Renderer* renderer) {
    // Fondo verde para victoria
    SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255);
    SDL_RenderClear(renderer);

    // Dibujar cartas emparejadas (más pequeñas)
    for (int i = 0; i < 16; i++) {
        SDL_Color color = cardColors[cards[i].value];
        color.r = (color.r + 255) / 2;
        color.g = (color.g + 255) / 2;
        color.b = (color.b + 255) / 2;

        SDL_Rect smallRect = {
            cards[i].rect.x + 20,
            cards[i].rect.y + 20,
            CARD_WIDTH - 40,
            CARD_HEIGHT - 40
        };

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderFillRect(renderer, &smallRect);
    }

    // Rectángulo central para mensaje
    SDL_Rect msgRect = {WINDOW_WIDTH/2 - 200, WINDOW_HEIGHT/2 - 60, 400, 120};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &msgRect);

    SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255);
    SDL_Rect innerRect = {msgRect.x + 5, msgRect.y + 5, msgRect.w - 10, msgRect.h - 10};
    SDL_RenderDrawRect(renderer, &innerRect);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(nullptr)));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Juego de Memoria",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    initCards();

    bool running = true;
    bool victory = false;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (victory) {
                    // Reiniciar juego
                    initCards();
                    attempts = 0;
                    matchedPairs = 0;
                    firstSelected = -1;
                    secondSelected = -1;
                    waitingToHide = false;
                    victory = false;
                } else {
                    handleClick(event.button.x, event.button.y);
                }
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                // Reiniciar con tecla R
                initCards();
                attempts = 0;
                matchedPairs = 0;
                firstSelected = -1;
                secondSelected = -1;
                waitingToHide = false;
                victory = false;
            }
        }

        update();

        if (matchedPairs == 8 && !victory) {
            victory = true;
            SDL_SetWindowTitle(window, ("Victoria! Intentos: " + std::to_string(attempts) + " - Click para reiniciar").c_str());
        }

        if (victory) {
            showVictoryScreen(renderer);
        } else {
            drawGame(renderer);
        }

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
