#define TINYOBJLOADER_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <src/tiny_obj_loader.h>
#include <iostream>
#include <vector>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

static const GLuint WIDTH = 512, HEIGHT = 512;
/* vertex data is passed as input to this shader
 * ourColor is passed as input to the to the fragment shader. */
static const GLchar* vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoords;\n"
    "out vec2 TexCoords;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "    TexCoords = aTexCoords;\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";
static const GLchar* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoords;\n"
    "uniform sampler2D texture1;\n"
    "void main() {\n"
    "    FragColor = texture(texture1, TexCoords);\n"
    "}\n";

int main(void)
{
    // Initialisation de GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Création de la fenêtre
    GLFWwindow* window = glfwCreateWindow(800, 600, "TinyOBJ Loader + GLFW", NULL, NULL);
    if (!window) {
        std::cout << "Erreur lors de la création de la fenêtre GLFW." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialisation de Glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Erreur lors de l'initialisation de Glad." << std::endl;
        return -1;
    }

    // Chargement du modèle avec TinyOBJ Loader
    tinyobj::attrib_t                attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;
    std::string                      warn;
    std::string                      err;
    bool                             success=tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "cornell_box.obj");
    
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return -1;
    }
    if (!success) {
        std::cerr << "Erreur lors du chargement du modèle." << std::endl;
        return -1;
    }

     // Liaison du shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    float fov    = 45.0f;
    int   width  = 800;
    int   height = 600;

   for (size_t s = 0; s < shapes.size(); s++) {
        /*-----------------------------------------------------------*/

        // std::vector<float> buffer; // pos(3float), normal(3float), color(3float)
        // I replace "buffer" by arrays:
        std::vector<GLfloat> mesh_vertex;
        std::vector<GLfloat> mesh_normals;
        std::vector<GLfloat> mesh_colors;
        std::vector<GLfloat> mesh_textCoords;
        std::vector<GLuint>  mesh_indices;
        

        /*fill index array*/
        for (long i = 0; i < shapes[s].mesh.indices.size(); i++) {
            mesh_indices.push_back(shapes[s].mesh.indices[i].vertex_index);
        }

        // Boucle de rendu
        while (!glfwWindowShouldClose(window)) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Transformation du modèle
            glm::mat4 model       = glm::mat4(1.0f);
            model                 = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Vue et projection

            glm::mat4    view    = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

            glm::mat4    projection    = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);
            unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

            // Liaison de la texture
            // glBindTexture(GL_TEXTURE_2D, textureID);

            // Dessin du modèle
            /* for (int s = 0; s < mesh_indices.size(); s++) {
                
            }*/

            glBindVertexArray(mesh_indices[s]);
            glDrawElements(GL_TRIANGLES, mesh_indices.size()/3, GL_UNSIGNED_INT, 0); // mesh.IndexCount
            glBindVertexArray(0);

            // Échange des buffers
            glfwSwapBuffers(window);

            // Traitement des événements
            glfwPollEvents();
        }
    }

    // Nettoyage de GLFW
    glfwTerminate();
    return 0;
}
