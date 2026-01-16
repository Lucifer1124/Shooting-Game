// shooter.c
#include <stdio.h>
#include <conio.h>   
#include <windows.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 20
#define TARGETS_COUNT 10
#define MAX_BULLETS 50


int target_x[TARGETS_COUNT];
int target_y[TARGETS_COUNT];
int target_hits[TARGETS_COUNT]; 

// Shooter state
int shooter_x;
int shooter_y;

// Bullet state single bullet at a time
bool bullet_active;
int bullet_x, bullet_y;
int bullets_fired;


int clamp(int v, int lo, int hi){
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

//initialize / reset the game arrays & vars
void init_game(){
    //place targets along row 2 spaced evenly
    int spacing = WIDTH / TARGETS_COUNT;
    for (int i = 0; i < TARGETS_COUNT; ++i){
        target_x[i] = clamp(i * spacing + spacing / 2, 1, WIDTH - 2);
        target_y[i] = 2;
        target_hits[i] = 0;
    }
    shooter_x = WIDTH / 2;
    shooter_y = HEIGHT - 2;
    bullet_active = false;
    bullets_fired = 0;
}

// Draw the current frame to the console
void draw_frame() {
    // Clear screen
    system("cls");

    //frame buffer
    char frame[HEIGHT + 1][WIDTH + 1];
    for (int r = 0; r < HEIGHT; ++r) {
        for (int c = 0; c < WIDTH; ++c)
            frame[r][c] = ' ';
        frame[r][WIDTH] = '\0';
    }

    // Draw targets
    for (int i = 0; i < TARGETS_COUNT; ++i){
        if (target_hits[i] < 2){
            // display remaining hits as 2 or 1
            char ch = (target_hits[i] == 0) ? 'O' : '1';
            // if one hit show 1 or 0
            frame[target_y[i]][target_x[i]] = ch;
        }
    }
    // Draw bullet if active
    if (bullet_active){
        if (bullet_y >= 0 && bullet_y < HEIGHT)
            frame[bullet_y][bullet_x] = '|';
    }
    frame[shooter_y][shooter_x] = 'A';

    for (int c = 0; c < WIDTH + 2; ++c)
        putchar('-');
    putchar('\n');

    for (int r = 0; r < HEIGHT; ++r){
        putchar('|');
        printf("%s", frame[r]);
        putchar('|');
        putchar('\n');
    }
    // Bottom border
    for (int c = 0; c < WIDTH + 2; ++c)
        putchar('-');
    putchar('\n');
    // hood
    int remainingTargets = 0;
    for (int i = 0; i < TARGETS_COUNT; ++i)
        if (target_hits[i] < 2)
            remainingTargets++;
    printf("Bullets fired: %d / %d\tRemaining Targets: %d\n", bullets_fired, MAX_BULLETS, remainingTargets);
    printf("Controls: Left/Right arrows or A/D to move | Space or W to shoot | Q to quit\n");
}
// Move bullet up, check for collisions
void update_bullet(){
    if (!bullet_active)
        return;
    // move up
    bullet_y--;

    // check collision with any target
    for (int i = 0; i < TARGETS_COUNT; ++i) {
        if (target_hits[i] < 2 && bullet_x == target_x[i] && bullet_y == target_y[i]) {
            target_hits[i]++; // increase hit count
            bullet_active = false; // bullet consumed
            return;
        }
    }
    // if bullet goes off top, deactivate
    if (bullet_y < 0)
        bullet_active = false;
}

// Returns number of targets still alive
int targets_remaining(){
    int count = 0;
    for (int i = 0; i < TARGETS_COUNT; ++i)
        if (target_hits[i] < 2)
            count++;
    return count;
}

// Score calculation more bullets = low score 
int compute_score(int bulletsUsed) {
    if (bulletsUsed <= 20)
        return 100;
    if (bulletsUsed >= 50)
        return 0;

    double ratio = (50.0 - bulletsUsed) / 30.0; // 30 = 50 - 20
    int score = (int)(ratio * 100.0 + 0.5);
    if (score < 0)
        score = 0;
    return score;
}

// Main game loop -> returns 1 if player won, 0 if lost
int play_game(int *outBulletsUsed){
    init_game();
    int ch;
    bool quit = false;

    // simple frame timing
    const int frame_ms = 60;

    while (!quit)
    {
        draw_frame();
        // input handling
        if (_kbhit())
        {
            ch = _getch();
            // check arrow keys sequence
            if (ch == 0 || ch == 224){
                int ch2 = _getch();
                if (ch2 == 75)
                { // left arrow
                    shooter_x = clamp(shooter_x - 1, 1, WIDTH - 2);
                }
                else if (ch2 == 77)
                { // right arrow
                    shooter_x = clamp(shooter_x + 1, 1, WIDTH - 2);
                }
            } else{
                // normal keys
                if (ch == 'a' || ch == 'A')
                {
                    shooter_x = clamp(shooter_x - 1, 1, WIDTH - 2);
                }
                else if (ch == 'd' || ch == 'D')
                {
                    shooter_x = clamp(shooter_x + 1, 1, WIDTH - 2);
                }
                else if (ch == 'w' || ch == 'W' || ch == ' ')
                {
                    // shoot if bullet not active and bullets remain
                    if (!bullet_active && bullets_fired < MAX_BULLETS)
                    {
                        bullet_active = true;
                        bullet_x = shooter_x;
                        bullet_y = shooter_y - 1;
                        bullets_fired++;
                    }
                }
                else if (ch == 'q' || ch == 'Q')
                {
                    quit = true;
                }
            }
        }
        // update bullet
        update_bullet();
        // check win
        if (targets_remaining() == 0) {
            if (outBulletsUsed)
                *outBulletsUsed = bullets_fired;
            return 1; // win
        }
        // bullets finished = lose
        if (bullets_fired >= MAX_BULLETS && !bullet_active){
            // Out of bullets and no bullet in flight = loose
            if (outBulletsUsed)
                *outBulletsUsed = bullets_fired;
            return 0;
        }

        Sleep(frame_ms);
    }
    //quit = lose
    if (outBulletsUsed)
        *outBulletsUsed = bullets_fired;
    return 0;
}

int main() {
    srand((unsigned)time(NULL));
    printf("Simple Shooter Game (Console)\n");
    printf("Press any key to start...\n");
    _getch();

    bool playAgain = true;
    while (playAgain) {
        int bulletsUsed = 0;
        int result = play_game(&bulletsUsed);
        system("cls");
        if (result){
            int score = compute_score(bulletsUsed);
            printf("CONGRATS — YOU KILLED ALL TARGETS!\n");
            printf("Bullets used: %d\n", bulletsUsed);
            printf("Score: %d / 100\n", score);
        } else {
            printf("GAME OVER — You ran out of bullets or quit.\n");
            printf("Bullets used: %d\n", bulletsUsed);
            int remaining = targets_remaining();
            printf("Targets remaining: %d\n", remaining);
        }
        printf("\nPlay again? (Y/N): ");
        int c = _getch();
        if (c == 'y' || c == 'Y') {
            playAgain = true;
        }
        else {
            playAgain = false;
        }
    }
    printf("\nThanks for playing. Exiting...\n");
    return 0;
}
