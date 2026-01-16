#define SDL_MAIN_HANDLED
// shooter.c
// - Shooter at bottom, move left/right with arrow keys or 'A'/'D'.
// - Shoot with spacebar 
// - 10 targets, each requires 2 hits to die.
// - 50 bullets total. If bullets run out and targets remain, game over.
// - Score: if you kill all targets, bulletsUsed <= 20 -> score 100.
//   Otherwise score decreases linearly to 0 when bulletsUsed == 50.
// Written to be simple and readable using arrays only.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Game Constants
#define SCREEN_WIDTH 1500
#define SCREEN_HEIGHT 750
#define TARGET_COUNT 10
#define MAX_BULLETS 50
#define SHOOTER_SPEED 5
#define BULLET_SPEED 10
#define TARGET_SPEED 2

// Game Structures
typedef struct
{
    float x, y;
    float dx, dy;
    bool active;
    int hits;
} Target;

typedef struct
{
    float x, y;
    bool active;
} Bullet;

// Global arrays
Target targets[TARGET_COUNT];
Bullet bullets[MAX_BULLETS];

// Game state
int shooter_x, shooter_y;
int bullets_used = 0;
int bullets_remaining = MAX_BULLETS;
int score = 0;
int targets_killed = 0;
bool game_running = true;
bool game_won = false;
bool game_lost = false;

// SDL variables
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;

// Function prototypes
void init_game();
void cleanup_game();
void reset_game();
void handle_input();
void update_game();
void render_game();
void spawn_target(int index);
void shoot_bullet();
void check_collisions();
int calculate_score();
void render_text(const char *text, int x, int y, SDL_Color color);
void draw_triangle(int x, int y, int size, SDL_Color color);
void draw_oval(int center_x, int center_y, int width, int height, SDL_Color color);

int main(int argc, char *argv[])
{
    // Tell SDL we're handling main ourselves
    SDL_SetMainReady();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    window = SDL_CreateWindow(
        "Shooter Game - Triangle & Ovals",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!window)
    {
        printf("Window Creation Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer with anti-aliasing for smooth shapes
    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Set renderer drawing quality
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    // Initialize font
    if (TTF_Init() == -1)
    {
        printf("TTF Init Error: %s\n", TTF_GetError());
        // Continue without text
    }
    else
    {
        // Try multiple font locations
        const char *font_paths[] = {
            "arial.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/arialbd.ttf",
            NULL};

        for (int i = 0; font_paths[i] != NULL; i++)
        {
            font = TTF_OpenFont(font_paths[i], 24);
            if (font)
            {
                printf("Loaded font from: %s\n", font_paths[i]);
                break;
            }
        }

        if (!font)
        {
            printf("Could not load any font. Game will run without text.\n");
        }
    }

    // Set up game
    srand(time(NULL));
    init_game();

    // Game loop
    while (game_running)
    {
        handle_input();
        update_game();
        render_game();
        SDL_Delay(16); //60 FPS
    }

    cleanup_game();
    return 0;
}

void init_game()
{
    // Initialize shooter position (center bottom)
    shooter_x = SCREEN_WIDTH / 2;
    shooter_y = SCREEN_HEIGHT - 120;

    // Reset counters
    bullets_used = 0;
    bullets_remaining = MAX_BULLETS;
    score = 0;
    targets_killed = 0;
    game_won = false;
    game_lost = false;

    // Clear bullets
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].active = false;
    }

    // Create targets at random positions
    for (int i = 0; i < TARGET_COUNT; i++)
    {
        spawn_target(i);
    }
}

void cleanup_game()
{
    // Cleanup font
    if (font)
    {
        TTF_CloseFont(font);
    }

    // Cleanup renderer and window
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
        SDL_DestroyWindow(window);
    }

    // Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
}

void reset_game()
{
    init_game();
}

void spawn_target(int index)
{
    // Position targets randomly in upper half of screen
    targets[index].x = 50 + (rand() % (SCREEN_WIDTH - 100));
    targets[index].y = 50 + (rand() % 200);

    // Random movement direction
    targets[index].dx = (rand() % 5) - 2; 
    targets[index].dy = (rand() % 5) - 2;

    targets[index].active = true;
    targets[index].hits = 0;
}

