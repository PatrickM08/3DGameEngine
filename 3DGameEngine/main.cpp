

/*
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera.h"
#include "text.hpp"
#include "asset_manager.hpp"

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
GLFWwindow* initWindow(unsigned int width, unsigned int height, const char* title);
void configureOpenglState(bool depthTestEnabled, bool blendEnabled, bool faceCullEnabled);

struct Framebuffer {
    uint32_t buffer;
    uint32_t textureAttachment;
    uint32_t renderBufferObject;
    Shader shader;
};

Framebuffer createFrameBuffer(const char* vsPath, const char* fsPath, unsigned int width, unsigned int height);

struct Resources {
    Shader lightingShader;
    Shader lightCubeShader;
    Shader textShader;
    Shader skyboxShader;
    MeshBuffers cube;
    MeshBuffers light;
    MeshBuffers text;
    MeshBuffers skybox;
    MeshBuffers quad;
    unsigned int diffuseMap;
    unsigned int specularMap;
    unsigned int bitmapFont;
    unsigned int skyboxTexture;
    std::unordered_map<int, Glyph> glyphs;
    
    Resources()
        : lightingShader("vshader.vs", "fshader.fs"),
        lightCubeShader("light_cube.vs", "light_cube.fs"),
        textShader("text_vertex_shader.vs", "text_fragment_shader.fs"),
        skyboxShader("skybox_vshader.vs", "skybox_fshader.fs")
    {
        createCubeBuffers(cube);
        light.VBO = cube.VBO;
        createLightBuffers(light);

        diffuseMap = loadTexture("container2.png");
        specularMap = loadTexture("container2_specular.png");

        glyphs = parseFont("ariallatin.fnt");
        bitmapFont = loadBitmapFont("ariallatin_0.png", glyphs);
        setupTextBuffers(text);

        createSkyboxbuffers(skybox);

        const char* skyboxFaces[] = {
        "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
        };
        const int NUM_SKYBOX_FACES = 6;
        skyboxTexture = loadCubemap(skyboxFaces, NUM_SKYBOX_FACES);

        createQuadBuffers(quad);
    }
};

struct RenderState {
    unsigned int scr_width = 1600;
    unsigned int scr_height = 1200;
    Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f) };
    glm::vec3 lightPos{ 1.2f, 1.0f, 2.0f };

    // Timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Settings
    bool postProcessingEnabled = false;
    bool showFPS = true;

    // Mouse
    float lastX = scr_width / 2.0f;
    float lastY = scr_height / 2.0f;
    bool firstMouse = true;

    // FPS display
    char fpsText[20] = "0";
    int counter = 1;
};

int main2()
{
    RenderState state;
    GLFWwindow* window;
    try {
        window = initWindow(state.scr_width, state.scr_height, "Draft");
    }
    catch (const std::exception& e){
        std::cerr << "Window init error: " << e.what() << "\n";
        return 1;
    }
    // ****** NEEDS IMPROVEMENT ****** 
    bool depthTestEnabled = true;
    bool blendEnabled = true;
    bool faceCullEnabled = true;
    configureOpenglState(depthTestEnabled,blendEnabled,faceCullEnabled);
    //*******                   *******
    
    Resources resources;
    
    // shader configuration
    // --------------------
    resources.lightingShader.use();
    resources.lightingShader.setIntUniform("material.diffuse", 0);
    resources.lightingShader.setIntUniform("material.specular", 1);

    
    resources.textShader.use();
    resources.textShader.setIntUniform("textTexture", 0);

    glm::mat4 projection;
    glm::mat4 view;
    

    Framebuffer framebuffer = createFrameBuffer("fb_vertex_shader.vs", "fb_fragment_shader.fs", state.scr_width, state.scr_height);

    resources.skyboxShader.use();
    resources.skyboxShader.setIntUniform("skybox", 0);


    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        state.deltaTime = currentFrame - state.lastFrame;
        state.lastFrame = currentFrame;
        float fps = 1.0f / state.deltaTime;
        state.counter += 1;
        state.counter = state.counter % 10;
        if (state.counter == 0) {
            snprintf(state.fpsText, sizeof(state.fpsText), "FPS: %.0f", fps);
        }

        // input
        // -----
        processInput(window);

        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.buffer);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        glEnable(GL_DEPTH_TEST);

        // be sure to activate shader when setting uniforms/drawing objects
        resources.lightingShader.use();
        resources.lightingShader.setVec3Uniform("light.position", state.lightPos);
        resources.lightingShader.setVec3Uniform("viewPos", state.camera.Position);

        // light properties
        resources.lightingShader.setVec3Uniform("light.ambient", 0.2f, 0.2f, 0.2f);
        resources.lightingShader.setVec3Uniform("light.diffuse", 0.5f, 0.5f, 0.5f);
        resources.lightingShader.setVec3Uniform("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        resources.lightingShader.setFloatUniform("material.shininess", 64.0f);

        // view/projection transformations
        projection = glm::perspective(glm::radians(state.camera.Zoom), (float)state.scr_width / (float)state.scr_height, 0.1f, 100.0f);
        view = state.camera.GetViewMatrix();
        resources.lightingShader.setMat4Uniform("projection", projection);
        resources.lightingShader.setMat4Uniform("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        resources.lightingShader.setMat4Uniform("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, resources.diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, resources.specularMap);

        // render the cube
        glBindVertexArray(resources.cube.VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 1000);

        // also draw the lamp object
        resources.lightCubeShader.use();
        resources.lightCubeShader.setMat4Uniform("projection", projection);
        resources.lightCubeShader.setMat4Uniform("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, state.lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        resources.lightCubeShader.setMat4Uniform("model", model);

        glBindVertexArray(resources.light.VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthFunc(GL_LEQUAL);
        resources.skyboxShader.use();
        view = glm::mat4(glm::mat3(state.camera.GetViewMatrix()));
        resources.skyboxShader.setMat4Uniform("view", view);
        resources.skyboxShader.setMat4Uniform("projection", projection);
        glBindVertexArray(resources.skybox.VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, resources.skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        if (state.showFPS) {
            glDisable(GL_DEPTH_TEST);
            resources.textShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, resources.bitmapFont);
            glBindVertexArray(resources.text.VAO);
            float fontScale = 2;
            float fontX = 10;
            float fontY = 10;
            char* fpsTextPointer = state.fpsText;
            renderText(fpsTextPointer, fontX, fontY, fontScale, resources.text.VBO, resources.textShader, state.scr_width, state.scr_height, resources.glyphs);
            glEnable(GL_DEPTH_TEST);
        }

        if (state.postProcessingEnabled) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            framebuffer.shader.use();
            glBindVertexArray(resources.quad.VAO);
            glDisable(GL_DEPTH_TEST);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, framebuffer.textureAttachment);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.buffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, state.scr_width, state.scr_height, 0, 0, state.scr_width, state.scr_height, 
                                GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    resources.cube.deleteBuffers();
    resources.light.deleteBuffers();
    resources.quad.deleteBuffers();
    resources.skybox.deleteBuffers();
    resources.text.deleteBuffers();
    glDeleteFramebuffers(1, &framebuffer.buffer);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, RenderState& state)
{
    static bool eWasPressed = false;
    bool eIsPressed = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        state.camera.ProcessKeyboard(FORWARD, state.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        state.camera.ProcessKeyboard(BACKWARD, state.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        state.camera.ProcessKeyboard(LEFT, state.deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        state.camera.ProcessKeyboard(RIGHT, state.deltaTime);
    if (eIsPressed && !eWasPressed) {
        state.showFPS = !state.showFPS;
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
    //SCR_HEIGHT = height;
    //SCR_WIDTH = width;
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

Framebuffer createFrameBuffer(const char* vsPath, const char* fsPath, unsigned int width, unsigned int height) {
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Shader framebufferShader(vsPath, fsPath);
    return Framebuffer{
        .buffer = framebuffer,
        .textureAttachment = textureColorbuffer,
        .renderBufferObject = rbo,
        .shader = framebufferShader
    };
}

GLFWwindow* initWindow(unsigned int width, unsigned int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable VSync (1 = sync every frame)
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::runtime_error("GLAD init failed");
    }
    return window;
}

void configureOpenglState(const bool depthTestEnabled, const bool blendEnabled, const bool faceCullEnabled) {
    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    if (blendEnabled)
        glEnable(GL_BLEND);
    if (faceCullEnabled) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
    };
}
*/