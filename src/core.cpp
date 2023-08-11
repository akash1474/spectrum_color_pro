#include "FontAwesome6.h"
#include "imgui.h"
#include "pch.h"
#include "core.h"
#include "pallet.h"
#include <winuser.h>


COLORREF GetColorAtCursorPos()
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    HDC hScreenDC = GetDC(NULL);
    COLORREF color = GetPixel(hScreenDC, cursorPos.x, cursorPos.y);
    ReleaseDC(NULL, hScreenDC);
    return color;
}

ImVec4 RGBAToHSL(const ImVec4& rgba) {
    float r = rgba.x;
    float g = rgba.y;
    float b = rgba.z;

    float maxChannel = max(max(r, g), b);
    float minChannel = min(min(r, g), b);

    float h = 0.0f;
    float s = 0.0f;
    float l = (maxChannel + minChannel) / 2.0f;

    float delta = maxChannel - minChannel;

    if (delta != 0.0f) {
        s = delta / (1.0f - std::abs(2.0f * l - 1.0f));

        if (maxChannel == r) {
            h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
        } else if (maxChannel == g) {
            h = (b - r) / delta + 2.0f;
        } else if (maxChannel == b) {
            h = (r - g) / delta + 4.0f;
        }

        h /= 6.0f;
    }

    return ImVec4(h, s, l, rgba.w);
}

float HueToRGB(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

ImVec4 HSLToRGBA(const ImVec4& hsl) {
    float h = hsl.x;
    float s = hsl.y;
    float l = hsl.z;

    float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    float p = 2.0f * l - q;

    float r = HueToRGB(p, q, h + 1.0f / 3.0f);
    float g = HueToRGB(p, q, h);
    float b = HueToRGB(p, q, h - 1.0f / 3.0f);

    return ImVec4(r, g, b, hsl.w);
}



ImVec4 RGBAToHSV(const ImVec4& rgba) {
    float r = rgba.x;
    float g = rgba.y;
    float b = rgba.z;

    float maxChannel = max(max(r, g), b);
    float minChannel = min(min(r, g), b);

    float h = 0.0f;
    float s = 0.0f;
    float v = maxChannel;

    float delta = maxChannel - minChannel;

    if (delta != 0.0f) {
        s = delta / maxChannel;

        if (maxChannel == r) {
            h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
        } else if (maxChannel == g) {
            h = (b - r) / delta + 2.0f;
        } else if (maxChannel == b) {
            h = (r - g) / delta + 4.0f;
        }

        h /= 6.0f;
    }
    return ImVec4(h, s, v, rgba.w);
}

void CoreSystem::renderMenuBar()
{
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu("File")) {
        ImGui::MenuItem("RGB Format");
        ImGui::MenuItem("HSL Format");
        ImGui::MenuItem("HSV Format");
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Color Pallet", 0, showColors)) showColors = !showColors;
        if (ImGui::MenuItem("Shades", 0, showShades)) showShades = !showShades;
        if (ImGui::MenuItem("Tints", 0, showTints)) showTints = !showTints;
        ImGui::MenuItem("Last Color");
        ImGui::MenuItem("Mini Picker");
        ImGui::MenuItem("Monochromatic colors");
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Options")) {
        if (ImGui::MenuItem("Show FPS", 0, showFps)) showFps = !showFps;
        if (ImGui::MenuItem("Enable Vsync", 0, vSync)) {
            vSync = !vSync;
            vSync ? glfwSwapInterval(1) : glfwSwapInterval(0);
        }
        ImGui::MenuItem("Monochromatic colors");
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help",true)) {
        ImGui::Spacing();
        ImGui::Text("©Akash Pandit. All Rights Reserved");
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Version: 0.1.0");
        ImGui::Text("GitHub: github.com/akash1474");
        ImGui::Separator();
        if(ImGui::MenuItem("Help","F1")) showHelp=true;
        ImGui::EndMenu();
    }
    if (showFps) {
        static char fps[16];
        sprintf_s(fps, "%.2f FPS", ImGui::GetIO().Framerate);
        float posx = (width - ImGui::CalcTextSize(fps).x) - 10;
        ImGui::SetCursorPos({posx, 0});
        ImGui::Text("%s", fps);
    }
    ImGui::EndMenuBar();
}
std::string format_hsv(std::string& format,ImVec4& color){
    std::string final_str;
    bool isFloat=format[0]=='f';
    size_t i=0;
    if(isFloat) ++i;
    float factor=1000.0f;
    ImVec4 hsv = RGBAToHSV(color);
    hsv.w=color.w;
    while(i<format.size()){
        if(format[i]=='$'){
            switch(format[++i]){
            case 'h':
                final_str+= isFloat ? std::to_string(float(round(hsv.x*factor))/factor).substr(0,5) :std::to_string(int(round(hsv.x*360)));
                break;
            case 's':
                final_str+= isFloat ? std::to_string(float(round(hsv.y*factor))/factor).substr(0,5) : std::to_string(int(round(hsv.y*100)));
                break;
            case 'l':
                final_str+= isFloat ? std::to_string(float(round(hsv.z*factor))/factor).substr(0,5) : std::to_string(int(round(hsv.z*100)));
                break;
            case 'a':
                final_str+= isFloat ? std::to_string(float(round(hsv.w*factor))/factor).substr(0,5) : std::to_string(int(round(hsv.w*100)));
                break;
            }
        }else{
            final_str+=format[i];
        }
        ++i;
    }
    return final_str;

}

