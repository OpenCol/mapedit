#!/usr/bin/env python3
"""Build the release downloads: the macOS application and the Windows program, zipped.

    python3 tools/release.py [--mac] [--win] [--out dist]

macOS (on a Mac with Xcode's command line tools and CMake): SDL3 is built from source as a
static, universal (arm64 + x86_64) library for macOS 11 and later, and linked into
"Colonization Map Editor.app", which is then signed ad hoc (not notarized: Gatekeeper asks
once, right-click > Open). Out: mapedit-<version>-macos.zip.

Windows (cross-built with MinGW-w64): against SDL3's MinGW development package, downloaded
once; mapedit.exe and SDL3.dll. Out: mapedit-<version>-windows-x64.zip.

Each zip also holds README.txt (how to start it), LICENSE and NOTICE.md. The version is
CMakeLists.txt's. Work happens in build-release/ (ignored); nothing is uploaded.
"""
import argparse, os, re, shutil, subprocess, sys, tarfile, urllib.request, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, 'build-release')
SDL_VERSION = '3.2.24'
SDL_GIT = 'https://github.com/libsdl-org/SDL.git'
SDL_MINGW = ('https://github.com/libsdl-org/SDL/releases/download/release-%s/SDL3-devel-%s-mingw.tar.gz'
             % (SDL_VERSION, SDL_VERSION))
MACOS_MIN = '11.0'

README = """Colonization Map Editor {version} ({platform})

The map editor that came with Sid Meier's Colonization for DOS (MicroProse, 1994), running
natively, built from a reconstruction of its source. It behaves as the 1994 editor did and
writes the same .MP map files.

You need your own copy of Colonization for DOS (GOG or Steam): the editor reads the game's
art, fonts and palette from the game's folder and saves its maps there. None of the game is
included here.

{start}

Keys and mouse are the original's: Shift + left click paints, Shift + right click picks up
a square's terrain; the arrows, number keys or keypad move the cursor; Space paints,
Backspace picks up; Alt opens the menu bar (Alt, S saves); Z and X zoom; Esc quits.

Back up your maps: the editor saves over the .MP file it opened, as it always did.

Source, issues and the full story: {url}
"""

START_MAC = """To start: open "Colonization Map Editor.app". The first time, macOS says it cannot check
the app (it is not notarized): right-click it, choose Open, then Open again. The editor asks
for your Colonization folder (the one with VICEROY.EXE in it) and remembers it. To pick
another folder later, start it from Terminal with --choose-game:

    "Colonization Map Editor.app/Contents/MacOS/Colonization Map Editor" --choose-game

Or put the app inside the game's folder: it then finds the game by itself."""

START_WIN = """To start: copy mapedit.exe and SDL3.dll into your Colonization folder (the one with
VICEROY.EXE in it) and double-click mapedit.exe. Or run it from anywhere: it asks for the
game's folder the first time and remembers it (mapedit.exe --choose-game asks again;
mapedit.exe --game C:\\GOG Games\\Colonization uses that folder)."""


def run(cmd, **kw):
    print('+', ' '.join(cmd) if isinstance(cmd, list) else cmd)
    subprocess.run(cmd, check=True, **kw)


def version():
    m = re.search(r'project\(mapedit_port VERSION ([0-9.]+)', open(os.path.join(ROOT, 'CMakeLists.txt')).read())
    return m.group(1)


def write_zip(out, files):
    """files: [(path on disk, name in the zip)]; directories go in whole, keeping modes"""
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for src, name in files:
            if os.path.isdir(src):
                for dp, _, fs in os.walk(src):
                    for f in sorted(fs):
                        p = os.path.join(dp, f)
                        info = zipfile.ZipInfo.from_file(p, os.path.join(name, os.path.relpath(p, src)))
                        info.compress_type = zipfile.ZIP_DEFLATED
                        z.writestr(info, open(p, 'rb').read())
            else:
                z.write(src, name)
    print('wrote', out)


def common(tmp, platform, start, url):
    readme = os.path.join(tmp, 'README.txt')
    text = README.format(version=version(), platform=platform, start=start, url=url)
    open(readme, 'w', newline='\r\n' if platform.startswith('Windows') else '\n').write(text)
    return [(readme, 'README.txt'), (os.path.join(ROOT, 'LICENSE'), 'LICENSE.txt'),
            (os.path.join(ROOT, 'NOTICE.md'), 'NOTICE.md')]


