/**
 * ============================================================================
 * Computacao Grafica e Jogos - 2025/2026
 * Assignment 2: 2D Tangram in OpenGL (Figure #100: WOLF - Pickagram)
 * 
 * Requisitos cumpridos:
 * 1. Apenas 3 formas base em buffers (VAO/VBO):
 *    - Triangulo retangulo isosceles unitario (partilhado pelos 5 triangulos).
 *    - Quadrado unitario (composto por 2 triangulos).
 *    - Paralelogramo unitario tipo 3a (composto por 2 triangulos).
 * 2. Ordem de vertices estritamente CCW (Counter-Clockwise / Right-hand rule).
 * 3. Transformacoes de modelo (T * R * S) passadas por uniform no Vertex Shader.
 * 4. Desenhado diretamente em Clip Space (z = 0, x, y em [-1, 1]).
 * 5. Classe de abstracao TangramPiece:
 *    - A ordem de desenho das pecas e independente (pressione ESPACO para baralhar).
 * 6. Cores distintas para cada uma das 7 pecas.
 * =============================================================================
 * DECLARAÇÃO DE HONRA 
 * =============================================================================
 *  "Nós, abaixo assinados,
 *  Mikhail Mohamed Shueb, n.º 104014, curso MEIC-A
 *  Tomás Martins Ferreira, n.º 109881, curso MEIC-A
 *  declaramos, sob compromisso de honra, que o trabalho intitulado "Assignment 2 - 2D Tangram (#100 WOLF)", 
 *  realizado no âmbito da unidade curricular Computacao Grafica para Jogos,
 *  resultou da participação ativa dos seguintes elementos do grupo. Os contributos individuais foram:
 *  Mikhail Shueb – Criação da iteração inicial e funcional do codigo;
 *  Tómás Ferreira – Correção de bugs, ajustes de transformações e refinamento do código.
 *  Declaramos que o trabalho é original e respeita as normas académicas em vigor.
 *  Campus Taguspark IST, 17/11/2026
 *  Assinaturas: Mikhail Shueb & Tomás Ferreira"
 * =============================================================================
 * DECLARAÇÃO DE USO DE INTELIGÊNCIA ARTIFICIAL (Nível 1 - Uso Limitado)
 * =============================================================================
 *  "Todas as decisões e conteúdo técnico deste trabalho são da nossa autoria.
 *   Foram utilizadas ferramentas de IA generativa (Google Antigravity / Gemini)
 *   para auxílio na extração e verificação trigonométrica das coordenadas da
 *   figura #100 (Pickagram)-Wolf, apoio na configuração das bibliotecas e resolução
 *   do problema de aspect ratio com viewport letterboxing no OpenGL."
 * =============================================================================
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <cmath>

// ============================================================================
// Shaders GLSL (OpenGL 3.3 Core)
// ============================================================================

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 u_Model;

void main() {
    // Aplica a matriz de transformacao de modelo (T * R * S) diretamente em clip space
    gl_Position = u_Model * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

uniform vec4 u_Color;

void main() {
    FragColor = u_Color;
}
)";

// Funcao auxiliar para compilar e verificar shaders
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "[Erro] Falha ao compilar shader ("
                  << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                  << "): " << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram(const char* vsSource, const char* fsSource) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "[Erro] Falha ao linkar shader program: " << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

// ============================================================================
// Classe de Abstracao da Peca de Tangram
// ============================================================================
// Permite que cada peca mantenha a sua propria transformacao de modelo (M = T * R * S),
// a sua cor unica e a referencia ao seu buffer de geometria base (VAO).
// Como todas as pecas sao geometricamente disjuntas na figura e usam as suas
// proprias matrizes, a ordem de desenho nao afeta o resultado visual final.
class TangramPiece {
public:
    std::string name;
    GLuint vao;
    GLsizei vertexCount;
    glm::vec4 color;
    glm::mat4 modelMatrix;

    TangramPiece(const std::string& name, GLuint vao, GLsizei vertexCount,
                 const glm::vec4& color, const glm::mat4& modelMatrix)
        : name(name), vao(vao), vertexCount(vertexCount),
          color(color), modelMatrix(modelMatrix) {}

    void draw(GLint modelLoc, GLint colorLoc) const {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform4fv(colorLoc, 1, glm::value_ptr(color));
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
};

// ============================================================================
// Callbacks e Variaveis Globais de Estado
// ============================================================================
bool wireframeMode = false;
std::vector<TangramPiece> tangramPieces;
std::vector<TangramPiece> originalPieces;

// Funcao para manter a proporcao 1:1 quadrada (Letterbox / Pillarbox)
// Garante que o desenho em Clip Space [-1, 1] nao sofre qualquer distorcao
// independentemente das dimensoes ou formato da janela.
void updateViewport(int width, int height) {
    if (height <= 0) height = 1;
    if (width <= 0) width = 1;

    int size = std::min(width, height);
    int offsetX = (width - size) / 2;
    int offsetY = (height - size) / 2;

    glViewport(offsetX, offsetY, size, size);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    updateViewport(width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(window, true);
        }
        else if (key == GLFW_KEY_W) {
            wireframeMode = !wireframeMode;
            if (wireframeMode) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                std::cout << "[Modo] Wireframe ativado (linhas).\n";
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                std::cout << "[Modo] Preenchimento solido ativado.\n";
            }
        }
        else if (key == GLFW_KEY_SPACE) {
            // Demonstra a independencia da ordem de desenho baralhando as pecas
            static std::random_device rd;
            static std::mt19937 g(rd());
            std::shuffle(tangramPieces.begin(), tangramPieces.end(), g);
            std::cout << "[Ordem] Pecas baralhadas! Nova ordem de desenho: ";
            for (size_t i = 0; i < tangramPieces.size(); ++i) {
                std::cout << tangramPieces[i].name << (i + 1 < tangramPieces.size() ? " -> " : "\n");
            }
        }
        else if (key == GLFW_KEY_R) {
            tangramPieces = originalPieces;
            std::cout << "[Ordem] Ordem de desenho reposta ao normal.\n";
        }
    }
}

// ============================================================================
// Funcao Principal
// ============================================================================
int main() {
    // 1. Inicializacao do GLFW
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // Anti-aliasing 4x MSAA

    GLFWwindow* window = glfwCreateWindow(800, 800, "CGJ - Assignment 2: 2D Tangram (#100 WOLF)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Inicializa o viewport com aspect ratio 1:1 centrado
    int initialWidth, initialHeight;
    glfwGetFramebufferSize(window, &initialWidth, &initialHeight);
    updateViewport(initialWidth, initialHeight);

    // 2. Inicializacao do GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Falha ao inicializar GLEW: " << glewGetErrorString(err) << "\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL Versao: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer:      " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "--------------------------------------------------------\n";
    std::cout << "Controlos:\n";
    std::cout << "  [ESPACO] : Baralhar ordem de desenho das pecas (order-independence)\n";
    std::cout << "  [W]      : Alternar modo Wireframe / Solido\n";
    std::cout << "  [R]      : Repor ordem inicial das pecas\n";
    std::cout << "  [ESC / Q]: Sair da aplicacao\n";
    std::cout << "--------------------------------------------------------\n";

    // 3. Criacao do Shader Program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    GLint modelLoc = glGetUniformLocation(shaderProgram, "u_Model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "u_Color");

    // ========================================================================
    // 4. Os 3 Buffers Base de Geometria (VAO / VBO)
    // ========================================================================
    // De acordo com as instrucoes do assignment:
    // "define only three base shapes as buffer objects (VAO / VBO):
    //  - a unit right isosceles triangle (shared by all five triangles)
    //  - a unit square (formed by two triangles)
    //  - a unit parallelogram (formed by two triangles, side 3a)"
    //
    // Todos os triangulos sao especificados estritamente em ordem CCW.

    // Forma 1: Triangulo Retangulo Isosceles Unitario
    // Vertices em CCW: (0,0) -> (1,0) -> (0,1)
    float triangleVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f
    };

    // Forma 2: Quadrado Unitario (2 triangulos em CCW)
    // Triangulo 1: (0,0) -> (1,0) -> (1,1)
    // Triangulo 2: (0,0) -> (1,1) -> (0,1)
    float squareVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    // Forma 3: Paralelogramo Unitario (Tipo 3a, 2 triangulos em CCW)
    // Base unitaria de comprimento sqrt(2), lado inclinado a 45 graus de comprimento 1.0
    // Triangulo 1: (0,0) -> (sqrt(2), 0) -> (3*sqrt(2)/2, sqrt(2)/2)
    // Triangulo 2: (0,0) -> (3*sqrt(2)/2, sqrt(2)/2) -> (sqrt(2)/2, sqrt(2)/2)
    const float SQRT2 = 1.41421356f;
    const float HALF_SQRT2 = SQRT2 * 0.5f;
    float parallelogramVertices[] = {
        0.0f, 0.0f,
        SQRT2, 0.0f,
        SQRT2 + HALF_SQRT2, HALF_SQRT2,

        0.0f, 0.0f,
        SQRT2 + HALF_SQRT2, HALF_SQRT2,
        HALF_SQRT2, HALF_SQRT2
    };

    // Criacao dos VAOs e VBOs
    GLuint vaoTriangle, vboTriangle;
    glGenVertexArrays(1, &vaoTriangle);
    glGenBuffers(1, &vboTriangle);
    glBindVertexArray(vaoTriangle);
    glBindBuffer(GL_ARRAY_BUFFER, vboTriangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint vaoSquare, vboSquare;
    glGenVertexArrays(1, &vaoSquare);
    glGenBuffers(1, &vboSquare);
    glBindVertexArray(vaoSquare);
    glBindBuffer(GL_ARRAY_BUFFER, vboSquare);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint vaoParallelogram, vboParallelogram;
    glGenVertexArrays(1, &vaoParallelogram);
    glGenBuffers(1, &vboParallelogram);
    glBindVertexArray(vaoParallelogram);
    glBindBuffer(GL_ARRAY_BUFFER, vboParallelogram);
    glBufferData(GL_ARRAY_BUFFER, sizeof(parallelogramVertices), parallelogramVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // ========================================================================
    // 5. Parametros e Matrizes das 7 Pecas do Lobo (#100 WOLF)
    // ========================================================================
    // Fator de escala e centralizacao para caber perfeitamente no Clip Space [-1, 1]
    const float sceneScale = 0.0105f;
    const float cx = 95.9545f;
    const float cy = 203.7530f;
    const float u = 22.8064f;
    const float su = u * sceneScale; // Escala unitaria em Clip Space (~0.2395)

    auto toClip = [&](float px, float py) -> glm::vec2 {
        return glm::vec2((px - cx) * sceneScale, (py - cy) * sceneScale);
    };

    // Funcao auxiliar para construir M = T * R * S
    auto makeModelMatrix = [](glm::vec2 t, float angleDeg, glm::vec2 s) -> glm::mat4 {
        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(t.x, t.y, 0.0f));
        m = glm::rotate(m, glm::radians(angleDeg), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(s.x, s.y, 1.0f));
        return m;
    };

    // Definicao das 7 pecas com transformacoes exatas obtidas da geometria de Pickagram:
    // Peca 1: Triangulo Grande 1 (Peito / Pescoco)
    // Catetos = 2.0 * su. Origem no angulo reto (58.634, 195.602)
    glm::mat4 mPiece1 = makeModelMatrix(toClip(58.6339f, 195.602f), -14.6388f, glm::vec2(2.0f * su));

    // Peca 2: Triangulo Grande 2 (Corpo / Pata Traseira)
    // Catetos = 2.0 * su. Origem no angulo reto (130.598, 200.377)
    glm::mat4 mPiece2 = makeModelMatrix(toClip(130.598f, 200.377f), 165.361f, glm::vec2(2.0f * su));

    // Peca 3a: Paralelogramo (Cauda)
    // Base = su. Origem no vertice inferior (114.994, 204.453)
    glm::mat4 mPiece3a = makeModelMatrix(toClip(114.994f, 204.453f), -14.6388f, glm::vec2(1.0f * su));

    // Peca 4: Triangulo Pequeno 1 (Garganta / Mandibula)
    // Catetos = 1.0 * su. Origem no angulo reto (64.398, 217.668)
    glm::mat4 mPiece4 = makeModelMatrix(toClip(64.3977f, 217.6685f), 165.361f, glm::vec2(1.0f * su));

    // Peca 5: Quadrado (Coroa / Cabeca)
    // Lados = 1.0 * su. Origem no vertice inferior (64.398, 217.668)
    glm::mat4 mPiece5 = makeModelMatrix(toClip(64.3977f, 217.6685f), 75.3605f, glm::vec2(1.0f * su));

    // Peca 6: Triangulo Pequeno 2 (Focinho a uivar)
    // Catetos = 1.0 * su. Origem no angulo reto (48.096, 245.499)
    glm::mat4 mPiece6 = makeModelMatrix(toClip(48.0956f, 245.4989f), 165.361f, glm::vec2(1.0f * su));

    // Peca 7: Triangulo Medio (Pata Dianteira)
    // Catetos = sqrt(2) * su. Origem no angulo reto (89.841, 187.451)
    glm::mat4 mPiece7 = makeModelMatrix(toClip(89.8409f, 187.451f), 165.362f, glm::vec2(SQRT2 * su));

    // Cores vibrantes e distintas para cada peca (Paleta moderna Tangram)
    glm::vec4 colPiece1(0.92f, 0.26f, 0.21f, 1.0f); // Vermelho carmesim
    glm::vec4 colPiece2(0.13f, 0.59f, 0.95f, 1.0f); // Azul real
    glm::vec4 colPiece3a(1.00f, 0.60f, 0.00f, 1.0f); // Laranja ambar
    glm::vec4 colPiece4(0.18f, 0.80f, 0.44f, 1.0f); // Verde esmeralda
    glm::vec4 colPiece5(0.95f, 0.77f, 0.06f, 1.0f); // Amarelo sol
    glm::vec4 colPiece6(0.91f, 0.12f, 0.39f, 1.0f); // Rosa magenta
    glm::vec4 colPiece7(0.61f, 0.35f, 0.71f, 1.0f); // Roxo ametista

    // Criacao dos objetos TangramPiece
    tangramPieces.emplace_back("1. Triangulo Grande (Peito)",    vaoTriangle,       3, colPiece1,  mPiece1);
    tangramPieces.emplace_back("2. Triangulo Grande (Corpo)",    vaoTriangle,       3, colPiece2,  mPiece2);
    tangramPieces.emplace_back("3a. Paralelogramo (Cauda)",      vaoParallelogram,  6, colPiece3a, mPiece3a);
    tangramPieces.emplace_back("4. Triangulo Pequeno (Garganta)",vaoTriangle,       3, colPiece4,  mPiece4);
    tangramPieces.emplace_back("5. Quadrado (Cabeca)",           vaoSquare,         6, colPiece5,  mPiece5);
    tangramPieces.emplace_back("6. Triangulo Pequeno (Focinho)", vaoTriangle,       3, colPiece6,  mPiece6);
    tangramPieces.emplace_back("7. Triangulo Medio (Pata)",      vaoTriangle,       3, colPiece7,  mPiece7);

    originalPieces = tangramPieces;

    // Ativa suporte para desenho de linhas suaves no modo wireframe
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);

    // ========================================================================
    // 6. Ciclo de Renderizacao Principal
    // ========================================================================
    while (!glfwWindowShouldClose(window)) {
        // Cor de fundo escura e elegante (#18181b / dark gray)
        glClearColor(0.094f, 0.094f, 0.106f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Desenha todas as pecas na ordem atual da colecao
        // Como cada peca gere a sua propria matriz de modelo e cor,
        // a ordem em que este ciclo corre e absolutamente irrelevante para o resultado.
        for (const auto& piece : tangramPieces) {
            piece.draw(modelLoc, colorLoc);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ========================================================================
    // 7. Limpeza de Recursos
    // ========================================================================
    glDeleteVertexArrays(1, &vaoTriangle);
    glDeleteBuffers(1, &vboTriangle);
    glDeleteVertexArrays(1, &vaoSquare);
    glDeleteBuffers(1, &vboSquare);
    glDeleteVertexArrays(1, &vaoParallelogram);
    glDeleteBuffers(1, &vboParallelogram);

    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Aplicacao terminada com sucesso.\n";
    return 0;
}
