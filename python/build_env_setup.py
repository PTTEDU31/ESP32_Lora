import os
import json
import struct
import sys

# Import môi trường PlatformIO
Import("env")

def get_version():
    """
    Lấy phiên bản từ biến môi trường hoặc dùng phiên bản mặc định.
    """
    try:
        return env['BUILD_VERSION']
    except KeyError:
        return "1.0.0"

def find_firmware_end(f):
    """
    Tìm vị trí cuối của firmware file.
    """
    f.seek(0, 0)
    (magic, segments, _, _, _) = struct.unpack('<BBBBI', f.read(8))
    if magic != 0xe9:
        sys.stderr.write("The file provided does not have the right magic for a firmware file!\n")
        exit(1)

    is_8285 = False
    if segments == 2:  # giả định là ESP8266/85
        f.seek(0x1000, 0)
        (magic, segments, _, _, _) = struct.unpack('<BBBBI', f.read(8))
        is_8285 = True
    else:
        f.seek(24, 0)

    for _ in range(segments):
        (_, size) = struct.unpack('<II', f.read(8))
        f.seek(size, 1)

    pos = f.tell()
    pos = (pos + 16) & ~15
    if not is_8285:
        pos += 32
    return pos

def detect_chip_type(env_name):
    """
    Xác định loại chip từ tên môi trường hoặc target.
    """
    env_name = env_name.lower()
    if "esp32s3" in env_name:
        return "esp32-s3"
    elif "esp32c3" in env_name:
        return "esp32-c3"
    else:
        return "esp32"

def config_hardware_info(source, target, env):
    """
    Ghi thông tin phần cứng vào cuối file .bin.
    """
    # Lấy platform từ tên môi trường (PIOENV)
    current_env = env.get('PIOENV', '').lower()
    current_platform = detect_chip_type(current_env)

    # Lọc danh sách targets dựa trên platform
    filtered_targets = [t for t in targets if t.get('platform', '').lower() == current_platform]

    if not filtered_targets:
        print(f"Không có target nào phù hợp với platform '{current_platform}'.")
        env.Exit(1)

    # Hiển thị danh sách target để người dùng chọn
    print("Chọn target (nhập số):")
    for i, target in enumerate(filtered_targets):
        print(f"{i+1}. {target['name']} - Phiên bản: {target['version']}")

    choice = input("Nhập lựa chọn (hoặc bấm Enter để dùng mặc định): ")
    selected_target = filtered_targets[0] if not choice else filtered_targets[int(choice) - 1]

    # Lấy thông tin từ target đã chọn
    target_name = selected_target["name"]
    target_version = selected_target["version"]
    target_part_path = os.path.join(env['PROJECT_DIR'], "hardware", selected_target["part"])
    product_name = selected_target.get("product_name", "Unknown Product")
    lua_name = selected_target.get("lua_name", "Unknown Lua")

    print(f"\nĐã chọn target: {target_name}")
    print(f"Phiên bản: {target_version}")
    print(f"Tệp phần cứng: {target_part_path}")
    print(f"Product Name: {product_name}")
    print(f"Lua Name: {lua_name}")

    # Đọc nội dung tệp part của target
    try:
        with open(target_part_path, "r") as f:
            target_part_data = json.load(f)
    except FileNotFoundError:
        print(f"Tệp cấu hình '{target_part_path}' không tồn tại, dừng quá trình build.")
        env.Exit(1)

    # Lấy đường dẫn file .bin
    bin_path = env.subst("$BUILD_DIR/${PROGNAME}.bin")

    # Ghi thông tin vào cuối firmware
    with open(bin_path, "rb+") as bin_file:
        firmware_end = find_firmware_end(bin_file)
        bin_file.seek(firmware_end)

        # Chuẩn bị dữ liệu để ghi
        product_name_encoded = (product_name.encode() + (b'\0' * 128))[:128]
        lua_name_encoded = (lua_name.encode() + (b'\0' * 16))[:16]
        defines = json.JSONEncoder().encode(env['OPTIONS_JSON'])
        defines = (defines.encode() + (b'\0' * 1024))[:1024]
        layout = (json.JSONEncoder().encode(target_part_data).encode() + (b'\0' * 2048))[:2048]

        bin_file.write(product_name_encoded)
        bin_file.write(lua_name_encoded)
        bin_file.write(defines)
        bin_file.write(layout)
        print(f"Đã thêm thông tin từ '{target_part_path}' vào cuối {bin_path}")

# Thêm hành động Post-Build
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", config_hardware_info)

# Đường dẫn tệp targets.json
targets_json_path = os.path.join(env['PROJECT_DIR'], "hardware", "targets.json")

# Xóa file .bin cũ (nếu có) trước khi build
try:
    os.remove(env['PROJECT_BUILD_DIR'] + '/' + env['PIOENV'] + '/' + env['PROGNAME'] + '.bin')
except FileNotFoundError:
    pass

# Đọc danh sách targets từ tệp targets.json
try:
    with open(targets_json_path, "r") as f:
        targets_data = json.load(f)
        targets = targets_data.get("target", [])
except FileNotFoundError:
    print("File 'targets.json' không tồn tại, dừng quá trình build.")
    env.Exit(1)

# In thông tin môi trường build
platform = env.get('PIOPLATFORM', '')
build_env = env.get('PIOENV', '').upper()
print("PLATFORM : '%s'" % platform)
print("BUILD ENV: '%s'" % build_env)
print("Build Version: %s\n" % get_version())