def mac(out, url):
    src = os.path.join(WORK, 'SDL-' + SDL_VERSION)
    prefix = os.path.join(WORK, 'sdl-macos')
    if not os.path.exists(os.path.join(prefix, 'lib', 'libSDL3.a')):
        if not os.path.exists(src):
            run(['git', 'clone', '--depth', '1', '--branch', 'release-' + SDL_VERSION, SDL_GIT, src])
        run(['cmake', '-S', src, '-B', os.path.join(src, 'build'), '-DCMAKE_BUILD_TYPE=Release',
             '-DSDL_SHARED=OFF', '-DSDL_STATIC=ON', '-DSDL_TESTS=OFF', '-DSDL_EXAMPLES=OFF',
             '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64', '-DCMAKE_OSX_DEPLOYMENT_TARGET=' + MACOS_MIN,
             '-DCMAKE_INSTALL_PREFIX=' + prefix])
        run(['cmake', '--build', os.path.join(src, 'build'), '--target', 'install', '-j8'])
    b = os.path.join(WORK, 'macos')
    run(['cmake', '-S', ROOT, '-B', b, '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_PREFIX_PATH=' + prefix,
         '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64', '-DCMAKE_OSX_DEPLOYMENT_TARGET=' + MACOS_MIN])
    run(['cmake', '--build', b, '--target', 'mapedit_app', '-j8'])
    app = os.path.join(b, 'Colonization Map Editor.app')
    exe = os.path.join(app, 'Contents', 'MacOS', 'Colonization Map Editor')
    deps = subprocess.run(['otool', '-L', exe], capture_output=True, text=True, check=True).stdout
    if 'SDL' in deps or '/opt/homebrew' in deps or '/usr/local' in deps:
        sys.exit('the app still links a library outside macOS:\n' + deps)
    run(['codesign', '--force', '--deep', '--sign', '-', app])
    tmp = os.path.join(WORK, 'macos-stage')
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(tmp)
    z = os.path.join(out, 'mapedit-%s-macos.zip' % version())
    # ditto keeps the bundle's signature, links and modes as Finder expects them
    stage = os.path.join(tmp, 'Colonization Map Editor')
    os.makedirs(stage)
    run(['ditto', app, os.path.join(stage, 'Colonization Map Editor.app')])
    for src, name in common(tmp, 'macOS 11 or later, Apple silicon and Intel', START_MAC, url):
        shutil.copy(src, os.path.join(stage, name))
    if os.path.exists(z):
        os.remove(z)
    run(['ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', stage, z])
    return z


def win(out, url):
    sdl = os.path.join(WORK, 'SDL3-%s-mingw' % SDL_VERSION)
    if not os.path.exists(sdl):
        os.makedirs(sdl)
        tgz = os.path.join(sdl, 'sdl.tar.gz')
        print('+ download', SDL_MINGW)
        urllib.request.urlretrieve(SDL_MINGW, tgz)
        with tarfile.open(tgz) as t:
            t.extractall(sdl, filter='data')
    tree = os.path.join(sdl, 'SDL3-%s' % SDL_VERSION, 'x86_64-w64-mingw32')
    b = os.path.join(WORK, 'windows')
    run(['cmake', '-S', ROOT, '-B', b, '-DCMAKE_BUILD_TYPE=Release',
         '-DCMAKE_TOOLCHAIN_FILE=' + os.path.join(ROOT, 'cmake', 'mingw-w64-x86_64.cmake'),
         '-DCMAKE_PREFIX_PATH=' + tree, '-DCMAKE_FIND_ROOT_PATH=' + tree,
         '-DCMAKE_EXE_LINKER_FLAGS=-static-libgcc'])
    run(['cmake', '--build', b, '--target', 'mapedit', '-j8'])
    run(['x86_64-w64-mingw32-strip', os.path.join(b, 'mapedit.exe')])
    tmp = os.path.join(WORK, 'windows-stage')
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(tmp)
    folder = 'Colonization Map Editor'
    files = [(os.path.join(b, 'mapedit.exe'), folder + '/mapedit.exe'),
             (os.path.join(b, 'SDL3.dll'), folder + '/SDL3.dll')]
    files += [(src, folder + '/' + name) for src, name in common(tmp, 'Windows, 64-bit', START_WIN, url)]
    z = os.path.join(out, 'mapedit-%s-windows-x64.zip' % version())
    write_zip(z, files)
    return z


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--mac', action='store_true')
    ap.add_argument('--win', action='store_true')
    ap.add_argument('--out', default=os.path.join(ROOT, 'dist'))
    ap.add_argument('--url', default='https://github.com/OpenCol/mapedit')
    a = ap.parse_args()
    if not a.mac and not a.win:
        a.mac = a.win = True
    os.makedirs(a.out, exist_ok=True)
    made = []
    if a.mac:
        made.append(mac(a.out, a.url))
    if a.win:
        made.append(win(a.out, a.url))
    for z in made:
        print('%s  %.1f MB' % (z, os.path.getsize(z) / 1e6))


if __name__ == '__main__':
    main()
