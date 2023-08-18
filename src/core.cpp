#include "FontAwesome6.h"
#include "GLFW/glfw3.h"
#include "Log.h"
#include "imgui.h"
#include "pch.h"
#include "core.h"
#include "pallet.h"
#include <stdint.h>
#include <string.h>
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

std::string RGBATOHEX(const ImVec4& color){
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
    return stream.str(); 
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
    float h,s,v;
    ImGui::ColorConvertRGBtoHSV(rgba.x,rgba.y,rgba.z,h,s,v);
    return ImVec4(h, s, v, rgba.w);
}

CoreSystem::CoreSystem():color(ImVec4(0.432,0.173,0.691,1.000)){
    usrRootDir=std::string(getenv("USERPROFILE"));
    std::replace(usrRootDir.begin(), usrRootDir.end(), '\\', '/');
    std::string savePath=usrRootDir+"/color_picker";
    if(!std::filesystem::exists(savePath)) std::filesystem::create_directory(savePath);
    savePath+="/init.ini";
    file=new mINI::INIFile(savePath);
    file->read(ini);
    if(!std::filesystem::exists(savePath)){
        //Initilze
        GL_INFO("Initializing Settings");
        ini["settings"]["font_size"]="16";
        ini["settings"]["default_copy"]="0";
        ini["settings"]["load_last_color"]="0";
        ini["settings"]["last_color"]="0";
        ini["settings"]["pallet"]="0";
        ini["settings"]["picker_flags"]="40960272";
        ini["settings"]["show_colors"]="1";
        ini["settings"]["shades"]="1";
        ini["settings"]["tints"]="1";
        ini["settings"]["pid"]="-1";
        ini["settings"]["rgb"]="rgba($r,$g,$b,$a)";
        ini["settings"]["hsl"]="hsla($h,$s%,$l%,$a)";
        ini["settings"]["hsv"]="hsva($h,$s%,$v%,$a)";
        ini["settings"]["showUsrColors"]="1";
        file->write(ini,true);
    }else{
        //Load
        GL_INFO("Loading Settings");
        if(!ini.has("settings")) ini["settings"];
        this->font_size=ini["settings"].has("font_size") ? stoi(ini["settings"]["font_size"]): 16;
        if(this->font_size < 10) this->font_size=10;
        this->picker_flags=ini["settings"].has("picker_flags") ? stoi(ini["settings"]["picker_flags"]): 40960272;
        this->default_copy=ini["settings"].has("default_copy") ? stoi(ini["settings"]["default_copy"]): 0;
        this->load_last_color=ini["settings"].has("load_last_color") ? stoi(ini["settings"]["load_last_color"]): 0;
        if(this->load_last_color){
            this->color=ini["settings"].has("last_color") ? ImGui::ColorConvertU32ToFloat4(stoul(ini["settings"]["last_color"])) : this->color;
        }
        this->showPallet= ini["settings"].has("pallet") ? stoi(ini["settings"]["pallet"]): false;
        this->showShades= ini["settings"].has("shades") ? stoi(ini["settings"]["shades"]): true;
        this->showColors= ini["settings"].has("show_colors") ? stoi(ini["settings"]["show_colors"]): true;
        this->showTints= ini["settings"].has("tints") ? stoi(ini["settings"]["tints"]): true;
        this->p_idx= ini["settings"].has("pid") ? stoi(ini["settings"]["pid"]) : -1;
        this->rgb_format= ini["settings"].has("rgb") ? ini["settings"]["rgb"]: "rgba($r,$g,$b,$a)";
        this->hsl_format= ini["settings"].has("hsl") ? ini["settings"]["hsl"]: "hsla($h,$s%,$l%,$a)";
        this->hsv_format= ini["settings"].has("hsv") ? ini["settings"]["hsv"]: "hsva($h,$s%,$v%,$a)";

        //loading usr colors
        this->showUsrColors=ini["settings"].has("showUsrColors") ? stoi(ini["settings"]["showUsrColors"]) : 1;
        if(ini.has("usr_colors")){
            GL_INFO(" USR LOADED:");
            for(auto const& it:ini["usr_colors"]){
                this->usr_colors.push_back(ImGui::ColorConvertU32ToFloat4(stoul(it.second)));
                GL_INFO("\t{}",it.second);
            }
        }
        
        saveSettings();
    }
    updateShades();
}

