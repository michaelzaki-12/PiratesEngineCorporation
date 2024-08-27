#include "glad.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include "stb_write_image.h"
#include "Renderers/OpenGLRenderer/Shader.h"
#include "Renderers/OpenGLRenderer/VAO.h"
#include "Renderers/OpenGLRenderer/VBO.h"
#include "Renderers/OpenGLRenderer/Texture.h"
#include "Renderers/OpenGLRenderer/Mesh.h"
#include "Renderers/OpenGLRenderer/AssimpModel.h"
#include <filesystem>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <filesystem>
#include <imgui.h>
#include "Deps/Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#ifdef WIN32
#include "Windows.h"
#endif

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
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

int main()
{
#ifdef WIN32
    ShowWindow(GetConsoleWindow(), 5);
#endif
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfw window creation
    // --------------------
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    //glfw window initionally width and height
    int Width = 800, Height = 600;
    GLFWwindow* window = glfwCreateWindow(Width, Height, "Pirates Engine", NULL, NULL);

    //loading our Engine Icon
    GLFWimage images[1]{};
    images[0].pixels = stbi_load("image.png", &images[0].width, &images[0].height, 0, 4); //rgba channels 
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    camera.width = Width;
    camera.height = Height;
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, 800, 600);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view;
    // build and compile our shader program
    // ------------------------------------
    Shader ourShader;
    ourShader.Init("ShaderSrc/default.vert", "ShaderSrc/default.frag");

    Shader skybox;
    skybox.Init("ShaderSrc/skybox.vert", "ShaderSrc/skybox.frag");
    //stbi_set_flip_vertically_on_load(true);


    std::vector<std::string> faces = {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg"
    };
    std::vector<std::string> oneface = {
        "cloud.jpg",
        "cloud.jpg",
        "cloud.jpg",
        "cloud.jpg",
        "cloud.jpg",
        "cloud.jpg"
    };

    
    AssimpModel Model("plane/scene.gltf");

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
    
    // Our light Cube vertex 
    std::vector<Vertex> Cubevertices = {
        // COORDINATES                         //Normals                            // TexCoord                      
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),  glm::vec3( 0.0f,  0.0f, -1.0f),  glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f, -0.5f),  glm::vec3( 0.0f,  0.0f, -1.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f, -0.5f),  glm::vec3( 0.0f,  0.0f, -1.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f, -0.5f),  glm::vec3( 0.0f,  0.0f, -1.0f),  glm::vec2(1.0f, 1.0f)}, //good
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),  glm::vec3( 0.0f,  0.0f, -1.0f),  glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),  glm::vec3( 0.0f,  0.0f, -1.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),  glm::vec3( 0.0f,  0.0f,  1.0f),  glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f,  0.5f),  glm::vec3( 0.0f,  0.0f,  1.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f),  glm::vec3( 0.0f,  0.0f,  1.0f),  glm::vec2(1.0f, 1.0f)}, // good
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f),  glm::vec3( 0.0f,  0.0f,  1.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),  glm::vec3( 0.0f,  0.0f,  1.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),  glm::vec3( 0.0f,  0.0f,  1.0f),  glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),  glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),  glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),  glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),  glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f)}, // good
        Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),  glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),  glm::vec3(-1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f),  glm::vec3( 1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f, -0.5f),  glm::vec3( 1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f, -0.5f),  glm::vec3( 1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f, -0.5f),  glm::vec3( 1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 1.0f)}, // good
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f),  glm::vec3( 1.0f,  0.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f,  0.5f),  glm::vec3( 1.0f,  0.0f,  0.0f),  glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),  glm::vec3( 0.0f, -1.0f,  0.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f, -0.5f),  glm::vec3( 0.0f, -1.0f,  0.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f,  0.5f),  glm::vec3( 0.0f, -1.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f,  0.5f),  glm::vec3( 0.0f, -1.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f,  0.5f),  glm::vec3( 0.0f, -1.0f,  0.0f),  glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f),  glm::vec3( 0.0f, -1.0f,  0.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),  glm::vec3( 0.0f,  1.0f,  0.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f),  glm::vec3( 0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f, -0.5f),  glm::vec3( 0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f),  glm::vec3( 0.0f,  1.0f,  0.0f),  glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f, -0.5f),  glm::vec3( 0.0f,  1.0f,  0.0f),  glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),  glm::vec3(0.0f,  1.0f,  0.0f),   glm::vec2(0.0f, 0.0f)}
    };

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
        // positions   // texCoords
        -0.3f,  1.0f,  0.0f, 1.0f,
        -0.3f,  0.7f,  0.0f, 0.0f,
         0.3f,  0.7f,  1.0f, 0.0f,

        -0.3f,  1.0f,  0.0f, 1.0f,
         0.3f,  0.7f,  1.0f, 0.0f,
         0.3f,  1.0f,  1.0f, 1.0f
    };
    VAO CubeVAO;
    VBO CubeVBO;
    CubeVAO.GenerateVAO();
    CubeVBO.GenerateVBO();

    CubeVAO.Bind();
    CubeVBO.Bind(Cubevertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    Texture cubetex;
    cubetex.LoadTexture("container2.png");
    cubetex.GenerateTexture(0, GL_REPEAT);


    unsigned int skyboxvao, skyboxvbo;
    glGenVertexArrays(1, &skyboxvao);
    glGenBuffers(1, &skyboxvbo);

    glBindVertexArray(skyboxvao);
    
    glBindBuffer(GL_ARRAY_BUFFER, skyboxvbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    Texture cubemap;
    stbi_set_flip_vertically_on_load(false);
    unsigned int skyboxTexture = loadCubemap(faces);
    //stbi_set_flip_vertically_on_load(true);

    unsigned int Vao, Vbo;
    glGenVertexArrays(1, &Vao);
    glGenBuffers(1, &Vbo);

    glBindVertexArray(Vao);
    glBindBuffer(GL_ARRAY_BUFFER, Vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);



    // render loop
    // -----------
    double xpos, ypos;
    int glfwwidth = 0, glfwheight = 0;
    ourShader.use();
    ourShader.setInt("material.texture_diffuse1", 0);
    ourShader.setInt("material.texture_specular1", 1);

    skybox.use();
    skybox.setInt("skybox", 0);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    while (!glfwWindowShouldClose(window)){
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::string title = "PiratesEngine - " + std::to_string(1 / deltaTime) + "FPS";
        glfwSetWindowTitle(window, title.c_str());
        glfwGetWindowSize(window, &glfwwidth, &glfwheight);
        if (glfwwidth != Width || glfwheight != Height) {
            camera.width = glfwwidth;
            camera.height = glfwheight;
            Width = glfwwidth;
            Height = glfwheight;
            glViewport(0, 0, Width, Height);
        }
        projection = glm::perspective(glm::radians(45.0f), (float)Width / Height, 0.1f, 1000.0f);
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        //glStencilFunc(GL_EQUAL, 1, 0xFF);
        glfwGetCursorPos(window, &xpos, &ypos);
        camera.ProcessKeyboard(window, deltaTime);

        camera.ProcessMouseMovement(window, xpos, ypos);
        // render the triangle
        ourShader.use();
        ourShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
        ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("light.direction", camera.Front.x, camera.Front.y, camera.Front.z);
        ourShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
        ourShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.linear", 0.09f);
        ourShader.setFloat("light.quadratic", 0.032f);
        
        //point light specfication;
        ourShader.setVec3("material.ambient", 0.0f, 0.0f, 0.0f);
        ourShader.setFloat("material.shininess",64.0f);
        view = camera.GetViewMatrix();

        //glm::mat3 transposedmodel =  glm::transpose(glm::inverse(model));
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// it's a bit too big for our scene, so scale it down

        ourShader.setMat4("model", model);
        Model.Draw(ourShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        CubeVAO.Bind();
        cubetex.Bind(0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        skybox.use();
        skybox.setMat4("projection", projection);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skybox.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        skybox.setMat4("model", model);

        glBindVertexArray(skyboxvao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            GLsizei nrChannels = 3;
            GLsizei stride = nrChannels * width;
            GLsizei bufferSize = stride * height;
            std::vector<char> buffer(bufferSize);
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
            glReadBuffer(GL_FRONT);
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
            stbi_flip_vertically_on_write(true);
            stbi_write_png("my_png_file.png", width, height, nrChannels, buffer.data(), stride);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            // TODO for OpenGL: restore current GL context.
            glfwMakeContextCurrent(window);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapInterval(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}