import PIL.Image
import os

ALT_POS = {
    "idle": (0, 3),
    "move": (4, 6),
    "kick": (10, 3),
    "hurt": (13, 4),
    "dash": (18, 6),
}

for sex in os.listdir("_assets/download"):
    for name in os.listdir(f"_assets/download/{sex}"):
        result = PIL.Image.new("RGBA", (24 * 6, 24 * 9))
        for idx, anim in enumerate(["idle", "move", "kick", "bite", "dash", "jump", "avoid", "hurt", "dead"]):
            try:
                img = PIL.Image.open(f"_assets/download/{sex}/{name}/base/{anim}.png")
            except:
                img = PIL.Image.open(f"_assets/dinoCharactersVersion1.1/sheets/DinoSprites - {name}.png")
                pos = ALT_POS[anim]
                img = img.crop((pos[0] * 24, 0, (pos[0] + pos[1]) * 24, 24))
            for x in range(0, img.size[0], 24):
                img.paste(img.crop((x, 0, x + 24, 24)).transpose(PIL.Image.FLIP_LEFT_RIGHT), (x, 0))
            result.paste(img, (0, idx * 24))
        result.save(f"resources/sprites/dino/{name}_{sex}.png")