void CoreSystem::saveSettings(){
    GL_WARN("Saving Settings");
    ini["settings"]["font_size"]=std::to_string(this->font_size);
    ini["settings"]["default_copy"]=std::to_string(default_copy);
    ini["settings"]["picker_flags"]=std::to_string(this->picker_flags);
    ini["settings"]["load_last_color"]=std::to_string(this->load_last_color);
    ini["settings"]["last_color"]=std::to_string(ImGui::ColorConvertFloat4ToU32(this->color));
    ini["settings"]["pallet"]=std::to_string(this->showPallet);
    ini["settings"]["shades"]=std::to_string(this->showShades);
    ini["settings"]["tints"]=std::to_string(this->showTints);
    ini["settings"]["show_colors"]=std::to_string(this->showColors);
    ini["settings"]["pid"]=(p_idx >= 0) ? std::to_string(p_idx) : "-1";
    ini["settings"]["rgb"]=this->rgb_format;
    ini["settings"]["hsv"]=this->hsv_format;
    ini["settings"]["hsl"]=this->hsl_format;

    //saving usr_colors
    ini["settings"]["showUsrColors"]=std::to_string(this->showUsrColors);
    for (size_t i = 0; i < usr_colors.size(); ++i) {
        const ImVec4& color = usr_colors[i];
        uint32_t colorHex = ImGui::ColorConvertFloat4ToU32(color);
        ini["usr_colors"]["color_" + std::to_string(i)]=std::to_string(colorHex);
    }

    file->write(ini,true);
    this->updateSettings=false;
}

