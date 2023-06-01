#include <cstddef>
#include <fstream>
#include "./boid/boid.hpp"
#include "./camera/camera.hpp"
#include "./include/wrapper.hpp"
#include "./loaderGLTF/Model.h"
#include "./player/player.hpp"
#include "boids/boids.hpp"
#include "glimac/default_shader.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "p6/p6.h"

void moveListener(const p6::Context& ctx, Player& player)
{
    if (ctx.key_is_pressed(GLFW_KEY_W))
    {
        player.goForward(ctx);
    }
    else
    {
        player.brake(ctx);
    }
    if (ctx.key_is_pressed(GLFW_KEY_S))
    {
        player.goBackward(ctx);
    }
    if (ctx.key_is_pressed(GLFW_KEY_A))
    {
        player.rotateLeft(ctx);
    }
    if (ctx.key_is_pressed(GLFW_KEY_D))
    {
        player.rotateRight(ctx);
    }
    if (ctx.key_is_pressed(GLFW_KEY_LEFT_SHIFT))
    {
        player.goUp(ctx);
    }
    if (ctx.key_is_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        player.goDown(ctx);
    }
}

int main()
{
    auto ctx = p6::Context{{1280, 720, "Boids in space"}};
    ctx.maximize_window();

    glEnable(GL_DEPTH_TEST);

    const p6::Shader shader = p6::load_shader(
        "shaders/shader.vs.glsl",
        "shaders/shader.fs.glsl"
    );

    const p6::Shader shaderGLTF = p6::load_shader(
        "shaders/gltf.vs.glsl",
        "shaders/gltf.fs.glsl"
    );

    /*********************************
     * HERE SHOULD COME THE INITIALIZATION CODE
     *********************************/

    Wrapper ground;

    struct Vertex3D g1 = {glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.5f, 0.f)};
    struct Vertex3D g2 = {glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.5f)};
    struct Vertex3D g3 = {glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.5f, 1.f)};
    struct Vertex3D g4 = {glm::vec3(0.0f, 0.5f, 0.f), glm::vec3(0.f, 0.5f, 1.f)};
    ground.vertices.push_back(g1);
    ground.vertices.push_back(g2);
    ground.vertices.push_back(g3);
    ground.vertices.push_back(g4);

    ground.indices.push_back(0);
    ground.indices.push_back(1);
    ground.indices.push_back(2);
    ground.indices.push_back(0);
    ground.indices.push_back(2);
    ground.indices.push_back(3);

    ground.init();

    /*********************/

    // matrice de projection, permet de delimiter champ qu'on voit, fait perspective
    // perspective(fov, aspect ratio fenetre,distance min, distance max)
    glm::mat4 projection = glm::perspective(glm::radians(45.f), static_cast<float>(1280) / static_cast<float>(720), 0.001f, 100.0f);

    Camera      camera;
    std::string playerFileGLTF = "./assets/models/vaisseauFinal.gltf";
    // glm::mat4   basePlayer     = glm::mat4(1.f);
    Player player(playerFileGLTF);

    glm::vec3 lightColor(1, 0.92, 0.85);
    glm::vec3 lightPosition(0, 100, 0);

    // const size_t    nbSquare   = 50;
    // const glm::vec2 squareSize = {5, 5};
    // const float     size       = 1;

    scopes    scopes;
    strengths strengths;

    std::string boidsFileGLTF = "./assets/models/planetesAllegee.gltf";
    Boids       boids(boidsFileGLTF);
    boids.init();

    // Declare your infinite update loop.
    ctx.update = [&]() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ctx.imgui = [&]() {
            ImGui::Begin("Boids");
            ImGui::SliderFloat("Scope", &scopes.scope, 0.f, 1.f);
            ImGui::SliderFloat("Separation strength", &strengths.separationStrength, 0.f, 0.1);
            ImGui::SliderFloat("Alignement strength", &strengths.alignementStrength, 0.f, 0.1);
            ImGui::SliderFloat("Cohesion strength", &strengths.cohesionStrength, 0.f, 0.1);

            ImGui::End();
        };

        moveListener(ctx, player);
        player.calcDir();

        shader.use();
        // envoie matrice au shader
        shader.set("projection", projection);

        // "world", translation/rotation etc d'un objet
        // rotate(matrice courante (identité car c'est la 1ère transfo), rotation, axe autour duquel se fait rotation)
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // model = glm::translate(model, )
        // matrice normale, définit où va être cam
        // lookAt(pos ete, centre d'ou tu regarde, où est le haut)
        // fc fait en sorte que l'horizon est tjrs droit
        camera.calCoords(player);

        glm::mat4 view = glm::lookAt(camera.getCoords(), player.getPos(), {0, 1, 0});

        shader.set("model", model);
        shader.set("view", view);

        // ground.update();
        // box.update();

        // pour init model on la met égale à matrice identité
        // model = glm::mat4(1.0f);

        shaderGLTF.use();
        shaderGLTF.set("view", view);
        shaderGLTF.set("projection", projection);
        shaderGLTF.set("model", model);
        shaderGLTF.set("lightColor", lightColor);
        shaderGLTF.set("lightPosition", lightPosition);
        shaderGLTF.set("camPos", camera.getCoords());

        player.draw(shaderGLTF);

        // std::cout << boids.getBoids().at(0).getPosition().x << std::endl;
        boids.draw(shaderGLTF);
        boids.update(scopes, strengths);

        // cube.Draw(shaderGLTF.id());

        /*********************************
         * HERE SHOULD COME THE RENDERING CODE
         *********************************/
        // glimac::bind_default_shader();

        /*********************************/
    };

    // Should be done last. It starts the infinite loop.
    ctx.start();
    // triangle.clear();
}