void handle_input()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            game_running = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                game_running = false;
                break;
            case SDLK_LEFT:
            case SDLK_a:
                shooter_x -= SHOOTER_SPEED;
                if (shooter_x < 40)
                    shooter_x = 40;
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                shooter_x += SHOOTER_SPEED;
                if (shooter_x > SCREEN_WIDTH - 40)
                    shooter_x = SCREEN_WIDTH - 40;
                break;
            case SDLK_SPACE:
                if (!game_won && !game_lost)
                {
                    shoot_bullet();
                }
                break;
            case SDLK_r:
                if (game_won || game_lost)
                {
                    reset_game();
                }
                break;
            case SDLK_q:
                game_running = false;
                break;
            }
        }
    }

    // Continuous movement for smooth controls
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A])
    {
        shooter_x -= SHOOTER_SPEED;
        if (shooter_x < 40)
            shooter_x = 40;
    }
    if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D])
    {
        shooter_x += SHOOTER_SPEED;
        if (shooter_x > SCREEN_WIDTH - 40)
            shooter_x = SCREEN_WIDTH - 40;
    }
}

void shoot_bullet()
{
    if (bullets_remaining <= 0)
        return;

    // Find first inactive bullet
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
        {
            bullets[i].x = shooter_x;
            bullets[i].y = shooter_y - 20; // Start from tip of triangle
            bullets[i].active = true;

            bullets_used++;
            bullets_remaining--;
            break;
        }
    }
}

void update_game()
{
    if (game_won || game_lost)
        return;

    // Update bullets - move upward
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            bullets[i].y -= BULLET_SPEED;

            // Remove bullet if off screen
            if (bullets[i].y < 0)
            {
                bullets[i].active = false;
            }
        }
    }

    // Update targets
    for (int i = 0; i < TARGET_COUNT; i++)
    {
        if (targets[i].active)
        {
            // Move target
            targets[i].x += targets[i].dx;
            targets[i].y += targets[i].dy;

            // Bounce off walls
            if (targets[i].x < 30 || targets[i].x > SCREEN_WIDTH - 30)
            {
                targets[i].dx *= -1;
            }
            if (targets[i].y < 30 || targets[i].y > SCREEN_HEIGHT - 150)
            { // Adjusted bottom boundary
                targets[i].dy *= -1;
            }

            // If out of bullets, targets attack (move toward shooter)
            if (bullets_remaining <= 0)
            {
                // Move down faster
                targets[i].y += 3;

                // Move horizontally toward shooter
                if (targets[i].x < shooter_x)
                {
                    targets[i].dx = 2;
                }
                else if (targets[i].x > shooter_x)
                {
                    targets[i].dx = -2;
                }

                // Check if target reached shooter (game over)
                if (targets[i].y > SCREEN_HEIGHT - 130)
                {
                    game_lost = true;
                }
            }
        }
    }

    // Check collisions
    check_collisions();

    // Check win condition
    if (targets_killed >= TARGET_COUNT)
    {
        game_won = true;
        score = calculate_score();
    }

    // Check lose condition (out of bullets)
    if (bullets_remaining <= 0)
    {
        // Check if any bullets are still active
        bool any_bullets_active = false;
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (bullets[i].active)
            {
                any_bullets_active = true;
                break;
            }
        }

        // If no bullets active and not all targets killed, game over
        if (!any_bullets_active && targets_killed < TARGET_COUNT)
        {
            game_lost = true;
        }
    }
}

void check_collisions()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
            continue;

        for (int j = 0; j < TARGET_COUNT; j++)
        {
            if (!targets[j].active)
                continue;

            // Calculate distance between bullet and target center
            float dx = bullets[i].x - targets[j].x;
            float dy = bullets[i].y - targets[j].y;
            float distance = sqrtf(dx * dx + dy * dy);

            // Oval collision detection (approximate with circle for simplicity)
            if (distance < 25)
            { // Collision radius for oval
                bullets[i].active = false;
                targets[j].hits++;

                if (targets[j].hits >= 2)
                {
                    targets[j].active = false;
                    targets_killed++;
                    score += 10; // Base points for killing a target
                }
                break;
            }
        }
    }
}

int calculate_score()
{
    // As per requirements: 20 bullets = 100 score, 50 bullets = 0 score
    if (bullets_used <= 20)
    {
        return 100;
    }

    if (bullets_used >= 50)
    {
        return 0;
    }

    // Linear decrease: 100 points for 20 bullets, 0 points for 50 bullets
    int bullets_over_minimum = bullets_used - 20;
    int max_bullets_over = 50 - 20;

    // Calculate score (100 - (excess bullets * 100 / 30))
    int calculated_score = 100 - (bullets_over_minimum * 100 / 30);

    if (calculated_score < 0)
        calculated_score = 0;
    if (calculated_score > 100)
        calculated_score = 100;

    return calculated_score;
}

