import requests
from bs4 import BeautifulSoup

file=open("index.html","r")
soup=BeautifulSoup(file.read(),"html.parser")
file.close();
pallets=soup.find_all(class_="palette-card_colors")

header=open('./src/pallet.h','w');
header.write("#include <vector>\n");
header.write("#include \"imgui.h\"\n");
header.write("#ifndef PALLET_H\n");
header.write("#define PALLET_H\n");
header.write("std::vector<std::vector<ImVec4>> pallets={\n");
# header.write("{\n");
pcount=0;
for pallet in pallets:
	divs=pallet.find_all("div")
	header.write("\t{\n");
	dcount=0;
	for div in divs:
		style=div.get("style")
		if style:
			color=style.split('background:')[1].split(';')[0].strip()[4:-1].replace(',','')
			r=0.0
			g=0.0
			b=0.0
			x=color.split(' ')
			r=round(int(x[0])/255,3)
			g=round(int(x[1])/255,3)
			b=round(int(x[2])/255,3)
			header.write(f"\t\tImVec4({r},{g},{b},1.000)\n") if(dcount==(len(divs)-1)) else header.write(f"\t\tImVec4({r},{g},{b},1.000),\n")
			# print(f"Color: {r} {g} {b}")
			dcount+=1


	header.write("\t}\n") if(pcount==(len(pallets)-1)) else header.write("\t},\n")
	pcount+=1;

header.write("};\n")
header.write("#endif\n")

