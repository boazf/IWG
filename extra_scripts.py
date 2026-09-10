Import("env")

import os

index_html = os.path.join(
    env.subst("$PROJECT_DIR"),
    "sd",
    "wwwroot",
    "index.htm"
)

index_obj = os.path.join(
    env.subst("$BUILD_DIR"),
    "index.o"
)

index_node = env.Command(
    target=index_obj,
    source=index_html,
    action="xtensa-esp32-elf-objcopy -I binary -O elf32-xtensa-le -B xtensa $SOURCE $TARGET"
)

# Add generated object as a real linker input
env.Append(
    PIOBUILDFILES=index_node
)
