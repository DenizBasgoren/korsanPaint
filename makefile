	
build:
	echo "We need root to install. Password:"
	su
	g++ main.cpp -lSDL2 -lSDL2_gfx -lSDL2_ttf -lSDL2_image -o /usr/bin/korsanPaint
	cp ./resources/KorsanPaint.desktop /usr/share/applications/
	mkdir /usr/share/korsanPaint
	cp ./resources/icon.png /usr/share/korsanPaint
	cp ./resources/NotoSans-Bold.ttf /usr/share/korsanPaint
	echo "Put following command to Settings > Keyboard > Shortcuts > New"
	echo "'xfce4-screenshooter -f -o korsanPaint'"
	echo "Save with shortcut ctrl+alt+K (or anything else)"
	