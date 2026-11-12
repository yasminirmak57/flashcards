"""Builds the app icon from the lace photo in icon/ (any .png except icon.png): zoom into the centre,
cut it to the rounded macOS shape, and write icon/icon.png (+ .ico on Windows)."""
import glob, sys
from PIL import Image, ImageDraw

photo = sorted(p for p in glob.glob("icon/*.png") if not p.endswith("icon.png"))[0]
src = Image.open(photo).convert("RGB")
w, h = src.size
s = int(min(w, h) * 0.6)                      # zoom in (60% of the photo)
x, y = (w - s) // 2 + 30, (h - s) // 2 - 40   # keep away from the corner badge
crop = src.crop((x, y, x + s, y + s)).resize((824, 824), Image.LANCZOS)

icon = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
mask = Image.new("L", (824, 824), 0)
ImageDraw.Draw(mask).rounded_rectangle((0, 0, 823, 823), radius=185, fill=255)
icon.paste(crop, (100, 100), mask)
icon.resize((512, 512), Image.LANCZOS).save("icon/icon.png")
if "--ico" in sys.argv:
    icon.save("icon/flashcards.ico", sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
print("icon ready")
