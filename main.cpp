#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include "glm/geometric.hpp"
#include <string>
#include <vector>
#include "print.h"
#include "color.h"
#include "intersect.h"
#include "object.h"
#include "cube.h"
#include "light.h"
#include "camera.h"

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
const float ASPECT_RATIO = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
const int MAX_RECURSION = 3;
const float BIAS = 0.0001f;

SDL_Renderer* renderer;
std::vector<Object*> objects;
Light light(glm::vec3(-1.0, 0, 10), 1.5f, Color(255, 255, 255));
Camera camera(glm::vec3(0.0, 0.0, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);

void point(glm::vec2 position, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

float castShadow(const glm::vec3& shadowOrigin, const glm::vec3& lightDir, Object* hitObject) {
    for (auto& obj : objects) {
        if (obj != hitObject) {
            Intersect shadowIntersect = obj->rayIntersect(shadowOrigin, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.dist > 0) {
                float shadowRatio = shadowIntersect.dist / glm::length(light.position - shadowOrigin);
                shadowRatio = glm::min(1.0f, shadowRatio);
                return 1.0f - shadowRatio;
            }
        }
    }
    return 1.0f;
}

Color castRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const short recursion = 0) {
    float zBuffer = 99999;
    Object* hitObject = nullptr;
    Intersect intersect;

    for (const auto& object : objects) {
        Intersect i = object->rayIntersect(rayOrigin, rayDirection);
        if (i.isIntersecting && i.dist < zBuffer) {
            zBuffer = i.dist;
            hitObject = object;
            intersect = i;
        }
    }

    float yPosition = rayDirection.y;

    // Colors for the sky, grass, and dirt
    Color dirtColor(139, 69, 19);
    Color grassColor(0, 100, 0);
    Color skyColor(173, 216, 230);

    if (!intersect.isIntersecting || recursion == MAX_RECURSION) {
        if (yPosition < -0.35f) {
            return dirtColor;
        } else if (yPosition < -0.1f) {
            return grassColor;
        } else {
            return skyColor;
        }
    }

    glm::vec3 lightDir = glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::normalize(rayOrigin - intersect.point);
    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal);

    float shadowIntensity = castShadow(intersect.point, lightDir, hitObject);
    float diffuseLightIntensity = std::max(0.0f, glm::dot(intersect.normal, lightDir));
    float specReflection = glm::dot(viewDir, reflectDir);
    Material mat = hitObject->material;
    float specLightIntensity = std::pow(std::max(0.0f, glm::dot(viewDir, reflectDir)), mat.specularCoefficient);

    Color reflectedColor(0.0f, 0.0f, 0.0f);
    if (mat.reflectivity > 0) {
        glm::vec3 origin = intersect.point + intersect.normal * BIAS;
        reflectedColor = castRay(origin, reflectDir, recursion + 1);
    }

    Color refractedColor(0.0f, 0.0f, 0.0f);
    if (mat.transparency > 0) {
        glm::vec3 origin = intersect.point - intersect.normal * BIAS;
        glm::vec3 refractDir = glm::refract(rayDirection, intersect.normal, mat.refractionIndex);
        refractedColor = castRay(origin, refractDir, recursion + 1);
    }

    Color diffuseLight = mat.diffuse * light.intensity * diffuseLightIntensity * mat.albedo * shadowIntensity;
    Color specularLight = light.color * light.intensity * specLightIntensity * mat.specularAlbedo * shadowIntensity;
    Color color = (diffuseLight + specularLight) * (1.0f - mat.reflectivity - mat.transparency) + reflectedColor * mat.reflectivity + refractedColor * mat.transparency;
    return color;
}