std::string format_rgb(std::string& format,ImVec4& color){
    std::string final_str;
    bool isFloat=format[0]=='f';
    size_t i=0;
    if(isFloat) ++i;
    float factor=1000.0f;
    while(i<format.size()){
        if(format[i]=='$'){
            switch(format[++i]){
            case 'r':
                final_str+= isFloat ? std::to_string(float(round(color.x*factor))/factor).substr(0,5) :std::to_string(int(round(color.x*255)));
                break;
            case 'g':
                final_str+= isFloat ? std::to_string(float(round(color.y*factor))/factor).substr(0,5) : std::to_string(int(round(color.y*255)));
                break;
            case 'b':
                final_str+= isFloat ? std::to_string(float(round(color.z*factor))/factor).substr(0,5) : std::to_string(int(round(color.z*255)));
                break;
            case 'a':
                final_str+= isFloat ? std::to_string(float(round(color.w*factor))/factor).substr(0,5) : std::to_string(int(round(color.w*255)));
                break;
            }
        }else{
            final_str+=format[i];
        }
        ++i;
    }
    return final_str;
}

void CoreSystem::updateShades(){
    ImVec4 hsl=RGBAToHSL(color);
    hsl.y=0.704918;
    hsl.z=0.119590;
    shades[0]=HSLToRGBA(hsl);
    hsl.y=0.557377;
    hsl.z=0.239181;
    shades[1]=HSLToRGBA(hsl);
    hsl.y=0.557522;
    hsl.z=0.443073;
    shades[2]=HSLToRGBA(hsl);
    hsl.y=0.562614;
    hsl.z=0.641083;
    shades[3]=HSLToRGBA(hsl);
    hsl.y=0.554416;
    hsl.z=0.929277;
    shades[4]=HSLToRGBA(hsl);
}

