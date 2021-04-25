#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

// Width and height should probably always stay
// the same in the game's current state.
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800
// I recognize that this is not the most versatile way to do
// this, but here I set this as 50 so that on a 800x800 screen
// it creates a grid of 16x16
#define GRID_BOX_SIZE 50 // 800 / 16 = 50

typedef struct _SnakeNode {
    int x, y;       // Coordinates are from the top left
    int direction;  // This could probably be easily removed
    struct _SnakeNode *next;
} SnakeNode;

typedef struct {
    int x, y;
} SnakeFood;

SDL_Window *init() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window) {
        printf("Failed to open window: %s\n", SDL_GetError());
    }

    SDL_Surface *screenSurface = SDL_GetWindowSurface(window);

    if (!screenSurface) {
        printf("Failed to get screen surface: %s\n", SDL_GetError());
    }

    // Seed the random number generator
    srand(time(NULL));

    return window;
}

void render_food(SDL_Renderer *renderer, SnakeFood *food, SDL_Texture *foodTexture) {
    SDL_Rect textureRenderRect;
    textureRenderRect.x = 2 + food->x * GRID_BOX_SIZE;
    textureRenderRect.y = 2 + food->y * GRID_BOX_SIZE;
    textureRenderRect.w = GRID_BOX_SIZE - 4;
    textureRenderRect.h = GRID_BOX_SIZE - 4;
    SDL_RenderCopy(renderer, foodTexture, NULL, &textureRenderRect);
}

void render_snake(SDL_Renderer *renderer, SnakeNode *head, SDL_Texture *snakeTexture) {
    // Setup reusable rect for passing to the renderer.
    SDL_Rect textureRenderRect;
    textureRenderRect.x = 2;
    textureRenderRect.y = 2;
    textureRenderRect.w = GRID_BOX_SIZE - 4;
    textureRenderRect.h = GRID_BOX_SIZE - 4;

    while (head) {
        textureRenderRect.x += GRID_BOX_SIZE * head->x;
        textureRenderRect.y += GRID_BOX_SIZE * head->y;

        SDL_RenderCopy(renderer, snakeTexture, NULL, &textureRenderRect);

        textureRenderRect.x -= GRID_BOX_SIZE * head->x;
        textureRenderRect.y -= GRID_BOX_SIZE * head->y;
        
        head = head->next;
    }
}

void generate_food_location(SnakeFood *food, SnakeNode *head) {
    int x = 16 * (rand() / (float) RAND_MAX);
    int y = 16 * (rand() / (float) RAND_MAX);

    SnakeNode *curr = head;
    while (curr) {
        if (x == curr->x && y == curr->y) {
            // Probably not the most efficient way of
            // doing this but it sounded cool to do 
            // it recursively.
            generate_food_location(food, head);
            return; // Leave the rest of the function to the next call
        }

        curr = curr->next;
    }

    food->x = x;
    food->y = y;
}

int move_snake(SnakeNode *head, SnakeFood *food) {
    int originalX = head->x, nextX = 0;
    int originalY = head->y, nextY = 0;
    int hitFood = 0;

    switch (head->direction) {
        case 0: // Right
        head->x++;
        break;
        case 1: // Down
        head->y++;
        break;
        case 2: // Left
        head->x--;
        break;
        case 3: // Up
        head->y--;
        break;
    }

    // Restrict movement
    if (head->x >= 16) {
        head->x = 15;
        return 1;
    }

    if (head->x < 0) {
        head->x = 0;
        return 1;
    }

    if (head->y >= 16) {
        head->y = 15;
        return 1;
    }

    if (head->y < 0) {
        head->y = 0;
        return 1;
    }

    if (head->x == food->x && head->y == food->y) {
        hitFood = 1;

        // The food can be moved here because
        // the new SnakeNode will be spawned
        // in place of the current tail so there
        // will be no overlap between the food and
        // the snake even after adding on to the tail.
        generate_food_location(food, head);
    }
    
    SnakeNode *curr = head;
    while (curr->next) {
        // Save the original position of head->next
        nextX = curr->next->x;
        nextY = curr->next->y;

        // Move head->next to the original location of head
        curr->next->x = originalX;
        curr->next->y = originalY;

        // Save the original location of head->next for the next iteration if necessary
        originalX = nextX;
        originalY = nextY;

        // Check for a collision between head and tail
        if (head->x == curr->next->x && head->y == curr->next->y) {
            return 1;
        }

        // Advance through the linked list
        curr = curr->next;
    }

    if (hitFood) {
        curr->next = malloc(sizeof(SnakeNode));
        curr->next->x = head->x;
        curr->next->y = head->y;
        curr->next->next = NULL;
    }

    return 0;
}

void reset_game(SnakeNode *head, SnakeFood *food) {
    SnakeNode *temp = head->next, *preTemp = head->next;
    while (temp) {
        temp = preTemp->next;
        free(preTemp);
        preTemp = temp;
    }

    head->x = 1;
    head->y = 1;
    head->direction = 0;
    head->next = NULL;

    generate_food_location(food, head);
}

int main(char* argc, char** argv) {
    SDL_Window *window = init();

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Surface *greenSurface = IMG_Load("images/green.png");
    SDL_Texture *greenTexture = SDL_CreateTextureFromSurface(renderer, greenSurface);
    SDL_FreeSurface(greenSurface);

    SDL_Surface *redSurface = IMG_Load("images/red.png");
    SDL_Texture *redTexture = SDL_CreateTextureFromSurface(renderer, redSurface);
    SDL_FreeSurface(redSurface);

    SnakeNode head;
    head.x = 1;
    head.y = 1;
    head.direction = 0; // right 0, down 1, left 2, up 3
    head.next = NULL;

    SnakeFood food;
    generate_food_location(&food, &head);

    int running = 1, tickCount = 0;
    SDL_Event event;
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type = SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                    if (!(head.next && head.next->x == head.x && head.next->y < head.y)) {
                        head.direction = 3;
                    }
                    break;
                    case SDLK_DOWN:
                    if (!(head.next && head.next->x == head.x && head.next->y > head.y)) {
                        head.direction = 1;
                    }
                    break;
                    case SDLK_LEFT:
                    if (!(head.next && head.next->x < head.x && head.next->y == head.y)) {
                        head.direction = 2;
                    }
                    break;
                    case SDLK_RIGHT:
                    if (!(head.next && head.next->x > head.x && head.next->y == head.y)) {
                        head.direction = 0;
                    }
                    break;
                }
            }
        }

        // Render to the screen
        SDL_RenderClear(renderer);

        render_food(renderer, &food, redTexture);
        render_snake(renderer, &head, greenTexture);

        // Update snake
        if (SDL_GetTicks() - tickCount >= 500) {
            tickCount = SDL_GetTicks();
            if (move_snake(&head, &food)) {
                reset_game(&head, &food); // Judging by the arguments move and reset could be combined... This may be more readable though.
            }
        }

        SDL_RenderPresent(renderer);
    }

    // Free the snake
    SnakeNode *temp = head.next, *preTemp = head.next;
    while (temp) {
        temp = preTemp->next;
        free(preTemp);
        preTemp = temp;
    }

    SDL_DestroyTexture(greenTexture);
    SDL_DestroyTexture(redTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}