//setUp cube
void setUp() {
    Material stoneMaterial = {
            Color(125, 125, 125),
            0.8,
            0.5,
            25.0f,
            0.1f,
            0.0f,
    };

    Material grassMaterial = {
            Color(0, 128, 0),
            0.7,
            0.3,
            10.0f,
            0.1f,
            0.0f
    };

    Material brickMaterial = {
            Color(100, 0, 0),
            0.8,
            0.3,
            15.0f,
            0.1f,
            0.0f
    };

    Material woodMaterial = {
            Color(139, 69, 19),
            0.7,
            0.3,
            20.0f,
            0.1f,
            0.0f
    };

    Material glassMaterial = {
            Color(255, 255, 255),
            0.0f,
            10.0f,
            1425.0f,
            0.2f,
            1.0f
    };

    //Tree
    objects.push_back(new Cube(glm::vec3(4.0f, -1.0f, -1.5f), 1.0f, woodMaterial, "wood"));
    objects.push_back(new Cube(glm::vec3(4.0f, 0.0f, -1.5f), 1.0f, woodMaterial, "wood"));
    objects.push_back(new Cube(glm::vec3(4.0f, 1.0f, -1.5f), 1.0f, woodMaterial, "wood"));
    objects.push_back(new Cube(glm::vec3(4.0f, 2.0f, -1.5f), 1.0f, grassMaterial, "grass"));
    objects.push_back(new Cube(glm::vec3(3.0f, 2.0f, -1.5f), 1.0f, grassMaterial, "grass"));
    objects.push_back(new Cube(glm::vec3(5.0f, 2.0f, -1.5f), 1.0f, grassMaterial, "grass"));
    objects.push_back(new Cube(glm::vec3(4.0f, 3.0f, -1.5f), 1.0f, grassMaterial, "grass"));

    //House
    //X1 Y1
    objects.push_back(new Cube(glm::vec3(1.0f, -1.0f, -1.5f), 1.0f, grassMaterial, "grass"));
    objects.push_back(new Cube(glm::vec3(0.0f, -1.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-1.0f, -1.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    //objects.push_back(new Cube(glm::vec3(-3.0f, -2.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, -1.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-4.0f, -1.0f, -1.5f), 1.0f, grassMaterial, "grass"));

    //X2 Y1
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-1.0f, 0.0f, -1.5f), 1.0f, glassMaterial, "glass"));
    //objects.push_back(new Cube(glm::vec3(-2.0f, 0.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 0.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    //X3 Y1
    objects.push_back(new Cube(glm::vec3(-0.0f, 1.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-1.0f, 1.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-2.0f, 1.0f, -1.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 1.0f, -1.5f), 1.0f, stoneMaterial, "stone"));

    //X2 Y1
    objects.push_back(new Cube(glm::vec3(0.0f, -1.0f, -2.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, -1.0f, -2.5f), 1.0f, stoneMaterial, "stone"));
    //X2 Y2
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, -2.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 0.0f, -2.5f), 1.0f, stoneMaterial, "stone"));
    //X2 Y3
    objects.push_back(new Cube(glm::vec3(0.0f, 1.0f, -2.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 1.0f, -2.5f), 1.0f, stoneMaterial, "stone"));

    //X3 Y1
    objects.push_back(new Cube(glm::vec3(0.0f, -1.0f, -3.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, -1.0f, -3.5f), 1.0f, stoneMaterial, "stone"));
    //X3 Y2
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, -3.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 0.0f, -3.5f), 1.0f, stoneMaterial, "stone"));
    //X3 Y3
    objects.push_back(new Cube(glm::vec3(0.0f, 1.0f, -3.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 1.0f, -3.5f), 1.0f, stoneMaterial, "stone"));

    //X4 Y1
    objects.push_back(new Cube(glm::vec3(0.0f, -1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-1.0f, -1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-2.0f, -1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, -1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    //X4 Y2
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-1.0f, 0.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-2.0f, 0.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 0.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    //X4 Y3
    objects.push_back(new Cube(glm::vec3(0.0f, 1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-1.0f, 1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-2.0f, 1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));
    objects.push_back(new Cube(glm::vec3(-3.0f, 1.0f, -4.5f), 1.0f, stoneMaterial, "stone"));

    //Chimney
    objects.push_back(new Cube(glm::vec3(-3.2f, 1.7f, -1.5f), 0.6f, brickMaterial, "brick"));
    objects.push_back(new Cube(glm::vec3(-3.2f, 2.0f, -1.5f), 0.6f, brickMaterial, "brick"));
}

void render() {
    float fov = 3.1415/3;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float screenX = (2.0f * (x + 0.5f)) / SCREEN_WIDTH - 1.0f;
            float screenY = -(2.0f * (y + 0.5f)) / SCREEN_HEIGHT + 1.0f;
            screenX *= ASPECT_RATIO;
            screenX *= tan(fov/2.0f);
            screenY *= tan(fov/2.0f);

            glm::vec3 cameraDir = glm::normalize(camera.target - camera.position);
            glm::vec3 cameraX = glm::normalize(glm::cross(cameraDir, camera.up));
            glm::vec3 cameraY = glm::normalize(glm::cross(cameraX, cameraDir));
            glm::vec3 rayDirection = glm::normalize(
                    cameraDir + cameraX * screenX + cameraY * screenY
            );

            Color pixelColor = castRay(camera.position, rayDirection);
            point(glm::vec2(x, y), pixelColor);
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("Hello World - FPS: 0",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    int frameCount = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime = startTime;

    setUp();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        camera.move(-1.0f);
                        break;
                    case SDLK_DOWN:
                        camera.move(1.0f);
                        break;
                    case SDLK_LEFT:
                        print("left");
                        camera.rotate(-1.0f, 0.0f);
                        break;
                    case SDLK_RIGHT:
                        print("right");
                        camera.rotate(1.0f, 0.0f);
                        break;
                }
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render();

        // Present the renderer
        SDL_RenderPresent(renderer);
        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            currentTime = SDL_GetTicks();
            std::string title = "Hello World - FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