bool CoreSystem::Pallet(const char* idx,std::vector<ImVec4>& colors,const bool isDisabled){
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(idx);



    const ImVec2 size(window->Size.x-25,50.0f);
    const float clr_width=round(size.x/colors.size());
    ImVec2 pos=window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb,0);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool isHovered=false;
    bool isClicked=ImGui::ButtonBehavior(bb,id,&isHovered,0,ImGuiButtonFlags_AllowOverlap);
    for(size_t i=0;i<colors.size();i++){
        ImVec2 p_min(pos.x+i*clr_width,pos.y);
        ImVec2 p_max(p_min.x+clr_width,pos.y+size.y);
        static const std::string colorId(std::string("Color_"+std::to_string(i)));
        const ImGuiID id_color = window->GetID(colorId.c_str());
        float rounding=0.0f;
        if(i==0 || i==(colors.size()-1)) rounding=4.0f;
        ImDrawFlags flags=ImDrawFlags_None;
        if(i==0) flags|=ImDrawFlags_RoundCornersLeft;
        if(i==colors.size()-1) flags|=ImDrawFlags_RoundCornersRight;
        window->DrawList->AddRectFilled(p_min,p_max,ImColor(colors[i]),rounding,flags);
        if(isDisabled){
            const ImRect color_bb(p_min,p_max);
            bool isColorHovered=false;
            // const ImVec2 points[2]={p_min,ImVec2(p_max.x,p_min.y),p_max,ImVec2(p_min.x,p_max.y)};
            if(ImGui::ButtonBehavior(color_bb, id_color, &isColorHovered,0,ImGuiButtonFlags_PressedOnClick)) this->color=colors[i];
            if(isColorHovered) window->DrawList->AddRectFilled(p_min,p_max,ImColor(255,255,255,80));
            if(isColorHovered) window->DrawList->AddLine(ImVec2(p_max.x-10,p_max.y+5),ImVec2(p_min.x+10,p_max.y+5),ImColor(255,255,255,255), 2.0f);
        }
        
    }
    if(isHovered && !isDisabled) window->DrawList->AddRectFilled(bb.Min,bb.Max,ImColor(255,255,255,80),4.0f);
    return isClicked;
}

void CoreSystem::renderPallets(){
    ImGui::Text("Pallets");
    ImGui::BeginChild("##Pallets",ImVec2(width,height-50),0,ImGuiWindowFlags_NoTitleBar);
    ImGui::Separator();
    int i=0;
    for(auto& pallet:pallets){
       if(Pallet(std::to_string(1000+i).c_str(),pallet)){
            p_idx=i;
            showPallet=true;
       }
       i++;
    }
    ImGui::EndChild();
}

