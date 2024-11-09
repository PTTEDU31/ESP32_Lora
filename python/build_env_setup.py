import os
import json
import struct
import sys

# Import môi trường PlatformIO
Import("env")

def get_version():
    # Lấy phiên bản từ biến môi trường hoặc dùng phiên bản mặc định
    try:
        version = env['BUILD_VERSION']
    except KeyError:
        version = "1.0.0"
    return version

def findFirmwareEnd(f):
    f.seek(0, 0)
    (magic, segments, _, _, _) = struct.unpack('<BBBBI', f.read(8))
    if magic != 0xe9:
        sys.stderr.write('The file provided does not the right magic for a firmware file!\n')
        exit(1)

    is8285 = False
    if segments == 2: # giả định là ESP8266/85
        f.seek(0x1000, 0)
        (magic, segments, _, _, _) = struct.unpack('<BBBBI', f.read(8))
        is8285 = True
    else:
        f.seek(24, 0)

    for _ in range(segments):
        (_, size) = struct.unpack('<II', f.read(8))
        f.seek(size, 1)

    pos = f.tell()
    pos = (pos + 16) & ~15
    if not is8285:
        pos = pos + 32
    return pos

# Hàm ghi thông tin vào cuối file .bin
def Config_hardware_info(source, target, env):
    # Hiển thị danh sách các target cho người dùng lựa chọn
    print("Chọn target (nhập số):")
    for i, target in enumerate(targets):
        print(f"{i+1}. {target['name']} - Phiên bản: {target['version']}")

    # Nhận lựa chọn từ người dùng, sử dụng target đầu tiên nếu không có lựa chọn
    choice = input("Nhập lựa chọn (hoặc bấm Enter để dùng mặc định): ")
    selected_target = targets[0] if not choice else targets[int(choice) - 1]

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

    # Đọc nội dung của file part của target đã chọn
    try:
        with open(target_part_path, "r") as f:
            target_part_data = json.load(f)
    except FileNotFoundError:
        print(f"Tệp cấu hình '{target_part_path}' không tồn tại, dừng quá trình build.")
        env.Exit(1)

    # Lấy đường dẫn file .bin từ môi trường build
    bin_path = env.subst("$BUILD_DIR/${PROGNAME}.bin")

    # Tìm vị trí cuối của firmware bằng cách dùng findFirmwareEnd
    with open(bin_path, "rb+") as bin_file:
        firmware_end = findFirmwareEnd(bin_file)
        bin_file.seek(firmware_end)

        # Chuẩn bị dữ liệu để ghi vào cuối firmware
        product_name_encoded = (product_name.encode() + (b'\0' * 128))[:128]
        lua_name_encoded = (lua_name.encode() + (b'\0' * 16))[:16]
        defines = json.JSONEncoder().encode(env['OPTIONS_JSON'])
        defines = (defines.encode() + (b'\0' * 512))[0:512]
        layout = (json.JSONEncoder().encode(target_part_data).encode() + (b'\0' * 2048))[0:2048]

        # Ghi dữ liệu vào đúng phần cuối của firmware
        bin_file.write(product_name_encoded)
        bin_file.write(lua_name_encoded)
        bin_file.write(defines)
        bin_file.write(layout)
        print(f"Đã thêm thông tin từ '{target_part_path}' vào cuối {bin_path}")

# Thêm hành động Post-Build để ghi thông tin vào file .bin
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", Config_hardware_info)

# Đường dẫn tới tệp targets.json
targets_json_path = os.path.join(env['PROJECT_DIR'], "hardware", "targets.json")
hardware_json_path = os.path.join(env['PROJECT_DIR'], "hardware", "hardware.json")

# Xóa file .bin cũ (nếu có) trước khi build
try:
    os.remove(env['PROJECT_BUILD_DIR'] + '/' + env['PIOENV'] + '/' + env['PROGNAME'] + '.bin')
except FileNotFoundError:
    pass

# Đọc tệp targets.json và lấy danh sách các target
try:
    with open(targets_json_path, "r") as f:
        targets_data = json.load(f)
        targets = targets_data.get("target", [])
except FileNotFoundError:
    print("File 'targets.json' không tồn tại, dừng quá trình build.")
    env.Exit(1)

# In thông tin môi trường build và phiên bản
platform = env.get('PIOPLATFORM', '')
build_env = env.get('PIOENV', '').upper()
print("PLATFORM : '%s'" % platform)
print("BUILD ENV: '%s'" % build_env)
print("Build Version: %s\n\n" % get_version())