void CoreSystem::renderMenuBar()
{
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu("File")) {
        ImGui::MenuItem("Pallets");
        if(ImGui::MenuItem("Settings")){
            showSettings=true;
            showAboutPage=false;
        }
        if(ImGui::MenuItem("Exit")) isRunning=false;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Color Pallet", 0, showColors)){
            this->updateSettings=true;
            showColors = !showColors;
        }
        if (ImGui::MenuItem("Shades", 0, showShades)){
            showShades = !showShades;
            this->updateSettings=true;
        }
        if (ImGui::MenuItem("Tints", 0, showTints)){
            showTints = !showTints;
            this->updateSettings=true;
        }
        if (ImGui::MenuItem("Saved Colors",0,showUsrColors)){
            showUsrColors=!showUsrColors;
            this->updateSettings=true;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Options")) {
        if (ImGui::MenuItem("Show FPS", 0, showFps)) showFps = !showFps;
        if (ImGui::MenuItem("Enable Vsync", 0, vSync)) {
            vSync = !vSync;
            vSync ? glfwSwapInterval(1) : glfwSwapInterval(0);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help",true)) {
        ImGui::Spacing();
        ImGui::Text("©Akash Pandit. All Rights Reserved");
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Version: 1.0.0");
        ImGui::Text("GitHub: github.com/akash1474");
        ImGui::Separator();
        if(ImGui::MenuItem("Help","F1")){
            showAboutPage=true;
            showSettings=false;
        }
        ImGui::EndMenu();
    }
    if (showFps) {
        static char fps[16];
        sprintf_s(fps, "%.2f FPS", ImGui::GetIO().Framerate);
        float posx = (ImGui::GetWindowWidth() - ImGui::CalcTextSize(fps).x) - 10;
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
            case 'v':
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

std::string format_hsl(std::string& format,ImVec4& color){
    std::string final_str;
    bool isFloat=format[0]=='f';
    size_t i=0;
    if(isFloat) ++i;
    float factor=1000.0f;
    ImVec4 hsl = RGBAToHSL(color);
    while(i<format.size()){
        if(format[i]=='$'){
            switch(format[++i]){
            case 'h':
                final_str+= isFloat ? std::to_string(float(round(hsl.x*factor))/factor).substr(0,5) :std::to_string(int(round(hsl.x*360)));
                break;
            case 's':
                final_str+= isFloat ? std::to_string(float(round(hsl.y*factor))/factor).substr(0,5) : std::to_string(int(round(hsl.y*100)));
                break;
            case 'l':
                final_str+= isFloat ? std::to_string(float(round(hsl.z*factor))/factor).substr(0,5) : std::to_string(int(round(hsl.z*100)));
                break;
            case 'a':
                final_str+= isFloat ? std::to_string(float(round(hsl.w*factor))/factor).substr(0,5) : std::to_string(int(round(hsl.w*100)));
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

    const ImVec2 container_size(ImGui::GetContentRegionAvail().x,90);

    const ImVec2 size(isDisabled ? ImGui::GetContentRegionAvail().x : ImGui::GetContentRegionAvail().x-20,50.0f);
    const float clr_width=round(size.x/colors.size());
    ImVec2 pos=window->DC.CursorPos;
    ImRect bb;
    if(isDisabled){
        bb=ImRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    }else{
        bb=ImRect(pos, ImVec2(pos.x + container_size.x, pos.y + container_size.y));
    }

    ImGui::ItemSize(bb,0);
    if (!ImGui::ItemAdd(bb, id)) return false;

    if(!isDisabled){
        window->DrawList->AddRectFilled({pos.x,pos.y},{pos.x+container_size.x,pos.y+container_size.y},ImColor(ImVec4(0.129,0.129,0.169,1.000)));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::RenderText({pos.x+10,pos.y+5}, ("#"+(std::string(idx).substr(1,3))).c_str());
        ImGui::PopFont();
        // ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.958,0.269,0.706,1.000));
        // ImGui::RenderText({container_size.x-20,pos.y+5}, ICON_FA_HEART);
        // ImGui::PopStyleColor();
    }
    bool isHovered=false;
    bool isClicked=ImGui::ButtonBehavior(bb,id,&isHovered,0,ImGuiButtonFlags_AllowOverlap);
    for(size_t i=0;i<colors.size();i++){
        ImVec2 p_min =isDisabled ? ImVec2{pos.x+i*clr_width,pos.y} : ImVec2{pos.x+10+i*clr_width,pos.y+30};
        ImVec2 p_max(p_min.x+clr_width,p_min.y+size.y);
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
            if(ImGui::ButtonBehavior(color_bb, id_color, &isColorHovered,0,ImGuiButtonFlags_PressedOnClick)){
                this->pcolor=this->color;
                this->color=colors[i];
            }
            if(isColorHovered) window->DrawList->AddRectFilled(p_min,p_max,ImColor(255,255,255,80));
            if(isColorHovered) window->DrawList->AddLine(ImVec2(p_max.x-10,p_max.y+5),ImVec2(p_min.x+10,p_max.y+5),ImColor(255,255,255,255), 2.0f);
        }
        
    }
    if(isHovered && !isDisabled) window->DrawList->AddRectFilled(bb.Min,bb.Max,ImColor(255,255,255,40),4.0f);
    return isClicked;
}

void CoreSystem::renderPallets(){
    ImGui::Text("Pallets");
    ImGui::BeginChild("##Pallets",ImVec2(ImGui::GetWindowWidth()-10,ImGui::GetContentRegionAvail().y),0,ImGuiWindowFlags_NoTitleBar);
    ImGui::Separator();
    int i=0;
    for(auto& pallet:pallets){
       if(Pallet(std::to_string(1000+i).c_str(),pallet)){
            p_idx=i;
            showPallet=true;
            updateSettings=true;
       }
       i++;
    }
    ImGui::EndChild();
}

struct Animation{
    float duration=0.0f;
    float currDuration=0.0f;
    float tick=0.0f;
    Animation(float duration=1.0f):duration{duration}{}
    bool isDone(){ return currDuration >= duration;}
    void begin(){currDuration=0.0f;}
    float update(){
        if(isDone()) return duration;
        currDuration+=ImGui::GetIO().DeltaTime;
        tick=currDuration/duration;
        if(currDuration >= duration){
            tick=1.0f; 
            currDuration=duration;
            return currDuration;
        }else{
            return currDuration;
        }
    }
};


void Notification(const char* msg,float tick){
    ImVec2 windowSize = ImGui::GetWindowSize();
    float m_tick=tick >=0.5 ? 1.0f : tick+0.5;
    ImGui::SetNextWindowPos(ImVec2((windowSize.x-windowSize.x+50.0f) * 0.5f, windowSize.y - (50.0f*m_tick)));
    ImGui::SetNextWindowSize({windowSize.x-50,0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,5.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg,ImColor(0.10f, 0.10f, 0.10f, 1.00f*m_tick).Value);
    ImGui::Begin("Notification", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextWrapped("%s", msg);
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void CoreSystem::renderPicker(){
    static bool isPicking = false;
    static bool showNotification=false;
    static Animation notify(2.0f);
    COLORREF colorx = GetColorAtCursorPos();
    bool s_visible=ImGui::GetCurrentWindow()->ContentSize.y > ImGui::GetWindowSize().y;

    static bool saved_palette_init = true;
    if (saved_palette_init) {
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++) {
            ImGui::ColorConvertHSVtoRGB(n / 40.0f, 0.8f, 0.8f, saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
            saved_palette[n].w = 1.0f; // Alpha
        }
        saved_palette_init = false;
    }

    //Key Binding Default copy 
    if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl,false) && ImGui::IsKeyPressed(ImGuiKey_C,false)){
        GL_INFO("Default Copy {}",default_copy);
        showNotification=true;
        notify.begin();
        switch(default_copy){
            case 0: ImGui::SetClipboardText(RGBATOHEX(color).c_str()); break;
            case 1: ImGui::SetClipboardText(format_rgb(rgb_format,color).c_str()); break;
            case 2: ImGui::SetClipboardText(format_hsl(hsl_format,color).c_str()); break;
            case 3: ImGui::SetClipboardText(format_hsv(hsv_format,color).c_str()); break;
        }
    }
    if(showNotification){
        Notification("Default Copied",(1 + (1+1.70158) * pow(notify.tick - 1, 3) + 1.70158 * pow(notify.tick - 1, 2)));
        notify.update();
    }
    if(notify.isDone()) showNotification=false;

    ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(ImGui::GetWindowWidth()-(s_visible ? 30.0f:15.0f), 30));
    ImGui::SetNextItemWidth(-FLT_MIN);
    if(ImGui::ColorPicker4("##picker", (float*)&color,picker_flags)) updateShades();
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        GL_INFO("Picked");
        updateSettings=true;
    }
    // ImGui::SameLine();
    // ImGui::BeginGroup();
    // ImGui::Text("Current");
    // ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 30));
    // if(showPrevColor) ImGui::ColorButton("##pcolor", pcolor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 30));
    // if(showMonochromatic){
    //     ImGui::Text("Shades");
    //     for(size_t i=0;i<shades.size();i++){
    //         if (ImGui::ColorButton(std::string("##shade_"+std::to_string(i)).c_str(), shades[i], ImGuiColorEditFlags_NoPicker,ImVec2(60, 30))){
    //             pcolor=color;
    //             color = shades[i];
    //         }
    //     }
    // }
    // ImGui::EndGroup();
    float width= s_visible ? ImGui::GetWindowWidth()-38 : ImGui::GetWindowWidth()-25;
    ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.115,0.239,0.497,1.000));
    ImGui::Button(ICON_FA_EYE_DROPPER "  Pick Color",{width*0.5f,0});
    ImGui::PopStyleColor();
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) isPicking = true;
    if (isPicking && ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        isPicking = false;
        updateSettings=true;
    }
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
    ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.165,0.476,0.472,1.000));
    if(ImGui::Button(ICON_FA_BOOKMARK "  Save Color", {width*0.5f,0})){
        this->usr_colors.push_back(this->color);
        updateSettings=true;
        GL_INFO("Added User Color [Size: {}]",usr_colors.size());
    }
    ImGui::PopStyleColor();


    static Animation hexCopy(2.0f);
    static char* hxBtn;
    if(hexCopy.isDone()) hxBtn=(ICON_FA_COPY"  Hex");
    else hxBtn=(ICON_FA_CHECK"  Copied");
    if(ImGui::Button(hxBtn, {width*0.5f,0})){
        hexCopy.begin();
        std::string hexCode=RGBATOHEX(color);
        GL_INFO(hexCode);
        ImGui::SetClipboardText(hexCode.c_str());
    }
    hexCopy.update();

    ImGui::SameLine();
    static Animation rgbAnim(2.0f);
    static char* rgbBtn;
    if(rgbAnim.isDone()) rgbBtn=(ICON_FA_COPY"  RGB");
    else rgbBtn=(ICON_FA_CHECK"  Copied");
    if(ImGui::Button(rgbBtn, {width*0.5f,0})){
        std::string text=format_rgb(rgb_format,color);
        GL_INFO(text);
        ImGui::SetClipboardText(text.c_str());
        rgbAnim.begin();
    }
    rgbAnim.update();


    static Animation hsvAnim(2.0f);
    static char* hsv_btn;
    if(hsvAnim.isDone()) hsv_btn=(ICON_FA_COPY"  HSV");
    else hsv_btn=(ICON_FA_CHECK"  Copied");
    if(ImGui::Button(hsv_btn, {width*0.5f,0})){
        std::string text=format_hsv(hsv_format,color);
        GL_INFO(text);
        ImGui::SetClipboardText(text.c_str());
        hsvAnim.begin();
    }
    hsvAnim.update();

    ImGui::SameLine();
    static Animation hslAnim(2.0f);
    static char* hsl_btn;
    if(hslAnim.isDone()) hsl_btn=(ICON_FA_COPY"  HSL");
    else hsl_btn=(ICON_FA_CHECK"  Copied");
    if(ImGui::Button(hsl_btn, {width*0.5f,0})){
        std::string text=format_hsl(hsl_format,color);
        GL_INFO(text);
        ImGui::SetClipboardText(text.c_str());
        hslAnim.begin();
    }
    hslAnim.update();


    float b_pos=ImGui::GetContentRegionAvail().x-10.0f;
    if(showPallet){
        ImGui::Separator();
        ImGui::Text("Pallet");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xpallet",ImVec2(20,20))){
            showPallet=false;
            updateSettings=true;
        }
        Pallet("##SelectedPallet",pallets[p_idx],true);
        ImGui::Spacing();
    }
    if(showShades){
        ImGui::Separator();
        ImGui::Text("Shades");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xshades",ImVec2(20,20))){
            showShades=false;
            updateSettings=true;
        }
        static std::vector<ImVec4> shades(7,ImVec4());
        for(int i=0;i<shades.size();i++) shades[i]=darkerShade(color,i*0.1428);
        Pallet("##color_shades",shades,true);
        ImGui::Spacing();
    }
    if(showTints){
        ImGui::Separator();
        ImGui::Text("Tints");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xtints",ImVec2(20,20))){
            showTints=false;
            updateSettings=true;
        }
        static std::vector<ImVec4> tints(7,ImVec4());
        for(int i=0;i<tints.size();i++) tints[i]=lighterShade(color,i*0.1428);
        Pallet("##color_shades",tints,true);
        ImGui::Spacing();
    }
    if (showColors) {
        ImGui::Separator();
        ImGui::Text("Colors");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xcolors",ImVec2(20,20))){
            showColors=false;
            updateSettings=true;
        }
        float width = ImGui::GetContentRegionAvail().x;
        int count = width / (20.0f+6.0f);
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++) {
            ImGui::PushID(n);
            if ((n % count) != 0) ImGui::SameLine();

            ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
            if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20))){
                pcolor=color;
                color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w);
                updateShades();
            }
            ImGui::PopID();
        }
        ImGui::Spacing();
    }

    if(showUsrColors){
        ImGui::Separator();
        ImGui::Text("Saved Colors");
        ImGui::SameLine(b_pos);
        if(ImGui::Button(ICON_FA_XMARK"##xusrcolors",ImVec2(20,20))){
            showUsrColors=false;
            updateSettings=true;
        }
        float width = ImGui::GetContentRegionAvail().x;
        int count = width / (20.0f+6.0f);
        size_t n = 0;
        for (auto it=usr_colors.begin();it!=usr_colors.end();++it) {
            ImGui::PushID(n);
            if ((n % count) != 0) ImGui::SameLine();
            ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
            if (ImGui::ColorButton("##usrpalette", *it, palette_button_flags, ImVec2(20, 20))){
                pcolor=color;
                color = ImVec4((*it).x, (*it).y, (*it).z, color.w);
                updateShades();
            }
            if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right,false)){
                GL_INFO("Deleted {}",n);
                if(n==(usr_colors.size()-1)){
                    this->usr_colors.pop_back();
                    ImGui::PopID();
                    break;
                }
                it=this->usr_colors.erase(it);
                ImGui::PopID();
                continue;
            }
            n++;
            ImGui::PopID();
        }
        ImGui::Spacing();
    }

}

