#include "imgui.h"
#include "string"
#include <array>
#include <vector>
#include "ini.h"

class CoreSystem{
	ImVec4 saved_palette[40] = {};
    ImVec4 color;
    ImVec4 pcolor;
    //ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaBar
    ImGuiColorEditFlags picker_flags;
    int default_copy=0;
    bool load_last_color=false;
    bool showPallet=false;
    bool showColors=true;
    bool showShades=true;
    bool showTints=true;
    bool saveState=false;
    bool saveSize=false;
    int p_idx=-1;
    bool showFps=false;
    bool vSync=true;
    bool showMonochromatic=true;
    bool showPrevColor=true;
    bool showAboutPage=false;

    std::vector<ImVec4> usr_colors;
    bool showUsrColors=true;

    void updateShades();
    void renderPallets();
    void renderPicker();
    void renderSettings();
    void renderAboutPage();
    bool Pallet(const char* idx,std::vector<ImVec4>& colors,const bool isDisabled=false);
    std::string rgb_format{"rgba($r,$g,$b,$a)"};
    std::string hsv_format{"hsva($h,$s%,$v%,$a)"};
    std::string hsl_format{"hsla($h,$s%,$l%,$a)"};
    std::array<ImVec4,5> shades;

    //Save Config
    mINI::INIFile* file{0};
    mINI::INIStructure ini;
    std::string usrRootDir{0};
    bool updateSettings=false;
    void saveSettings();
    bool showSettings=false;

public:
    bool isRunning=true;
    int font_size=16;
    bool buildFonts=false;
    float width{0.0f};
    float height{0.0f};
	CoreSystem();
	void render();
	void renderMenuBar();
};