void CoreSystem::renderPicker(){
    static bool isPicking = false;
    COLORREF colorx = GetColorAtCursorPos();

    static bool saved_palette_init = true;
    if (saved_palette_init) {
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++) {
            ImGui::ColorConvertHSVtoRGB(n / 40.0f, 0.8f, 0.8f, saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
            saved_palette[n].w = 1.0f; // Alpha
        }
        saved_palette_init = false;
    }

    if(ImGui::ColorPicker4("##picker", (float*)&color,ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaBar)){
        updateShades();
    }
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Current");
    ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 30));
    ImGui::Text("Shades");
    for(size_t i=0;i<shades.size();i++){
        if (ImGui::ColorButton(std::string("##shade_"+std::to_string(i)).c_str(), shades[i], ImGuiColorEditFlags_NoPicker,ImVec2(60, 30))) color = shades[i];
    }
    ImGui::EndGroup();
    ImGui::Button(ICON_FA_EYE_DROPPER "  Pick Color", {120, 0});
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) isPicking = true;
    if (isPicking && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) isPicking = false;
    if (isPicking) {
        int red = GetRValue(colorx);
        int green = GetGValue(colorx);
        int blue = GetBValue(colorx);
        color.x = red * 0.003921;
        color.y = green * 0.003921;
        color.z = blue * 0.003921;
        updateShades();
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) isPicking = false;
    ImGui::SameLine();
    ImGui::Button(ICON_FA_BOOKMARK "  Save", ImVec2{120, 0});

    if(ImGui::Button(ICON_FA_COPY "  Hex", ImVec2{120, 0})){
        std::stringstream stream;
        stream << "#" << std::hex;
        int r=int(round(color.x*255));
        int g=int(round(color.y*255));
        int b=int(round(color.z*255));
        int a=int(round(color.w*255));
        r<16 ? stream << "0" << r : stream << r;
        g<16 ? stream << "0" << g : stream << g;
        b<16 ? stream << "0" << b : stream << b;
        a<16 ? stream << "0" << a : stream << a;
        GL_INFO(stream.str());
        ImGui::SetClipboardText(stream.str().c_str());
    }
    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_COPY "  RGB", ImVec2{120, 0})){
        std::string text=format_rgb(rgb_format,color);
        GL_INFO(text);
        ImGui::SetClipboardText(text.c_str());
    }
    if(ImGui::Button(ICON_FA_COPY "  HSV", ImVec2{120, 0})){
        std::string text=format_hsv(hsv_format,color);
        GL_INFO(text);
        ImGui::SetClipboardText(text.c_str());
    }
    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_COPY "  HSL", ImVec2{120, 0})){
        std::string text=format_hsv(hsv_format,color);
        ImVec4 hsl=RGBAToHSL(color);
        GL_INFO("h:{} s:{} l:{}",hsl.x,hsl.y,hsl.z);
        // GL_INFO("r:{} g:{} b:{}",color.x,color.y,color.z);
        // ImVec4 rgb=HSLToRGBA(hsl);
        // GL_INFO("r:{} g:{} b:{}",rgb.x,rgb.y,rgb.z);
    }
    bool s_visible=ImGui::GetCurrentWindow()->ContentSize.y > ImGui::GetWindowSize().y;
    float b_pos=s_visible ? ImGui::GetWindowWidth()-40.0f :ImGui::GetWindowWidth()-30.0f;
    if(showPallet){
        ImGui::Separator();
        ImGui::Text("Pallet");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xpallet",ImVec2(20,20))) showPallet=false;
        Pallet("##SelectedPallet",pallets[p_idx],true);
        ImGui::Spacing();
    }
    if(showShades){
        ImGui::Separator();
        ImGui::Text("Shades");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xshades",ImVec2(20,20))) showShades=!showShades;
        static std::vector<ImVec4> shades(7,ImVec4());
        for(int i=0;i<shades.size();i++) shades[i]=darkerShade(color,i*0.1428);
        Pallet("##color_shades",shades,true);
        ImGui::Spacing();
    }
    if(showTints){
        ImGui::Separator();
        ImGui::Text("Tints");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xtints",ImVec2(20,20))) showTints=!showTints;
        static std::vector<ImVec4> tints(7,ImVec4());
        for(int i=0;i<tints.size();i++) tints[i]=lighterShade(color,i*0.1428);
        Pallet("##color_shades",tints,true);
        ImGui::Spacing();
    }
    if (showColors) {
        ImGui::Separator();
        ImGui::Text("Colors");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xcolors",ImVec2(20,20))) showColors=!showColors;
        float width = ImGui::GetWindowWidth();
        int count = width / (20 + ImGui::GetStyle().ItemSpacing.y + 0.5f);
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++) {
            ImGui::PushID(n);
            if ((n % count) != 0) ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

            ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
            if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20))){
                color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w);
                updateShades();
            }

            ImGui::PopID();
        }
    }
}

void CoreSystem::render()
{
    ImGui::ShowDemoWindow();
    ImGui::SetNextWindowPos({0, 0});
    // #ifndef GL_DEBUG
    // ImGui::SetNextWindowSize({width, showColors ? height : height-125});
    // #endif
    ImGui::Begin("##ColorPicker", 0,
        #ifndef GL_DEBUG
        ImGuiWindowFlags_NoResize | 
        #endif
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

    renderMenuBar();
    if(ImGui::BeginTabBar("TabBar",ImGuiTabBarFlags_None)){

        if(ImGui::BeginTabItem("Picker")){
            renderPicker();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Pallets")){
            renderPallets();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}