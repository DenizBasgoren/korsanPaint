# Maintainer: Deniz Basgoren <denizbasgoren1997@gmail.com>
pkgname="KorsanPaint"
pkgver="1"
pkgrel=1
epoch=1
pkgdesc="Paint program for drawing quickly"
arch=('any')
url="https://github.com/denizbasgoren/korsanpaint"
license=('Unlicense')
groups=()
depends=('glibc' 'sdl2' 'sdl2_gfx' 'sdl2_ttf')
makedepends=('gcc')
checkdepends=()
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("git+https://github.com/denizBasgoren/korsanPaint.git")
noextract=()
md5sums=()
validpgpkeys=()

package() {
	cd korsanPaint
	g++ main.cpp -lSDL2 -lSDL2_gfx -lSDL2_ttf -o korsanPaint
	mv korsanPaint /usr/bin
	mv KorsanPaint.desktop /usr/share/applications/
	mkdir /usr/share/korsanPaint
	mv icon.png /usr/share/korsanPaint
	mv NotoSans-Bold.ttf /usr/share/korsanPaint
}