void CoreSystem::renderSettings(){
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Settings");
    ImGui::SameLine(ImGui::GetWindowWidth()-40);
    if(ImGui::Button(ICON_FA_XMARK,{25,25})) showSettings=false;


    ImGui::SeparatorText("Configurations");
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Font Size:");
    ImGui::SameLine();
    if(ImGui::InputInt("##font_size",&font_size)){
        GL_INFO("Font Size:{}",font_size);
        buildFonts=true;
        saveSettings();
    }
    if(ImGui::Checkbox("Preserve State",&load_last_color)){
        GL_INFO(ImGui::ColorConvertFloat4ToU32(color));
        GL_INFO(ImGui::ColorConvertU32ToFloat4(ImGui::ColorConvertFloat4ToU32(color)).x);
        updateSettings=true;
    }
    static bool configs[4]={
        !!(picker_flags & ImGuiColorEditFlags_DisplayRGB),
        !!(picker_flags & ImGuiColorEditFlags_DisplayHSV),
        !!(picker_flags & ImGuiColorEditFlags_DisplayHex),
        !!(picker_flags & ImGuiColorEditFlags_PickerHueBar)
    };
    if (ImGui::Checkbox("Display RGB",configs)) {
        configs[0] ? picker_flags|=ImGuiColorEditFlags_DisplayRGB : picker_flags&=~ImGuiColorEditFlags_DisplayRGB;
        if(configs[0]){GL_INFO("Enabled");}else{GL_INFO("Disabled");}
        updateSettings=true;
    }
    if (ImGui::Checkbox("Display HSV",configs+1)) {
        updateSettings=true;
        configs[1] ? picker_flags|=ImGuiColorEditFlags_DisplayHSV : picker_flags&=~ImGuiColorEditFlags_DisplayHSV;
    }
    if (ImGui::Checkbox("Display Hex",configs+2)) {
        updateSettings=true;
        configs[2] ? picker_flags|=ImGuiColorEditFlags_DisplayHex : picker_flags&=~ImGuiColorEditFlags_DisplayHex;
    }

    const char* items[] = {"Hex","RGB","HSL","HSV"};
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Default Copy");ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if(ImGui::Combo("##default_copy", &default_copy, items, IM_ARRAYSIZE(items))){
        updateSettings=true;
    }



    ImGui::SeparatorText("Picker Type");
    if(ImGui::RadioButton("Hue Bar",configs[3])){
        picker_flags&=~ImGuiColorEditFlags_PickerHueWheel;
        picker_flags|=ImGuiColorEditFlags_PickerHueBar;
        configs[3]=true;
        updateSettings=true;
    }
    ImGui::SameLine();
    if(ImGui::RadioButton("Hue Wheel",!configs[3])){
        picker_flags&=~ImGuiColorEditFlags_PickerHueBar;
        picker_flags|=ImGuiColorEditFlags_PickerHueWheel;
        configs[3]=false;
        updateSettings=true;
    }



    // ImGui::SeparatorText("Alpha Preview");
    // static int opt=2;
    // if(ImGui::RadioButton("Bar",&opt,0)) picker_flags|=ImGuiColorEditFlags_AlphaBar;
    // ImGui::SameLine();
    // if(ImGui::RadioButton("Preview",&opt,1)) picker_flags|=ImGuiColorEditFlags_AlphaPreview;
    // ImGui::SameLine();
    // if(ImGui::RadioButton("Preview Half",&opt,2)) picker_flags|=ImGuiColorEditFlags_AlphaPreviewHalf;


    ImGui::SeparatorText("Copy Formatting");
    ImGui::AlignTextToFramePadding();
    ImGui::Text("RGB");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth()-90);
    char buff[64];
    strcpy_s(buff,rgb_format.c_str());
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    if(ImGui::InputText("##rgb_input", buff, IM_ARRAYSIZE(buff))) rgb_format=buff;
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(ICON_FA_CODE);
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("Output: %s", format_rgb(rgb_format,color).c_str());
    ImGui::Spacing();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("HSV");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth()-90);
    strcpy_s(buff,hsv_format.c_str());
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    if(ImGui::InputText("##hsv_input", buff, IM_ARRAYSIZE(buff))) hsv_format=buff;
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(ICON_FA_CODE);
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("Output: %s", format_hsv(hsv_format,color).c_str());
    ImGui::Spacing();


    ImGui::AlignTextToFramePadding();
    ImGui::Text("HSL");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth()-90);
    strcpy_s(buff,hsl_format.c_str());
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    if(ImGui::InputText("##hsl_input", buff, IM_ARRAYSIZE(buff))) hsl_format=buff;
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(ICON_FA_CODE);
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("Output: %s", format_hsl(hsl_format,color).c_str());
    ImGui::Spacing();
    ImGui::Dummy({ImGui::GetWindowWidth()-140,10});
    ImGui::SameLine();
    if(ImGui::Button("Save Format")) saveSettings();

    if(updateSettings) saveSettings();
}

