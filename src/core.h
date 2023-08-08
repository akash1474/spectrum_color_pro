#include "imgui.h"
#include "string"
#include <array>

class CoreSystem{
	ImVec4 saved_palette[40] = {};
    ImVec4 color;
    bool showPallet=true;
    bool showFps=false;
    bool vSync=true;
    bool showHelp{false};
    void updateShades();
    std::string rgb_format{"fImVec4($r,$g,$b,$a)"};
    std::string hsv_format{"hsla($h,$s%,$l%,$a)"};
    std::array<ImVec4,5> shades;
public:
    float width{0.0f};
    float height{0.0f};
	CoreSystem():color(ImVec4(1.0f, 0.0f, 1.0f, 1.0f)){
		updateShades();
	}
	void render();
	void renderMenuBar();
};