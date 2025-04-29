Import("env")

# Source text file
text_file = "sd/wwwroot/index.htm"

# Output object file
output_obj = env["BUILD_DIR"] + "\\" + "index.o"

# Generate an object file from the text
env.Execute(
    f"xtensa-esp32-elf-objcopy -I binary -O elf32-xtensa-le -B xtensa {text_file} {output_obj}"
)

env.Append(LINKFLAGS=output_obj)