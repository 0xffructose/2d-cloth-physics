#include <stdio.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

#define WIDTH 640
#define HEIGHT 640

#define GRAVITY 980

#define VERTICAL_PARTICLE_COUNT 15
#define HORIZONTAL_PARTICLE_COUNT 15

#define PARTICLE_REST_LENGTH 30
#define MOUSE_RADIUS 50.0F

// Vector2 WIND = { 5 , 0 };
// const float WIND_SPEED = 1000;

typedef struct Particle {
    bool pinned;

    Vector2 position;
    Vector2 previous_position;

    Vector2 acceleration;
} Particle;

Particle PARTICLES[VERTICAL_PARTICLE_COUNT * HORIZONTAL_PARTICLE_COUNT];

bool DEBUG_RENDERER = true;

void DebugCloth() {
    for (int i = 0; i < VERTICAL_PARTICLE_COUNT * HORIZONTAL_PARTICLE_COUNT; ++i) {
        DrawCircle(PARTICLES[i].position.x , PARTICLES[i].position.y , 5 , PARTICLES[i].pinned ? RED : WHITE);
    }
}

void DrawCloth() {
    for (int y = 0; y < VERTICAL_PARTICLE_COUNT; ++y) {
        for (int x = 0; x < HORIZONTAL_PARTICLE_COUNT - 1; ++x) {
            DrawLine(PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.x , PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.y , PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 1)].position.x , PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 1)].position.y , WHITE); 
        }
    }

    for (int x = 0; x < HORIZONTAL_PARTICLE_COUNT; ++x) {
        for (int y = 0; y < VERTICAL_PARTICLE_COUNT - 1; ++y) {
            DrawLine(PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.x , PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.y , PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + x].position.x , PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + x].position.y , WHITE);
        }
    }
}

void UpdateMouseEffect() {

    for (int i = 0; i < VERTICAL_PARTICLE_COUNT * HORIZONTAL_PARTICLE_COUNT; ++i) {
        float distance = Vector2Distance(PARTICLES[i].position , GetMousePosition());
        if (distance > MOUSE_RADIUS || PARTICLES[i].pinned) continue;

        Vector2 diff = Vector2Subtract(PARTICLES[i].position , GetMousePosition());
        Vector2 n = Vector2Scale(diff , 1.0F/distance);

        Vector2 target = Vector2Add(GetMousePosition() , Vector2Scale(n , MOUSE_RADIUS));
        PARTICLES[i].position = target;
        PARTICLES[i].previous_position = Vector2Subtract(PARTICLES[i].position , Vector2Scale(n , 25.0F));
    }

}

void UpdateClothParticles(float deltaTime) {
    for (int i = 0; i < VERTICAL_PARTICLE_COUNT * HORIZONTAL_PARTICLE_COUNT; ++i) {

        if (PARTICLES[i].pinned) continue;

        PARTICLES[i].acceleration.y += GRAVITY;

        Vector2 velocity = Vector2Subtract(PARTICLES[i].position , PARTICLES[i].previous_position);
        
        Vector2 new_position = Vector2Add(Vector2Add(PARTICLES[i].position , velocity) , Vector2Scale(PARTICLES[i].acceleration , deltaTime * deltaTime));
        PARTICLES[i].previous_position = PARTICLES[i].position; 
        PARTICLES[i].position = new_position;   
        PARTICLES[i].acceleration = (Vector2) { 0 , 0 };

    }
}

void SolveDistanceConstraint(Particle* a , Particle* b , float REST_LENGTH) {

    Vector2 vec_dist = Vector2Subtract(b->position , a->position);
    float dist = sqrt(vec_dist.x * vec_dist.x + vec_dist.y * vec_dist.y);
    if (dist < 0.0001f) return;

    float diff = (dist - REST_LENGTH) / dist;
    
    if (!a->pinned && !b->pinned) {
        Vector2 correction = {
            vec_dist.x * 0.5 * diff,
            vec_dist.y * 0.5 * diff
        };

        a->position.x += correction.x;
        a->position.y += correction.y;

        b->position.x -= correction.x;
        b->position.y -= correction.y;
    }
    else if (!a->pinned && b->pinned) {
        Vector2 correction = {
            vec_dist.x * diff,
            vec_dist.y * diff
        };

        a->position.x += correction.x;
        a->position.y += correction.y;
    }
    else if (a->pinned && !b->pinned) {
        Vector2 correction = {
            vec_dist.x * diff,
            vec_dist.y * diff
        };

        b->position.x -= correction.x;
        b->position.y -= correction.y;
    }
    
}

