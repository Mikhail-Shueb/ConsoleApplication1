/**
 * ============================================================================
 * Computacao Grafica e Jogos - 2025/2026
 * Assignment 3: 3D Pickagram (Figure #100: WOLF on Tabletop)
 *
 * Requisitos cumpridos:
 * 1. Geometria:
 *    - Formas 3D carregadas a partir de ficheiros com volume e normais (Assimp).
 *    - Tonalidade por face modulada pelo vetor normal (volumetria clara sem luzes complexas).
 *    - Grafo de cena hierarquico (SceneNode) com WorldMatrix = ParentWorld * Local.
 *    - Superficie plana (mesa) no grafo; mover a mesa arrasta o Pickagram.
 *    - Arrastar com botao esquerdo do rato move a mesa nas direcoes frontal/lateral da camara.
 * 2. Animacao:
 *    - Duas configuracoes: Caixa fechada (plano X-Z na mesa) e Lobo em pe (plano X-Y).
 *    - Setas Esquerda/Direita animam continuamente entre as duas configuracoes.
 *    - Paragem a meio e reversao suave sem saltos (LERP e SLERP com quaternioes).
 * 3. Camaras:
 *    - 2 camaras orbitais independentes com alvos e orientacoes proprias.
 *    - Orbita com botao direito do rato usando quaternioes (prevencao de Gimbal Lock).
 *    - Zoom com scroll wheel.
 *    - Tecla 'c' comuta entre as 2 camaras mantendo o estado de cada uma.
 *    - Tecla 'p' comuta entre projecao Perspetiva e Ortografica.
 *    - Redimensionamento de janela preserva a razao de aspeto (aspect ratio).
 * =============================================================================
 * DECLARACAO DE HONRA
 * =============================================================================
 *  "Nos, abaixo assinados,
 *  Mikhail Mohamed Shueb, n.º 104014, curso MEIC-A
 *  [Nome do Estudante 2], n.º [xxxx], curso [curso]
 *  declaramos, sob compromisso de honra, que o trabalho intitulado "Assignment 3 - 3D Pickagram",
 *  realizado no ambito da unidade curricular Computacao Grafica para Jogos,
 *  resultou da participacao ativa dos seguintes elementos do grupo. Os contributos individuais foram:
 *  Mikhail Shueb – Criacao da iteracao inicial e arquitetura do projeto;
 *  [Nome do Estudante 2] – [descricao sucinta do contributo]
 *  Declaramos que o trabalho e original e respeita as normas academicas em vigor.
 *  Campus Taguspark IST, 24/11/2026
 *  Assinaturas: Mikhail Shueb & [Nome do Estudante 2]"
 * =============================================================================
 * DECLARACAO DE USO DE INTELIGENCIA ARTIFICIAL (Nivel 1 - Uso Limitado)
 * =============================================================================
 *  "Todas as decisoes e conteudo tecnico deste trabalho sao da nossa autoria.
 *   Foram utilizadas ferramentas de IA generativa (Google Antigravity / Gemini)
 *   para apoio na estruturacao da arquitetura de classes do grafo de cena,
 *   matematica de quaternioes e sistema de camaras orbitais no OpenGL."
 * =============================================================================
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>

#include "Shader.h"
#include "Mesh.h"
#include "ModelLoader.h"
#include "Camera.h"
#include "SceneNode.h"
#include "Table.h"
#include "Animation.h"

// ============================================================================
// Shaders GLSL 3.3 Core (Com modulacao de tonalidade por normal de face)
// ============================================================================

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 vNormal;
out vec3 vFragPos;

void main() {
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    // Transforma a normal para o espaco de mundo
    vNormal = normalize(mat3(transpose(inverse(u_Model))) * aNormal);
    gl_Position = u_Projection * u_View * worldPos;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vNormal;
in vec3 vFragPos;
out vec4 FragColor;

uniform vec4 u_BaseColor;

void main() {
    vec3 N = normalize(vNormal);

    // Requisito do enunciado:
    // Produzir diferentes tonalidades da cor base em cada face atraves do vetor normal,
    // garantindo que a volumetria 3D e perfeitamente percetivel sem iluminacao pesada.
    float tint = 0.65 + 0.35 * (N.y * 0.45 + N.z * 0.35 + N.x * 0.20);
    tint = clamp(tint, 0.40, 1.08);

    vec3 shadedColor = u_BaseColor.rgb * tint;
    FragColor = vec4(shadedColor, u_BaseColor.a);
}
)";

// ============================================================================
// Variaveis Globais de Estado e Controlos
// ============================================================================

Camera cameras[2];
int activeCameraIndex = 0;

std::unique_ptr<Table> tableInstance;
std::unique_ptr<AnimationManager> animManager;
std::shared_ptr<SceneNode> sceneRoot;

bool leftMouseButtonPressed = false;
bool rightMouseButtonPressed = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;

bool keyLeftPressed = false;
bool keyRightPressed = false;

// ============================================================================
// Callbacks do GLFW
// ============================================================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height <= 0) height = 1;
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)height;
    cameras[0].setAspect(aspect);
    cameras[1].setAspect(aspect);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMouseButtonPressed = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            leftMouseButtonPressed = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMouseButtonPressed = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            rightMouseButtonPressed = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    double deltaX = xpos - lastMouseX;
    double deltaY = ypos - lastMouseY;
    lastMouseX = xpos;
    lastMouseY = ypos;

    // Botao Direito: Orbita a camara ativa com quaternioes
    if (rightMouseButtonPressed) {
        cameras[activeCameraIndex].orbit((float)deltaX, (float)deltaY);
    }

    // Botao Esquerdo: Move a mesa (e todo o Pickagram com ela) relativo a camara
    if (leftMouseButtonPressed && tableInstance) {
        // Invertemos deltaY para que arrastar para cima afaste (direcao frontal)
        tableInstance->moveWithMouse((float)deltaX, -(float)deltaY, cameras[activeCameraIndex]);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Zoom com a roda do rato
    cameras[activeCameraIndex].zoom((float)yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(window, true);
        }
        else if (key == GLFW_KEY_C) {
            // Alterna entre Camara 1 e Camara 2 mantendo os seus respetivos estados
            activeCameraIndex = 1 - activeCameraIndex;
            std::cout << "[Camara] Ativa: Camara " << (activeCameraIndex + 1)
                      << (activeCameraIndex == 0 ? " (Foco no Lobo)" : " (Foco na Caixa)")
                      << " | Projecao: " << (cameras[activeCameraIndex].isPerspective ? "Perspetiva" : "Ortografica")
                      << std::endl;
        }
        else if (key == GLFW_KEY_P) {
            // Alterna entre Projecao Perspetiva e Ortografica na camara ativa
            cameras[activeCameraIndex].toggleProjection();
            std::cout << "[Camara " << (activeCameraIndex + 1) << "] Projecao alterada para: "
                      << (cameras[activeCameraIndex].isPerspective ? "Perspetiva" : "Ortografica")
                      << std::endl;
        }
        else if (key == GLFW_KEY_R) {
            // Repoe a posicao da mesa
            if (tableInstance) tableInstance->resetPosition();
            std::cout << "[Mesa] Posicao reposta ao centro.\n";
        }
    }

    // Controle continuo da animacao com as setas
    if (key == GLFW_KEY_LEFT) {
        keyLeftPressed = (action != GLFW_RELEASE);
    }
    if (key == GLFW_KEY_RIGHT) {
        keyRightPressed = (action != GLFW_RELEASE);
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

    GLFWwindow* window = glfwCreateWindow(1024, 768, "CGJ - Assignment 3: 3D Pickagram (#100 WOLF)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // 2. Inicializacao do GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Falha ao inicializar GLEW!\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Configuracoes de OpenGL 3D
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);

    std::cout << "OpenGL Versao: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer:      " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "--------------------------------------------------------\n";
    std::cout << "CONTROLOS DISPONIVEIS:\n";
    std::cout << "  [Botao Direito Rato + Arrastar] : Orbitar camara (quaternioes, sem gimbal lock)\n";
    std::cout << "  [Scroll Roda Rato]             : Zoom in / Zoom out\n";
    std::cout << "  [Botao Esquerdo Rato + Arrastar]: Mover mesa nas direcoes frontal/lateral da camara\n";
    std::cout << "  [Seta Esquerda] (manter)       : Animar em direcao a Caixa (Box em X-Z)\n";
    std::cout << "  [Seta Direita]  (manter)       : Animar em direcao ao Lobo (Wolf em X-Y)\n";
    std::cout << "  [Soltar Seta]                  : Pausar animacao a meio do caminho\n";
    std::cout << "  [C]                            : Comutar entre Camara 1 e Camara 2\n";
    std::cout << "  [P]                            : Alternar Projecao Perspetiva / Ortografica\n";
    std::cout << "  [R]                            : Repor mesa ao centro\n";
    std::cout << "  [ESC / Q]                      : Sair da aplicacao\n";
    std::cout << "--------------------------------------------------------\n";

    // 3. Criacao dos Shaders
    std::shared_ptr<Shader> shader3D = std::make_shared<Shader>(vertexShaderSource, fragmentShaderSource);

    // 4. Carregamento das Malhas 3D (Assimp / ModelLoader)
    // Se o executavel estiver na raiz ou em bin, procuramos nos caminhos relativos
    std::string modelPathPrefix = "A3/models/";
    std::ifstream testFile(modelPathPrefix + "table.obj");
    if (!testFile.is_open()) {
        modelPathPrefix = "models/";
    } else {
        testFile.close();
    }

    std::shared_ptr<Mesh> meshSmallTri  = ModelLoader::loadModel(modelPathPrefix + "small_triangle.obj", "SmallTriangle");
    std::shared_ptr<Mesh> meshMedTri    = ModelLoader::loadModel(modelPathPrefix + "medium_triangle.obj", "MediumTriangle");
    std::shared_ptr<Mesh> meshLargeTri  = ModelLoader::loadModel(modelPathPrefix + "large_triangle.obj", "LargeTriangle");
    std::shared_ptr<Mesh> meshSquare    = ModelLoader::loadModel(modelPathPrefix + "square.obj", "Square");
    std::shared_ptr<Mesh> meshParall    = ModelLoader::loadModel(modelPathPrefix + "parallelogram.obj", "Parallelogram");
    std::shared_ptr<Mesh> meshTable     = ModelLoader::loadModel(modelPathPrefix + "table.obj", "Table");

    // 5. Construcao do Grafo de Cena
    sceneRoot = std::make_shared<SceneNode>("Root");
    tableInstance = std::make_unique<Table>(meshTable, shader3D.get());
    sceneRoot->addChild(tableInstance->node);

    // Cores vibrantes e distintas para as 7 pecas
    glm::vec4 colPiece1(0.92f, 0.26f, 0.21f, 1.0f); // Vermelho carmesim
    glm::vec4 colPiece2(0.13f, 0.59f, 0.95f, 1.0f); // Azul real
    glm::vec4 colPiece3a(1.00f, 0.60f, 0.00f, 1.0f); // Laranja ambar
    glm::vec4 colPiece4(0.18f, 0.80f, 0.44f, 1.0f); // Verde esmeralda
    glm::vec4 colPiece5(0.95f, 0.77f, 0.06f, 1.0f); // Amarelo sol
    glm::vec4 colPiece6(0.91f, 0.12f, 0.39f, 1.0f); // Rosa magenta
    glm::vec4 colPiece7(0.61f, 0.35f, 0.71f, 1.0f); // Roxo ametista

    std::vector<std::shared_ptr<SceneNode>> pieces;
    pieces.push_back(std::make_shared<SceneNode>("Piece1_LargeTri1",  meshLargeTri, shader3D.get(), colPiece1));
    pieces.push_back(std::make_shared<SceneNode>("Piece2_LargeTri2",  meshLargeTri, shader3D.get(), colPiece2));
    pieces.push_back(std::make_shared<SceneNode>("Piece3a_Parallelo", meshParall,   shader3D.get(), colPiece3a));
    pieces.push_back(std::make_shared<SceneNode>("Piece4_SmallTri1",  meshSmallTri, shader3D.get(), colPiece4));
    pieces.push_back(std::make_shared<SceneNode>("Piece5_Square",     meshSquare,   shader3D.get(), colPiece5));
    pieces.push_back(std::make_shared<SceneNode>("Piece6_SmallTri2",  meshSmallTri, shader3D.get(), colPiece6));
    pieces.push_back(std::make_shared<SceneNode>("Piece7_MediumTri",  meshMedTri,   shader3D.get(), colPiece7));

    // As pecas sao filhas da mesa no Grafo de Cena: mover a mesa arrasta o Pickagram!
    for (auto& p : pieces) {
        tableInstance->node->addChild(p);
    }

    // 6. Inicializacao do Gestor de Animacao
    animManager = std::make_unique<AnimationManager>();
    animManager->registerPieces(pieces);

    // 7. Inicializacao das 2 Camaras Orbitais
    // Camara 1: Alinhada para ver o Lobo em pe confortavelmente
    glm::quat rotCam1 = glm::angleAxis(glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                        glm::angleAxis(glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    cameras[0] = Camera(glm::vec3(0.0f, 1.2f, 0.0f), 10.5f, rotCam1, true);

    // Camara 2: Elevada a 45 graus para ver o tampo da mesa e a Caixa perfeitamente
    glm::quat rotCam2 = glm::angleAxis(glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                        glm::angleAxis(glm::radians(40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    cameras[1] = Camera(glm::vec3(0.0f, 0.5f, 0.0f), 11.5f, rotCam2, true);

    int initialW, initialH;
    glfwGetFramebufferSize(window, &initialW, &initialH);
    framebuffer_size_callback(window, initialW, initialH);

    double lastTime = glfwGetTime();

    // ========================================================================
    // 8. Ciclo Principal de Renderizacao
    // ========================================================================
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Atualiza a interpolacao da animacao
        animManager->update(deltaTime, keyLeftPressed, keyRightPressed);

        // Atualizacao hierarquica do Grafo de Cena
        sceneRoot->update(glm::mat4(1.0f));

        // Limpeza dos buffers de cor e profundidade
        glClearColor(0.094f, 0.094f, 0.106f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrizes da camara ativa
        glm::mat4 view = cameras[activeCameraIndex].getViewMatrix();
        glm::mat4 projection = cameras[activeCameraIndex].getProjectionMatrix();

        // Desenho hierarquico de toda a cena a partir da raiz
        sceneRoot->draw(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "Aplicacao A3 terminada com sucesso.\n";
    return 0;
}
