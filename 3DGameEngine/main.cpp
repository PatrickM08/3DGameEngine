

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera.h"
#include "text.hpp"

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <cstdio>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(const char **faces, const int numberOfFaces);



// settings
unsigned int SCR_WIDTH = 1600;
unsigned int SCR_HEIGHT = 1200;
const int MAX_TEXT_LENGTH = 30;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// post-processing
bool postProcessingEnabled = false;

// FPS
bool showFPS = true;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable VSync (1 = sync every frame)
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("vshader.vs", "fshader.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");
    Shader textShader("text_vertex_shader.vs", "text_fragment_shader.fs");
    Shader skyboxShader("skybox_vshader.vs", "skybox_fshader.fs");


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
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


    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, instancedVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &instancedVBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

    glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_BOXES, translations, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture("container2.png");
    unsigned int specularMap = loadTexture("container2_specular.png");

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    
    std::unordered_map<int, Glyph> glyphs = parseFont("ariallatin.fnt");
    unsigned int bitmapFont = loadBitmapFont("ariallatin_0.png", glyphs);
    textShader.use();
    textShader.setInt("textTexture", 0);
    unsigned int textVBO, textVAO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    setupTextBuffers(textVBO, textVAO);

    glm::mat4 projection;
    glm::mat4 view;

    int counter = 1;
    char fpsText[20] = "0";
    

    // Framebuffer
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader framebufferShader("fb_vertex_shader.vs", "fb_fragment_shader.fs");

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVBO, quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    const char* skyboxFaces[] = {
        "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
    };
    const int NUM_SKYBOX_FACES = 6;
    unsigned int skyboxTexture = loadCubemap(skyboxFaces, NUM_SKYBOX_FACES);
    if (skyboxTexture == 0) {
        std::cout << "Failed to load skybox texture!" << std::endl;
    }

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
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);


    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float fps = 1.0f / deltaTime;
        counter += 1;
        counter = counter % 10;
        if (counter == 0) {
            snprintf(fpsText, sizeof(fpsText), "FPS: %.0f", fps);
        }

        // input
        // -----
        processInput(window);

        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        glEnable(GL_DEPTH_TEST);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("light.position", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);

        // light properties
        lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader.setFloat("material.shininess", 64.0f);

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // render the cube
        glBindVertexArray(cubeVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1000);

        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        if (showFPS) {
            glDisable(GL_DEPTH_TEST);
            textShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, bitmapFont);
            glBindVertexArray(textVAO);
            float fontScale = 2;
            float fontX = 10;
            float fontY = 10;
            char* fpsTextPointer = fpsText;
            renderText(fpsTextPointer, fontX, fontY, fontScale, textVBO, textShader, SCR_WIDTH, SCR_HEIGHT, glyphs);
            glEnable(GL_DEPTH_TEST);
        }

        if (postProcessingEnabled) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            framebufferShader.use();
            glBindVertexArray(quadVAO);
            glDisable(GL_DEPTH_TEST);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, 
                                GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &textVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteFramebuffers(1, &framebuffer);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    static bool eWasPressed = false;
    bool eIsPressed = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (eIsPressed && !eWasPressed) {
        showFPS = !showFPS;
    }
    eWasPressed = eIsPressed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(const char **faces, const int numberOfFaces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < numberOfFaces; i++)
    {
        unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}