#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "asset_manager.hpp"

void createQuadBuffers(MeshBuffers& quad) {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quad.VAO);
    glGenBuffers(1, &quad.VBO);

    glBindVertexArray(quad.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void createCubeBuffers(MeshBuffers& cube) {
    float vertices[] = {
        // Back face (-Z)
        -0.5f, -0.5f, -0.5f,  0, 0, -1,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0, 0, -1,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0, 0, -1,  1.0f, 0.0f,

         0.5f,  0.5f, -0.5f,  0, 0, -1,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0, 0, -1,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0, 0, -1,  0.0f, 1.0f,

        // Front face (+Z)
        -0.5f, -0.5f,  0.5f,  0, 0, 1,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0, 0, 1,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0, 0, 1,  1.0f, 1.0f,

         0.5f,  0.5f,  0.5f,  0, 0, 1,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0, 0, 1,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0, 0, 1,  0.0f, 0.0f,

        // Left face (-X)
        -0.5f,  0.5f,  0.5f,  -1, 0, 0,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1, 0, 0,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1, 0, 0,  0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, -1, 0, 0,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1, 0, 0,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1, 0, 0,  1.0f, 0.0f,

        // Right face (+X)
         0.5f,  0.5f,  0.5f,  1, 0, 0,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1, 0, 0,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1, 0, 0,  1.0f, 1.0f,

         0.5f, -0.5f, -0.5f,  1, 0, 0,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1, 0, 0,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1, 0, 0,  0.0f, 0.0f,

         // Bottom face (-Y)
         -0.5f, -0.5f, -0.5f,  0, -1, 0,  0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,  0, -1, 0,  1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  0, -1, 0,  1.0f, 0.0f,

          0.5f, -0.5f,  0.5f,  0, -1, 0,  1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0, -1, 0,  0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0, -1, 0,  0.0f, 1.0f,

         // Top face (+Y)
         -0.5f,  0.5f, -0.5f,  0, 1, 0,  0.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0, 1, 0,  1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  0, 1, 0,  1.0f, 1.0f,

          0.5f,  0.5f,  0.5f,  0, 1, 0,  1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0, 1, 0,  0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0, 1, 0,  0.0f, 0.0f
    };

    glGenVertexArrays(1, &cube.VAO);
    glGenBuffers(1, &cube.VBO);

    glBindVertexArray(cube.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    const int NUM_BOXES = 1024;
    int index = 0;
    glm::vec3 translations[NUM_BOXES];
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            translations[index].x = i * 2;
            translations[index].y = 0;
            translations[index].z = -j * 2;
            index++;
        }
    }

    glGenBuffers(1, &cube.instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube.instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_BOXES, translations, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
}

void createLightBuffers(MeshBuffers& light) {
    glGenVertexArrays(1, &light.VAO);
    glBindVertexArray(light.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, light.VBO);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void createSkyboxbuffers(MeshBuffers& skybox) {
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    glGenVertexArrays(1, &skybox.VAO);
    glGenBuffers(1, &skybox.VBO);
    glBindVertexArray(skybox.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, skybox.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}