void draw_triangle(int x, int y, int size, SDL_Color color)
{
    // Draw a triangle pointing upward
    // Points: top, bottom-left, bottom-right
    SDL_Point points[4] = {
        {x, y - size},        // Top point
        {x - size, y + size}, // Bottom-left
        {x + size, y + size}, // Bottom-right
        {x, y - size}         // Back to top (to close the triangle)
    };

    // Set color and draw filled triangle
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Method 1: Draw filled triangle by drawing multiple lines
    // This is a simple approach for filled triangle
    for (int dy = -size; dy <= size; dy++)
    {
        int width = size - abs(dy);
        SDL_Rect line = {x - width, y + dy, width * 2, 1};
        SDL_RenderFillRect(renderer, &line);
    }

    // Draw triangle outline in a slightly different color for better visibility
    SDL_SetRenderDrawColor(renderer,
                           color.r > 200 ? color.r - 50 : color.r + 50,
                           color.g > 200 ? color.g - 50 : color.g + 50,
                           color.b > 200 ? color.b - 50 : color.b + 50,
                           color.a);
    SDL_RenderDrawLines(renderer, points, 4);
}

void draw_oval(int center_x, int center_y, int width, int height, SDL_Color color)
{
    // Draw a filled oval using multiple rectangles (approximation)
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Simple oval drawing using filled ellipsoid approximation
    for (int dy = -height; dy <= height; dy++)
    {
        // Calculate width at this y-position (ellipse equation)
        float ratio = (float)dy / height;
        int current_width = (int)(width * sqrt(1 - ratio * ratio));

        if (current_width > 0)
        {
            SDL_Rect line = {
                center_x - current_width,
                center_y + dy,
                current_width * 2,
                1};
            SDL_RenderFillRect(renderer, &line);
        }
    }

    // Draw oval outline for better visibility
    SDL_SetRenderDrawColor(renderer,
                           color.r > 200 ? color.r - 50 : color.r + 50,
                           color.g > 200 ? color.g - 50 : color.g + 50,
                           color.b > 200 ? color.b - 50 : color.b + 50,
                           color.a);

    // Draw oval outline using multiple points
    const int segments = 40;
    SDL_Point points[segments + 1];

    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * 3.14159f * i / segments;
        points[i].x = center_x + (int)(width * cos(angle));
        points[i].y = center_y + (int)(height * sin(angle));
    }

    SDL_RenderDrawLines(renderer, points, segments + 1);
}