void SolveClothConstraints() {
    for (int y = 0; y < VERTICAL_PARTICLE_COUNT; ++y) {
        for (int x = 0; x < HORIZONTAL_PARTICLE_COUNT; ++x) {

            #pragma region STRUCTURAL
            if (x + 1 < HORIZONTAL_PARTICLE_COUNT) {
                SolveDistanceConstraint(&PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x] , &PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 1)] , PARTICLE_REST_LENGTH);
            }

            if (y + 1 < VERTICAL_PARTICLE_COUNT) {
                SolveDistanceConstraint(&PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x] , &PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + x] , PARTICLE_REST_LENGTH);
            }
            #pragma endregion STRUCTURAL

            #pragma region SHEAR
            if (x + 1 < HORIZONTAL_PARTICLE_COUNT && y + 1 < VERTICAL_PARTICLE_COUNT) {
                SolveDistanceConstraint(&PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x] , &PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + (x + 1)] , sqrt(PARTICLE_REST_LENGTH * PARTICLE_REST_LENGTH + PARTICLE_REST_LENGTH * PARTICLE_REST_LENGTH) );
            }

            if (y + 1 < VERTICAL_PARTICLE_COUNT && x + 1 < HORIZONTAL_PARTICLE_COUNT) {
                SolveDistanceConstraint(&PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 1)] , &PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + x] , sqrt(PARTICLE_REST_LENGTH * PARTICLE_REST_LENGTH + PARTICLE_REST_LENGTH * PARTICLE_REST_LENGTH) );
            }
            #pragma endregion SHEAR
        
            #pragma region BEND
            if (x + 2 < HORIZONTAL_PARTICLE_COUNT) {
                SolveDistanceConstraint(&PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x] , &PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 2)] , PARTICLE_REST_LENGTH * 2);
            }

            if (y + 2 < VERTICAL_PARTICLE_COUNT) {
                SolveDistanceConstraint(&PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x] , &PARTICLES[(y + 2) * HORIZONTAL_PARTICLE_COUNT + x] , PARTICLE_REST_LENGTH * 2);
            }
            #pragma endregion BEND
        }
    }
}   

/*
void UpdateWind() {

    for (int y = 0; y < VERTICAL_PARTICLE_COUNT; ++y) {
        for (int x = 0; x < HORIZONTAL_PARTICLE_COUNT; ++x) {

            if (x + 1 < HORIZONTAL_PARTICLE_COUNT - 1) {

                Vector2 e = Vector2Subtract(PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 1)].position , PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position);
                Vector2 n = { -e.y , e.x };

                float L = sqrt(e.x * e.x + e.y * e.y);
                Vector2 nL = Vector2Scale(n , 1.0F/L);

                float I = (WIND.x * nL.x) + (WIND.y * nL.y);
                Vector2 F = Vector2Scale(Vector2Scale(nL , I * L) , 1.0F/2); // CHECK

                if (!PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].pinned) {
                    PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.x += F.x;
                    PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.y += F.y;
                }

                PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 1)].position.x += F.x;
                PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + (x + 1)].position.y += F.y;

            }

            if (y + 1 < VERTICAL_PARTICLE_COUNT - 1) {

                Vector2 e = Vector2Subtract(PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + x].position , PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position);
                Vector2 n = { -e.y , e.x };

                float L = sqrt(e.x * e.x + e.y * e.y);
                Vector2 nL = Vector2Scale(n , 1.0F/L);

                float I = (WIND.x * nL.x) + (WIND.y * nL.y);
                Vector2 F = Vector2Scale(Vector2Scale(nL , I * L) , 1.0F/2);

                if (!PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].pinned) {
                    PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.x += F.x;
                    PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x].position.y += F.y;
                }

                PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + x].position.x += F.x;
                PARTICLES[(y + 1) * HORIZONTAL_PARTICLE_COUNT + x].position.y += F.y;

            }

        }
    }

}
*/

int main(void) {

    InitWindow(WIDTH , HEIGHT , "2D Cloth Physics");
    SetTargetFPS(60);

    for (int y = 0; y < VERTICAL_PARTICLE_COUNT; ++y) {
        for (int x = 0; x < HORIZONTAL_PARTICLE_COUNT; ++x) {
            Vector2 position = { 100 + x * PARTICLE_REST_LENGTH , 100 + y * PARTICLE_REST_LENGTH };
            Vector2 previous_position = position;

            PARTICLES[y * HORIZONTAL_PARTICLE_COUNT + x] = (Particle) { y == 0 && (x == 0 || x == HORIZONTAL_PARTICLE_COUNT - 1) ? true : false , position , previous_position , (Vector2) { 0 , 0 } };
        }
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        /*
        if (IsKeyPressed(KEY_D)) {
            DEBUG_RENDERER = !DEBUG_RENDERER;
        }
        */

        if (DEBUG_RENDERER) {
            // WIND = Vector2Scale(WIND , abs(sin(GetFrameTime() * WIND_SPEED)));
            // UpdateWind();
            
            DrawCircleLines(GetMousePosition().x , GetMousePosition().y , MOUSE_RADIUS , SKYBLUE);

            UpdateClothParticles(GetFrameTime());
            for (int i = 0; i < 3; ++i) {
                SolveClothConstraints();
            }
            UpdateMouseEffect();
            DebugCloth();
            DrawCloth();
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;

}