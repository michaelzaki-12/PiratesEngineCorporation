#include "glad.h"
#include "glad/khrplatform.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "iostream"
#include "stb_write_image.h"
#include "Renderers/OpenGLRenderer/Shader.h"
#include "Renderers/OpenGLRenderer/ComputeShader.h"
#include "Renderers/OpenGLRenderer/VAO.h"
#include "Renderers/OpenGLRenderer/VBO.h"
#include "Renderers/OpenGLRenderer/Texture.h"
#include "Renderers/OpenGLRenderer/Mesh.h"
#include "Renderers/OpenGLRenderer/AssimpModel.h"
#include <filesystem>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui.h>
#include "Deps/Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#ifdef WIN32
#include "Windows.h"
#endif

struct alignas(16) Cluster
{
    glm::vec4 minPoint; // 16 bytes
    glm::vec4 maxPoint; // 16 bytes
    unsigned int count; // 4 bytes
    unsigned int lightIndices[100]; // 400 bytes
};

struct alignas(16) PointLight
{
    glm::vec4 position;
    glm::vec4 Color;
    
    float constant;
    float linear;
    float quadratic;
    float radius;
};


int main()
{
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
    bool gamma = true;
    // build and compile our shader program
    // ------------------------------------
    Shader ourShader;
    ourShader.Init("ShaderSrc/default.vert", "ShaderSrc/default.frag");


    Shader Framebuffer;
    Framebuffer.Init("ShaderSrc/liningShader.vert", "ShaderSrc/liningShader.frag");

    Shader SimpleShader;
    SimpleShader.Init("ShaderSrc/SimpleShader.vert", "ShaderSrc/SimpleShader.frag", "ShaderSrc/SimpleShader.geom");

    Shader DepthDebug;
    DepthDebug.Init("ShaderSrc/DepthDebug.vert", "ShaderSrc/DepthDebug.frag");

    Shader LightShader;
    LightShader.Init("ShaderSrc/light.vert", "ShaderSrc/light.frag");
    
    ComputeShader lightingengine("ShaderSrc/Clusters.comp");
    ComputeShader CullEngine("ShaderSrc/Cull.comp");

    Shader BlurShader;
    BlurShader.Init("ShaderSrc/standard.vert", "ShaderSrc/Blur.frag");

    Shader hdrskybox;
    hdrskybox.Init("ShaderSrc/Equirectangular.vert", "ShaderSrc/Equirectangular.frag");
    
    //Shader BackGroundShader;
    //BackGroundShader.Init("ShaderSrc/BackGround.vert", "ShaderSrc/BackGround.frag");
    //Shader skybox;
    //skybox.Init("ShaderSrc/skybox.vert", "ShaderSrc/skybox.frag");
    //
    //Shader MetroitsShader;
    //MetroitsShader.Init("ShaderSrc/Instanced.vert", "ShaderSrc/Instanced.frag");
    //Shader NormalShader;
    //NormalShader.Init("ShaderSrc/Normals.vert", "ShaderSrc/Normals.frag", "ShaderSrc/Normals.geom");

    //Shader Mirror;
    //Mirror.Init("ShaderSrc/Mirror.vert", "ShaderSrc/Mirror.frag");
    //stbi_set_flip_vertically_on_load(true);


    std::vector<std::string> faces = {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg"
    };
    float heightScale = 0.1f;
    std::vector<std::string> oneface = {
        "Standard-Cube-Map/px.png",
        "Standard-Cube-Map/nx.png",
        "Standard-Cube-Map/py.png",
        "Standard-Cube-Map/ny.png",
        "Standard-Cube-Map/pz.png",
        "Standard-Cube-Map/nz.png"
    };

    std::string hdrface = {
        "wildflower_field_2k.hdr"
    };

    AssimpModel Model("plane/scene.gltf");


    //stbi_set_flip_vertically_on_load(true);
    //AssimpModel BackPack("backpack/backpack.obj");
    //stbi_set_flip_vertically_on_load(false);

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

    float QuadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    float framebufferquadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    float cubeVertices[] = {
        // positions          // normals
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };


    float planeVertices[] = {
        // positions            // normals         // texcoords
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,

         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
    };

    int maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    int maxCompteWorkGroupx = 0, maxCompteWorkGroupy = 0, maxCompteWorkGroupz = 0;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxCompteWorkGroupx);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxCompteWorkGroupy);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxCompteWorkGroupz);
    std::cout << maxSamples << "\n";
    std::cout << maxCompteWorkGroupx << "\n";
    std::cout << maxCompteWorkGroupy << "\n";
    std::cout << maxCompteWorkGroupz << "\n";
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
    glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
    glm::vec3 pos3(1.0f, -1.0f, 0.0f);
    glm::vec3 pos4(1.0f, 1.0f, 0.0f);
    // texture coordinates
    glm::vec2 uv1(0.0f, 1.0f);
    glm::vec2 uv2(0.0f, 0.0f);
    glm::vec2 uv3(1.0f, 0.0f);
    glm::vec2 uv4(1.0f, 1.0f);
    // normal vector
    glm::vec3 nm(0.0f, 0.0f, 1.0f);

    // calculate tangent/bitangent vectors of both triangles
    glm::vec3 tangent1{}, bitangent1{};
    glm::vec3 tangent2{}, bitangent2{};
    // triangle 1
    // ----------
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    // triangle 2
    // ----------
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


    bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


    float quadVertices[] = {
        // positions            // normal         // texcoords  // tangent                          // bitangent
        pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

        pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
    };
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    // configure plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glBindVertexArray(0);

    //glBindVertexArray(quadVAO);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //texture
    
    Texture floortexture;
    floortexture.LoadTexture("wood.png");
    floortexture.GenerateTexture(0, GL_REPEAT, true);
    floortexture.Bind(0);
    //
    Texture BrickTexture;
    BrickTexture.LoadTexture("brickwall.jpg");
    BrickTexture.GenerateTexture(0, GL_REPEAT, true);
    Texture NormalTexture;
    NormalTexture.LoadTexture("brickwall_normal.jpg");
    NormalTexture.GenerateTexture(3, GL_REPEAT, false);
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 2048, 2048);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    unsigned int skybox2vao, skybox2vbo;
    glGenVertexArrays(1, &skybox2vao);
    glGenBuffers(1, &skybox2vbo);

    glBindVertexArray(skybox2vao);

    glBindBuffer(GL_ARRAY_BUFFER, skybox2vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glBindVertexArray(skybox2vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int Boxvao, boxvbo;
    glGenVertexArrays(1, &Boxvao);
    glGenBuffers(1, &boxvbo);

    glBindVertexArray(Boxvao);

    glBindBuffer(GL_ARRAY_BUFFER, boxvbo);
    glBufferData(GL_ARRAY_BUFFER, Cubevertices.size() * sizeof(Vertex), Cubevertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));

    //unsigned int skyboxtexture = loadCubemap(faces);

    Texture hdrskyboxtex;
    hdrskyboxtex.loadHDREquiRectangularMap(hdrface);

    Texture Brick2normalMap;
    Brick2normalMap.LoadTexture("toy_box_normal.png");
    Brick2normalMap.GenerateTexture(3, GL_REPEAT, false);
    Texture Parallaxmap;
    Parallaxmap.LoadTexture("toy_box_disp.png");
    Parallaxmap.GenerateTexture(4, GL_REPEAT, false);
    Texture WoodToy;
    WoodToy.LoadTexture("woodtoy.png");
    WoodToy.GenerateTexture(0, GL_REPEAT, true);
    unsigned int uniformbuffer;
    glGenBuffers(1, &uniformbuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, uniformbuffer);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformbuffer, 0, 2 * sizeof(glm::mat4));

    // Our feamebuffer quad
    unsigned int QuadVAO, QuadVBO;
    glGenVertexArrays(1, &QuadVAO);
    glGenBuffers(1, &QuadVBO);
    glBindVertexArray(QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), &QuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // default opengl depth & stencil and blending global values
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // set depth function to less than AND equal for skybox depth trick.

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_NORMALIZE);

    glEnable(GL_MULTISAMPLE);

    //glEnable(GL_FRAMEBUFFER_SRGB);
    int samples = 8;
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA16F, Width, Height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, Width, Height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! MultiSampled \n" << fboStatus;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Width, Height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, screenTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render loop
    // -----------
    double xpos, ypos;
    int glfwwidth = 0, glfwheight = 0;

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB_ALPHA,
            2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    hdrskybox.use();
    hdrskybox.setInt("equirectangularMap", 0);
    hdrskybox.setMat4("projection", captureProjection);
    hdrskyboxtex.Bind(0);
    glViewport(0, 0, 2048, 2048); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    for (unsigned int i = 0; i < 6; ++i)
    {
        hdrskybox.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glBindVertexArray(skybox2vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Framebuffer.use();
    Framebuffer.setInt("text_diffuse1", 0);

    glViewport(0, 0, Width, Height);
    // configure depth map FBO
// -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthCubeMap;
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);        
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const int NR_LIGHTS = 32;
    std::vector<Cluster> cluster;
    cluster.push_back({ glm::vec4(1.0), glm::vec4(1.0), NR_LIGHTS, NULL });
    unsigned int SSBOCluster;
    glGenBuffers(1, &SSBOCluster);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOCluster);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cluster.size() * sizeof(Cluster), NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBOCluster);

    glm::vec3 lightPos(0.0f, 3.0f, 1.0f);
    
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(13);
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
        float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
    }

    std::vector<PointLight> pointlight;
    float constant = 4.0;
    float linear = 1.0f;
    float quadratic = 1.0f;
    for (int i = 0; i < NR_LIGHTS; i++) {
    float lightMax = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
    float radius =
        (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 3.0) * lightMax)))
        / (2 * quadratic);
    pointlight.push_back({ glm::vec4(lightPositions[i], 1.0f), glm::vec4(lightColors[i], 1.0f), constant, linear, quadratic, radius });
    }
    unsigned int SSBOPointLight;
    glGenBuffers(1, &SSBOPointLight);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPointLight);
    glBufferData(GL_SHADER_STORAGE_BUFFER, pointlight.size() * sizeof(PointLight), NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, SSBOPointLight);

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    float near_plane = 0.1f;
    float far_plane = 100.0f;
    //glm::vec3 lightPos(0.0f, 4.0f, -1.0f);

    ourShader.use();
    ourShader.setInt("texture_diffuse1", 0);
    ourShader.setInt("texture_specular1", 1);
    ourShader.setInt("depthMap", 2);
    ourShader.setInt("normalMap", 3);
    ourShader.setInt("ParallaxMap", 4);

    BlurShader.use();
    BlurShader.setInt("image", 0);

    glm::mat4 shadowProjection = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.reserve(6);
    shadowTransforms.emplace_back(shadowProjection *
        glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.emplace_back(shadowProjection *
        glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.emplace_back(shadowProjection *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.emplace_back(shadowProjection *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms.emplace_back(shadowProjection *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.emplace_back(shadowProjection *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

    shadowTransforms.shrink_to_fit();
    // Setup Platform/Renderer backends
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

            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            // create a multisampled color attachment texture
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA16F, Width, Height, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, Width, Height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
            glBindTexture(GL_TEXTURE_2D, screenTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Width, Height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // attach texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

        }
        lightingengine.use();
        unsigned int SSBOBlock = glGetUniformBlockIndex(lightingengine.ID, "clusterSSBO");
        glUniformBlockBinding(lightingengine.ID, SSBOBlock, 1);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOCluster);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, cluster.size() * sizeof(Cluster), cluster.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        

        lightingengine.setFloat("zNear", 0.1f);
        lightingengine.setFloat("zFar", 100.0f);
        lightingengine.setMat4("inverseProjection", glm::inverse(projection));
        lightingengine.setUVec3("gridSize", glm::uvec3(12.0, 12.0, 24.0));
        lightingengine.setUVec2("screenDimensions", Width, Height);
        
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwGetCursorPos(window, &xpos, &ypos);
        camera.ProcessKeyboard(window, deltaTime);

        camera.ProcessMouseMovement(window, xpos, ypos);
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        SimpleShader.use();
        for (unsigned int i = 0; i < 6; ++i) {
            SimpleShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        //SimpleShader.setVec3("lightPos", lightPos);
        // 
        //for (int i = 0; i < NR_LIGHTS; i++) {
        SimpleShader.setVec3("lightPos", glm::vec3(pointlight[0].position));
        
        //}

        SimpleShader.setInt("NR_Light", NR_LIGHTS);
        SimpleShader.setFloat("far_plane", far_plane);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        SimpleShader.setMat4("model", model);
        Model.Draw(SimpleShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::rotate(model, glm::radians(-90.0f), glm::normalize(glm::vec3(1.0, 0.0, 0.0))); // rotate the quad to show normal mapping from multiple directions
        model = glm::scale(model, glm::vec3(100.0f));	// it's a bit too big for our scene, so scale it down
        SimpleShader.setMat4("model", model);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(0.0, 1.0,0.0))); // rotate the quad to show normal mapping from multiple directions
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        SimpleShader.setMat4("model", model);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
        SimpleShader.use();
        SimpleShader.setMat4("model", model);
        glBindVertexArray(Boxvao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, Width, Height);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        view = camera.GetViewMatrix();

        CullEngine.use();
        unsigned int SSBOCULLBLOCK = glGetUniformBlockIndex(CullEngine.ID, "clusterSSBO");
        glUniformBlockBinding(CullEngine.ID, SSBOCULLBLOCK, 1);
        unsigned int SSBOPoint = glGetUniformBlockIndex(CullEngine.ID, "lightSSBO");
        glUniformBlockBinding(CullEngine.ID, SSBOPoint, 2);
        CullEngine.setMat4("viewMatrix", view);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOPointLight);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, pointlight.size() * sizeof(PointLight), pointlight.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glDispatchCompute(27, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        ourShader.use();
        unsigned int OurSSBOCULLBLOCK = glGetUniformBlockIndex(ourShader.ID, "clusterSSBO");
        glUniformBlockBinding(ourShader.ID, OurSSBOCULLBLOCK, 1);

        unsigned int OurSSBOLightBLOCK = glGetUniformBlockIndex(ourShader.ID, "lightSSBO");
        glUniformBlockBinding(ourShader.ID, OurSSBOLightBLOCK, 2);

        ourShader.setFloat("zNear", 0.1f);
        ourShader.setFloat("zFar", 100.0f);
        ourShader.setUVec3("gridSize", glm::uvec3(12.0, 12.0, 24.0));
        ourShader.setUVec2("screenDimensions", Width, Height);

        ourShader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setFloat("far_plane", far_plane);
        ourShader.setVec3("lightColor", glm::vec3(500.0f));
        //lightPos = glm::vec3(0.0f, 4.0f, 0.0f);

        for (int i = 0; i < 1; i++) {
            ourShader.setVec3(("light[" + std::to_string(i) + "].ambient"), glm::vec3(5.0f));
            ourShader.setVec3("light[" + std::to_string(i) + "].position", lightPos);
            ourShader.setVec3("light[" + std::to_string(i) + "].diffuse", glm::vec3(5.0f));
            ourShader.setVec3("light[" + std::to_string(i) + "].specular", glm::vec3(0.3f));
            ourShader.setVec3("light[" + std::to_string(i) + "].direction", glm::vec3(-0.2f, -1.0f, -0.3f));
            ourShader.setFloat("light[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(12.5f)));
            ourShader.setFloat("light[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(17.5f)));

            ourShader.setFloat("light[" + std::to_string(i) + "].constant", 1.0f);
            ourShader.setFloat("light[" + std::to_string(i) + "].linear", 0.09f);
            ourShader.setFloat("light[" + std::to_string(i) + "].quadratic", 0.032f);
        }
        glUniformBlockBinding(ourShader.ID, SSBOPoint, 2);
        ourShader.setFloat("time", static_cast<float>(glfwGetTime()));
        ourShader.setBool("normalMapON", false);
        //material specfication;
        ourShader.setVec3("material.ambient", glm::vec3(0.0f));
        ourShader.setFloat("material.shininess", 64.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)Width / (float)Height, 0.1f, 100.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformbuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBuffer(GL_UNIFORM_BUFFER, uniformbuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));	// it's a bit too big for our scene, so scale it down

        ourShader.setMat4("model", model);

        Model.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(100.0f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(-90.0f), glm::normalize(glm::vec3(1.0, 0.0, 0.0))); // rotate the quad to show normal mapping from multiple directions

        ourShader.use();
        ourShader.setBool("normalMapON", true);
        ourShader.setMat4("model", model);
        glBindVertexArray(quadVAO);
        BrickTexture.Bind(0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        NormalTexture.Bind(3);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        {
            if (heightScale > 0.0f)
                heightScale -= 0.0005f;
            else
                heightScale = 0.0f;
        }
        else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        {
            if (heightScale < 1.0f)
                heightScale += 0.0005f;
            else
                heightScale = 1.0f;
        }
        ourShader.use();
        ourShader.setBool("parallaxmappingEnabled", true);
        ourShader.setBool("normalMapON", true);
        ourShader.setFloat("heightScale", heightScale);
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            ourShader.setBool("parallaxmappingEnabled", false);
        }
        glBindVertexArray(quadVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0))); // rotate the quad to show normal mapping from multiple directions
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        WoodToy.Bind(0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        Brick2normalMap.Bind(3);
        Parallaxmap.Bind(4);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ourShader.setBool("normalMapON", false);
        ourShader.setBool("parallaxmappingEnabled", false);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
        ourShader.use();
        ourShader.setMat4("model", model);
        glBindVertexArray(Boxvao);
        floortexture.Bind(0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        LightShader.use();
        LightShader.setMat4("view", view);
        LightShader.setMat4("projection", projection);
        LightShader.setMat4("model", model);
        LightShader.setVec4("LightColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        glBindVertexArray(Boxvao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // convert HDR equirectangular environment map to cubemap equivalent
        glDepthFunc(GL_LEQUAL);
        hdrskybox.use();
        hdrskybox.setMat4("projection", projection);
        hdrskyboxtex.Bind(0);
        
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        hdrskybox.setMat4("view", view);
        glBindVertexArray(skybox2vao);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);        

        glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);       
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        Framebuffer.use();

        Framebuffer.setBool("gamma", true);
        Framebuffer.setBool("hdr", true);

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        {
        Framebuffer.setBool("hdr", false);

        }
        Framebuffer.setFloat("exposure", 1.0f);
        Framebuffer.setFloat("gamma1", 2.2f);
        glBindVertexArray(QuadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTexture); // use the now resolved color attachment as the quad's texture
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

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

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
