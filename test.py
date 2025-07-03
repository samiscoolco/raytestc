import pygame
import struct

VSWAP_FILE = "VSWAP.WL6"
PAL_FILE = "wolf.pal"
TEXTURE_WIDTH = 64
TEXTURE_HEIGHT = 64
TEXTURE_SIZE = TEXTURE_WIDTH * TEXTURE_HEIGHT
SCREEN_SIZE = (512, 512)

def load_jasc_pal(filename):
    with open(filename, "r") as f:
        lines = f.readlines()

    if lines[0].strip() != "JASC-PAL":
        raise ValueError("Invalid palette header")
    if lines[1].strip() != "0100":
        raise ValueError("Unsupported version")

    num_colors = int(lines[2].strip())
    if num_colors != 256:
        raise ValueError("Expected 256 colors")

    palette = []
    for line in lines[3:]:
        r, g, b = map(int, line.strip().split())
        palette.append((r, g, b))

    return palette

def read_vswap_header(filename):
    with open(filename, "rb") as f:
        num_chunks, sprite_start, sound_start = struct.unpack("<HHH", f.read(6))
        offsets = struct.unpack("<%dL" % num_chunks, f.read(4 * num_chunks))
        f.read(2 * num_chunks)  # skip sizes
    return offsets, sprite_start

def read_texture(f, offset):
    f.seek(offset)
    return f.read(TEXTURE_SIZE)

def render_texture(data, palette):
    surf = pygame.Surface((TEXTURE_WIDTH, TEXTURE_HEIGHT))
    for y in range(TEXTURE_HEIGHT):
        for x in range(TEXTURE_WIDTH):
            val = ord(data[x * TEXTURE_WIDTH + y])
            surf.set_at((x, y), palette[val])
    return surf

def show_textures_from_vswap():
    pygame.init()
    screen = pygame.display.set_mode(SCREEN_SIZE)
    pygame.display.set_caption("Wolf3D VSWAP Texture Viewer")

    palette = load_jasc_pal(PAL_FILE)

    with open(VSWAP_FILE, "rb") as f:
        offsets, sprite_start = read_vswap_header(VSWAP_FILE)
        current = 0

        while True:
            screen.fill((0, 0, 0))

            while current < sprite_start:
                if offsets[current] == 0xFFFFFFFF:
                    current += 1
                    continue
                data = read_texture(f, offsets[current])
                if len(data) == TEXTURE_SIZE:
                    break
                current += 1

            surf = render_texture(data, palette)
            pygame.transform.scale(surf, SCREEN_SIZE, screen)
            
            pygame.display.flip()

            for event in pygame.event.get():
                if event.type == pygame.KEYDOWN and event.key == pygame.K_SPACE:
                    current += 1
                    print("Showing texture #%d" % current)
                    break
                elif event.type == pygame.QUIT:
                    pygame.quit()
                    return

show_textures_from_vswap()
