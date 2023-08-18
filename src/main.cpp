#include "GLFW/glfw3.h"
#include "Timer.h"
#include "pch.h"
#include "imgui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_img.h"
#include "core.h"

// #ifdef GL_DEBUG
// #define WIDTH 600
// #define HEIGHT 450
// #else
#define WIDTH 270
#define HEIGHT 405
// #endif

    

int width{0};
int height{0};

CoreSystem* corePtr=nullptr;
void initFonts(int front_size);
void draw(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


int main(void)
{
    OpenGL::Timer timer;
    GLFWwindow* window;
#ifdef GL_DEBUG
    OpenGL::Log::Init();
#endif

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Color Picker", NULL, NULL);
    #ifdef GL_DEBUG 
    glfwSetWindowSizeLimits(window, 270, 405, GLFW_DONT_CARE, GLFW_DONT_CARE);
    #else
    glfwSetWindowSizeLimits(window, 270, 405, 320, 442);
    #endif

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLFWimage images[1];
    images[0].pixels = stbi_load_from_memory(logo_img, IM_ARRAYSIZE(logo_img), &images[0].width, &images[0].height, 0, 4); // rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    CoreSystem core;
    core.width=WIDTH;
    core.height=HEIGHT;
    corePtr=&core;

    // Initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    if (!ImGui_ImplOpenGL2_Init()) GL_ERROR("Failed to initit OpenGL 2");

    initFonts(core.font_size);

    glfwSwapInterval(0);
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 2.0f;
    style.ItemSpacing.y = 6.0f;
    style.ScrollbarRounding = 2.0f;
    StyleColorsDracula();

    GL_WARN("[Timer] Initialization - {}",timer.ElapsedMillis());
    while (!glfwWindowShouldClose(window)) {
        if(!core.isRunning) break;
        if(core.buildFonts){
            initFonts(core.font_size);
            core.buildFonts=false;
            ImGui_ImplOpenGL2_CreateFontsTexture();
        }
        glfwGetWindowSize(window, &width, &height);
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Render Other Stuff

        core.render();

        // End of render
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


void initFonts(int font_size){
    #ifdef GL_DEBUG
    OpenGL::ScopedTimer font_init("Font Build");
    #endif
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    #ifndef GL_DEBUG
    io.IniFilename=nullptr;
    #endif
    io.LogFilename = nullptr;

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icon_config;
    icon_config.MergeMode = true;
    icon_config.PixelSnapH = true;
    icon_config.FontDataOwnedByAtlas = false;

    const int font_data_size = IM_ARRAYSIZE(data_font);
    const int font_data_mono_size = IM_ARRAYSIZE(data_font_mono);
    const int icon_data_size = IM_ARRAYSIZE(data_icon);

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)data_font, font_data_size, (float)font_size, &font_config);
    io.Fonts->AddFontFromMemoryTTF((void*)data_icon, icon_data_size, (font_size+4) * 2.0f / 3.0f, &icon_config, icons_ranges);

    io.Fonts->AddFontFromMemoryTTF((void*)data_font_mono, font_data_mono_size, (float)(font_size+2), &font_config);
}

void draw(GLFWwindow* window)
{
    glfwGetWindowSize(window, &width, &height);
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if(corePtr) corePtr->render();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    #ifndef GL_DEBUG
    corePtr->width=width;
    corePtr->height=height;
    #endif
    draw(window);
}
