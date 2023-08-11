#include "imgui.h"
#include "string"
#include <array>
#include <vector>

class CoreSystem{
	ImVec4 saved_palette[40] = {};
    ImVec4 color;
    bool showPallet=false;
    bool showColors=true;
    bool showShades=true;
    bool showTints=true;
    size_t p_idx=-1;
    bool showFps=false;
    bool vSync=true;
    bool showHelp{false};
    void updateShades();
    void renderPallets();
    void renderPicker();
    bool Pallet(const char* idx,std::vector<ImVec4>& colors,const bool isDisabled=false);
    std::string rgb_format{"fImVec4($r,$g,$b,$a)"};
    std::string hsv_format{"hsla($h,$s%,$l%,$a)"};
    std::array<ImVec4,5> shades;
public:
    float width{0.0f};
    float height{0.0f};
	CoreSystem():color(ImVec4(0.432,0.173,0.691,1.000)){
		updateShades();
	}
	void render();
	void renderMenuBar();
};