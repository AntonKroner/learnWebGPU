other dependencies: 
sudo apt install xorg-dev
sudo apt install xcb libxcb-xkb-dev x11-xkb-utils libx11-xcb-dev libxkbcommon-x11-dev

build:
cmake . -B build
cmake --build build