static ImVec2 calculateSectionBoundsX(float padding) {
  auto *window = ImGui::GetCurrentWindow();
  float windowStart = ImGui::GetWindowPos().x;

  return {windowStart + window->WindowPadding.x + padding,
          windowStart + ImGui::GetWindowWidth() - window->WindowPadding.x -
              padding};
}

void RenderCard(const char* id,float height,ImVec4 title_color,const char* title, const char* content) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.137,0.137,0.180,1.000));
    ImGui::BeginChild(id,ImVec2(ImGui::GetContentRegionAvail().x+5.0f,height), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::PushStyleColor(ImGuiCol_Text,title_color);
    ImGui::Text("%s", title);

    ImGui::PushStyleColor(ImGuiCol_Separator,title_color);
    ImGui::Separator();
    ImGui::PopStyleColor(2);
    ImGui::TextWrapped("%s", content);

    ImGui::EndChild();
    ImGui::PopStyleColor();
    // ImGui::SetCursorPos({ImGui::GetCursorPos().x,ImGui::GetCursorPos().y+})
}

void CoreSystem::renderAboutPage(){
    static bool isLoaded=false;
    static SVG svg;
    float y=100.f;
    ImGui::SetCursorPos({ImGui::GetWindowWidth()-40.0f,30.0f});
    if(ImGui::Button(ICON_FA_XMARK,{25,25})) showAboutPage=false;
    if(!isLoaded){
        svg.load_from_buffer((const char*)logo_svg,60,60);
        isLoaded=true;
    }
    ImVec2 size=ImGui::GetWindowSize();
    ImGui::SetCursorPos({(size.x-60.0f)*0.5f,y});
    ImGui::Image((void*)(intptr_t)svg.texture,{60,60});
    y+=70.0f;
    ImGui::SetCursorPos({(size.x-ImGui::CalcTextSize("Color Picker").x)*0.5f,y});
    ImGui::Text("Color Picker");
    y+=ImGui::GetTextLineHeightWithSpacing();
    ImGui::SetCursorPos({(size.x-ImGui::CalcTextSize("v1.0.1").x)*0.5f,y});
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.428,0.371,0.529,1.000));
    ImGui::Text("v1.0.0");
    ImGui::PopStyleColor();
    ImGui::Dummy({ImGui::GetContentRegionAvail().x,50});

    const char* key_bindings_help="Ctrl+C - Copy Color To Clipboard\n\nYou can change the default copy format in the settings. The Default Copy format can be changed by selecting and of the drop downs";
    const char* copy_formatting_help=
    "The \"Copy Formatting\" feature in the color picker lets you create custom color formats with placeholders for specific color components.\n\n"
        "You can use formats like\n"
        "\trgba($r,$g,$b,$a)\n" 
        "\thsva($h,$s,$v,$a)\n"
        "\thsla($h,$s,$l,$a)\n\n"
        "Add 'f' at the start for 0 to 1 range conversion,\n"
        "\tFrom: frgba($r,$g,$b,$a)\n"
        "\tTo: rgba(0.256,0.89,0.456,1.000)\n\n"
        "When you copy, placeholders turn into color values.\n";

    const char* color_pallets_help=
        "You can select from a range of predefined beautiful pallets provided.\n\n"
        "Just click on the pallet and then go back to the picker and the pallet will be displayed in pallet section and colors can be selected and modified. \n\n"
        "You can alter the color and then save it using the Save Color button.\n";

    const char* saved_colors_help=
        "These are the colors that are saved using the “save color” button.\n\n"
        "You can view these colors by enabling the saved colors in the view tab of menu bar\n\n"
        "Color can be deleted by right clicking on it.\n";


    ImVec2 s1=ImGui::CalcTextSize(key_bindings_help,0,0,ImGui::GetWindowWidth()-50.0f);
    ImVec2 s2=ImGui::CalcTextSize(copy_formatting_help,0,0,ImGui::GetWindowWidth()-50.0f);
    ImVec2 s3=ImGui::CalcTextSize(color_pallets_help,0,0,ImGui::GetWindowWidth()-50.0f);
    ImVec2 s4=ImGui::CalcTextSize(saved_colors_help,0,0,ImGui::GetWindowWidth()-50.0f);

    RenderCard("##key_bindings",(60.0f+s1.y),ImVec4(0.024,0.839,0.627,1.000),"Key Bindings",key_bindings_help);
    RenderCard("##copy_formatting",(60.0f+s2.y),ImVec4(0.298,0.612,1.000,1.000),"Copy Formatting",copy_formatting_help);
    RenderCard("##color_pallets",(60.0f+s3.y),ImVec4(0.522,1.000,0.510,1.000),"Color Pallets",color_pallets_help);
    RenderCard("##saved_colors",(60.f+s4.y),ImVec4(0.647,0.482,1.000,1.000),"Saved Colors",saved_colors_help);

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
    if(ImGui::IsKeyPressed(ImGuiKey_Escape)){
        showSettings=false;
        showAboutPage=false;
    }
    renderMenuBar();
    if(showSettings){
        renderSettings();
        ImGui::End();
        return;
    }
    if(showAboutPage){
        renderAboutPage();
        ImGui::End();
        return;
    }
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
    if(updateSettings) saveSettings();
}