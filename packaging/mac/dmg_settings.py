# dmgbuild settings for the installer disk image (used by packaging/make_mac.sh).
# Expects -D app=<path to .app> -D icon=<.icns> -D background=<png>.
import os.path

app = defines['app']  # noqa: F821 (provided by dmgbuild)
app_name = os.path.basename(app)

format = 'ULFO'
filesystem = 'HFS+'
files = [app]
symlinks = {'Applications': '/Applications'}
icon = defines['icon']  # noqa: F821
background = defines['background']  # noqa: F821 (background@2x.png is picked up too)

# The size is the window's content area: exactly the 640x480 artwork.
window_rect = ((200, 120), (640, 480))
default_view = 'icon-view'
icon_size = 80
text_size = 13
show_icon_preview = False
icon_locations = {
    app_name: (245, 225),
    'Applications': (395, 225),
}