void render_game()
{
    // Clear screen with dark blue (like space)
    SDL_SetRenderDrawColor(renderer, 10, 10, 40, 255);
    SDL_RenderClear(renderer);

    // Draw a starfield background (only in game area, not in control panel)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
    for (int i = 0; i < 50; i++)
    {
        int star_x = rand() % SCREEN_WIDTH;
        int star_y = rand() % (SCREEN_HEIGHT - 100); // Only in game area
        SDL_RenderDrawPoint(renderer, star_x, star_y);
    }

    // Draw shooter as GREEN TRIANGLE
    SDL_Color shooter_color = {0, 255, 0, 255}; // Green
    draw_triangle(shooter_x, shooter_y, 20, shooter_color);

    // Draw targets as RED OVALS
    for (int i = 0; i < TARGET_COUNT; i++)
    {
        if (targets[i].active)
        {
            // Color: Red for no hits, Orange for one hit
            SDL_Color target_color;
            if (targets[i].hits == 0)
            {
                target_color = (SDL_Color){255, 0, 0, 255}; // Bright red
            }
            else
            {
                target_color = (SDL_Color){255, 140, 0, 255}; // Orange
            }

            // Draw oval target
            draw_oval((int)targets[i].x, (int)targets[i].y, 20, 15, target_color);

            // Draw hit indicator (white circle inside oval)
            if (targets[i].hits > 0)
            {
                SDL_Color hit_color = {255, 255, 255, 255};
                draw_oval((int)targets[i].x, (int)targets[i].y, 8, 6, hit_color);
            }
        }
    }

    // Draw bullets as YELLOW RECTANGLES (laser beams)
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            // Draw laser beam
            SDL_Rect laser_core = {
                (int)bullets[i].x - 2,
                (int)bullets[i].y - 15,
                4,
                30};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &laser_core);
        }
    }

    // Draw UI text if font is available
    if (font)
    {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color green = {0, 255, 0, 255};
        SDL_Color red = {255, 50, 50, 255};
        SDL_Color yellow = {255, 255, 0, 255};
        SDL_Color blue = {100, 150, 255, 255};

        char buffer[100];

        // ===== GAME STATS PANEL (Top Left - Always Visible) =====
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect stats_panel = {5, 5, 250, 90};
        SDL_RenderFillRect(renderer, &stats_panel);

        // Stats title
        render_text("GAME STATUS", 10, 10, blue);

        // Bullets counter
        snprintf(buffer, sizeof(buffer), "BULLETS: %d/%d", bullets_remaining, MAX_BULLETS);
        render_text(buffer, 20, 35, white);

        // Targets counter
        snprintf(buffer, sizeof(buffer), "TARGETS: %d/%d", targets_killed, TARGET_COUNT);
        render_text(buffer, 20, 60, white);

        // Score
        snprintf(buffer, sizeof(buffer), "SCORE: %d", score);
        render_text(buffer, 20, 85, yellow);

        // ===== PERMANENT CONTROLS PANEL (Bottom - Always Visible) =====
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 240);
        SDL_Rect controls_panel = {0, SCREEN_HEIGHT - 100, SCREEN_WIDTH, 100};
        SDL_RenderFillRect(renderer, &controls_panel);

        // Panel border
        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
        SDL_RenderDrawRect(renderer, &controls_panel);

        // Controls title
        render_text("CONTROLS (Always Active)", 20, SCREEN_HEIGHT - 95, green);

        // Controls in organized layout
        render_text("MOVEMENT:", 20, SCREEN_HEIGHT - 65, yellow);
        render_text("A / D   OR   Arrow Keys", 120, SCREEN_HEIGHT - 65, white);

        render_text("SHOOT:", 20, SCREEN_HEIGHT - 35, yellow);
        render_text("SPACEBAR", 120, SCREEN_HEIGHT - 35, white);

        render_text("GAME:", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 65, yellow);
        render_text("R=Restart   Q=Quit   ESC=Exit", SCREEN_WIDTH / 2 + 80, SCREEN_HEIGHT - 65, white);

        render_text("GOAL:", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 35, yellow);
        render_text("Hit targets twice, 50 bullets max", SCREEN_WIDTH / 2 + 80, SCREEN_HEIGHT - 35, white);

        // ===== GAME STATE MESSAGES (Center Screen - Only when game ends) =====
        if (game_won)
        {
            // Victory overlay - semi-transparent over game area only
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay = {
                SCREEN_WIDTH / 2 - 200,
                SCREEN_HEIGHT / 2 - 150, // Centered in game area
                400,
                200};
            SDL_RenderFillRect(renderer, &overlay);

            // Draw victory border
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &overlay);

            render_text("VICTORY!", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 130, green);

            snprintf(buffer, sizeof(buffer), "FINAL SCORE: %d", score);
            render_text(buffer, SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 90, yellow);

            snprintf(buffer, sizeof(buffer), "Bullets Used: %d", bullets_used);
            render_text(buffer, SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 50, white);

            render_text("Press R to play again", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, green);
        }
        else if (game_lost)
        {
            // Game over overlay
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay = {
                SCREEN_WIDTH / 2 - 200,
                SCREEN_HEIGHT / 2 - 150, // Centered in game area
                400,
                200};
            SDL_RenderFillRect(renderer, &overlay);

            // Draw danger border
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &overlay);

            render_text("GAME OVER", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 130, red);
            render_text("Out of bullets!", SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 90, white);
            render_text("Targets are attacking!", SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 - 50, white);
            render_text("Press R to try again", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 10, green);
        }
    }
    else
    {
        // Fallback if no font
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect bullets_text = {10, 10, 150, 20};
        SDL_RenderDrawRect(renderer, &bullets_text);

        SDL_Rect targets_text = {10, 40, 150, 20};
        SDL_RenderDrawRect(renderer, &targets_text);

        // Draw control panel separator line
        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
        SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT - 100, SCREEN_WIDTH, SCREEN_HEIGHT - 100);
    }

    // Update screen
    SDL_RenderPresent(renderer);
}

void render_text(const char *text, int x, int y, SDL_Color color)
{
    if (!font)
        return;

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest_rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}