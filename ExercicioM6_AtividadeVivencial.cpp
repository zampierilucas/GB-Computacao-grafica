// Bibliotecas externas.
#include <assert.h>
#include <fstream>
#include "stb_image.h"
#include "gl_utils.h"
#include <iostream>
#include "ltMath.h"
#include <math.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ExercicioM6_DiamondView.h"
#include "ExercicioM6_TileMap.h"
#include "ExercicioM6_TilemapView.h";

// Namespace.
using namespace std;

// Constantes.
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const int TILESET_COLS = 9, TILESET_ROWS = 9;

// Variáveis Globais.
bool mouseEnabled = true;
TilemapView* tview = new DiamondView();

// Função para ler um tilemap a partir de um arquivo externo com o mapeamento de tiles.
TileMap* readMap(char* filename) {
    ifstream arq(filename);

    int w, h;
    arq >> w >> h;
    TileMap* tmap = new TileMap(w, h, 0);

    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            int tid;
            arq >> tid;
            tmap->setTile(c, h - r - 1, tid);
        }
    }

    arq.close();
    return tmap;
}

// Função para carregar a imagem (textura).
void loadTexture(const char* file_name, GLuint* texture) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_TEXTURE_2D);

    GLfloat max_aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

    int width, height, nChannels;
    unsigned char* image_data = stbi_load(file_name, &width, &height, &nChannels, 0);
    if (image_data) {
        if (nChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(image_data);
}

// Função para calcular e obter o comprimento e altura dos tiles.
void getTileWidthAndTileHeight(TileMap* tmap, float* tileWidth, float* tileHeight) {
    *tileWidth = WINDOW_WIDTH / tmap->getWidth();
    *tileHeight = *tileWidth / 2;
}

// Função para obter o tile pertencente a posição pressionada (mouse).
void getTileClickedPosition(TileMap* tmap, double& mx, double& my, int* collidedX, int* collidedY) {
    // 1) Obtem as dimensoes dos tiles.
    float tileWidth = 0;
    float tileHeight = 0;
    getTileWidthAndTileHeight(tmap, &tileWidth, &tileHeight);

    // 2) Obtem a coluna e linha do clique do mouse.
    int c, r;
    tview->computeMouseMap(c, r, WINDOW_HEIGHT, tileWidth, tileHeight, mx, my);
    c += (tmap->getWidth() - 1) / WINDOW_WIDTH;
    r += (tmap->getHeight() - 1) / WINDOW_HEIGHT;

    // 3) Verifica se a coluna e linha encontrada pertencem ao tilemap.
    if ((c < 0) || (c >= tmap->getWidth()) || (r < 0) || (r >= tmap->getHeight())) {
        return;
    }   

    *collidedX = c;
    *collidedY = r;
}

// Função para controlar redimensionamento da janela.
void resizeWindow(GLFWwindow* window) {
    int screenWidth, screenHeight;
    glfwGetWindowSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
}

// Função para verificar pressionamento de tecla de escape (ESC).
void verifyIfEscapeKeyWasPressed(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// Função para verificar se o mouse foi pressionado e executar ação.
void verifyAndProcessMouseClick(GLFWwindow* window, TileMap* tmap, int* collidedX, int* collidedY) {
    const int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS && mouseEnabled) {
        mouseEnabled = false;

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        getTileClickedPosition(tmap, mx, my, &*collidedX, &*collidedY);
    }
    else if (state != GLFW_PRESS)
    {
        mouseEnabled = true;
    }
}

// Função principal.
int main() {
    // Inicializando GLFW.
    if (glfwInit() == GLFW_FALSE)
    {
        fprintf(stderr, "Erro ao iniciar GLFW.");
        return EXIT_FAILURE;
    }

    // Necessário para MAC.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Definindo Window.
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Titulo da Janela", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Erro ao iniciar Janela.");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Seta a janela no contexto atual do GLFW.
    glfwMakeContextCurrent(window);

    // Inicializando GLEW.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Erro ao iniciar GLEW.");
        return EXIT_FAILURE;
    }

    // Definindo os Shaders.
    const char* vertex_shader =
        "#version 410\n"

        "layout(location=0) in vec2 vertex_position;"
        "layout(location=1) in vec2 texture_mapping;"

        "out vec2 texture_coords;"

        "uniform float tx;"
        "uniform float ty;"
        "uniform float layer_z;"
        "uniform mat4 proj;"
        
        "void main() {"
        "   texture_coords = texture_mapping;"
        "	gl_Position = proj * vec4(vertex_position.x + tx, vertex_position.y + ty, layer_z, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 410\n"

        "in vec2 texture_coords;"

        "uniform sampler2D sprite;"
        "uniform float offsetx;"
        "uniform float offsety;"
        "uniform float weight;"

        "out vec4 frag_color;"

        "void main () {"
        "    vec4 texel = mix(texture(sprite, vec2(texture_coords.x + offsetx, texture_coords.y + offsety)), vec4(0,0,1,1), weight);"
        "    if(texel.a < 0.5) {"
        "        discard;"
        "    }"
        "    frag_color = texel;"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_programme = glCreateProgram();
    glAttachShader(shader_programme, fs);
    glAttachShader(shader_programme, vs);
    glLinkProgram(shader_programme);

    // Carregando a imagem como textura.
    GLuint tid;
    loadTexture("images/terrain.png", &tid);

    // Inicializando o Tilemap Base (mapeamento dos tiles de chão).
    TileMap* tmap = readMap("ExercicioM6_Tilemap.tmap");
    tmap->setTid(tid);

    // Inicializando o Tilemap Base (mapeamento dos tiles onde o usuário pode andar).
    TileMap* tmapBase = readMap("ExercicioM6_TilemapBase.tmap");
    tmapBase->setTid(0);

    // Obtendo tamanho e altura dos tiles.
    float tileWidth = 0;
    float tileHeight = 0;
    getTileWidthAndTileHeight(tmap, &tileWidth, &tileHeight);

    // Obtendo valores para mapeamento da textura.
    float tileW = 1.0f / (float)TILESET_COLS;
    float tileW2 = tileW / 2.0f;
    float tileH = 1.0f / (float)TILESET_ROWS;
    float tileH2 = tileH / 2.0f;

    // Definindo os vertices.
    GLfloat vertices[] = {
        // positions                 // texture coords
                  0, tileHeight/2,   0.0f, tileH2,
        tileWidth/2,            0,   tileW2, 0.0f,
        tileWidth/2,   tileHeight,   tileW2, tileH,
                                     
          tileWidth, tileHeight/2,   tileW, tileH2,
        tileWidth/2,            0,   tileW2, 0.0f,
        tileWidth/2,   tileHeight,   tileW2, tileH,
    };

    // Projeção Ortho para facilitar trabalho com window e inverter eixo y.
    glm::mat4 proj = glm::ortho(0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 0.0f, -1.0f, 1.0f);

    // Definindo VAO e VBO
    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Layout(0) - vértices; tamanho de cada item 2; pula de 4 em 4; pega a partir do index 0;
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Layout(1) - cordenadas da textura; tamanho de cada item 2; pula de 4 em 4; pega a partir do index 2;
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Desfazendo bind para liberação de memória.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Habilita o controle de profundidade.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Habilita o controle de canal alpha (transparencia).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Variáveis para detecção de colisão no clique do mouse.
    int collidedX = -1, collidedY = -1;

    // Controle da Janela e Desenho
    while (!glfwWindowShouldClose(window)) {
        // Limpa a Janela.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Verifica se tecla de escape (ESC) foi pressionada para fechar a janela.
        verifyIfEscapeKeyWasPressed(window);

        // Trata redimensionamento da janela.
        resizeWindow(window);

        // Verifica e trara clique do mouse.
        verifyAndProcessMouseClick(window, tmap, &collidedX, &collidedY);

        // Seta o programa do shader.
        glUseProgram(shader_programme);

        // Realiza o bing com o VAO.
        glBindVertexArray(VAO);

        // Passa a variável global do shader (uniform) chamada 'proj' ao shader.
        glUniformMatrix4fv(glGetUniformLocation(shader_programme, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

        // Itera sobre os tiles.
        float x, y;
        int r = 0, c = 0;
        for (int r = 0; r < tmap->getHeight(); r++) {
            for (int c = 0; c < tmap->getWidth(); c++) {
                int t_id = (int)tmap->getTile(c, r);
                int u = t_id % TILESET_COLS;
                int v = t_id / TILESET_COLS;

                tview->computeDrawPosition(c, r, tileWidth, tileHeight, x, y);

                // Passa variável global via uniform para o shader.
                glUniform1f(glGetUniformLocation(shader_programme, "tx"), x);
                glUniform1f(glGetUniformLocation(shader_programme, "ty"), y + ((WINDOW_HEIGHT / 2) - (tileHeight / 2)));
                glUniform1f(glGetUniformLocation(shader_programme, "layer_z"), tmap->getZ());
                glUniform1f(glGetUniformLocation(shader_programme, "offsetx"), u * tileW);
                glUniform1f(glGetUniformLocation(shader_programme, "offsety"), v * tileH);                
                glUniform1f(glGetUniformLocation(shader_programme, "weight"), (c == collidedX) && (r == collidedY) ? 0.5 : 0.0);

                // Realiza o bind da textura.
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tmap->getTileSet());
                
                // Passa variável global via uniform para o shader.
                glUniform1i(glGetUniformLocation(shader_programme, "sprite"), 0);

                // Desenha o triângulo/retângulo.
                glDrawArrays(GL_TRIANGLES, 0, (sizeof(vertices) / sizeof(*vertices)) / 4); // vertices.length / 4 floats para cada vértice.
            }
        }

        // Desfazendo bind para liberação de memória.
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Processa todos os eventos pendentes.
        glfwPollEvents();

        // Altera o buffer para que a janela seja atualizada de acordo com o Buffer pelo OpenGL.
        glfwSwapBuffers(window);
    }

    // Desaloca memória deletando o VAO e VBO.
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // Deleta os tilemaps em memória.
    delete tmap;
    delete tmapBase;

    // Finaliza GLFW.
    glfwTerminate();

    // Finaliza a execução com sucesso.
    return EXIT_SUCCESS;
}