import os
import json

# Import môi trường PlatformIO
Import("env")

def get_version():
    # Lấy phiên bản từ biến môi trường hoặc dùng phiên bản mặc định
    try:
        version = env['BUILD_VERSION']
    except KeyError:
        version = "1.0.0"
    return version



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

    print(f"\nĐã chọn target: {target_name}")
    print(f"Phiên bản: {target_version}")
    print(f"Tệp phần cứng: {target_part_path}")

    # Đọc nội dung của file part của target đã chọn
    try:
        with open(target_part_path, "r") as f:
            target_part_data = json.load(f)
    except FileNotFoundError:
        print(f"Tệp cấu hình '{target_part_path}' không tồn tại, dừng quá trình build.")
        env.Exit(1)

    # Lấy đường dẫn file .bin từ môi trường build
    bin_path = env.subst("$BUILD_DIR/${PROGNAME}.bin")

    defines = json.JSONEncoder().encode(env['OPTIONS_JSON'])

    with open(bin_path, "ab") as bin_file:
        defines = (defines.encode() + (b'\0' * 512))[0:512]
        bin_file.write(defines)
        layout = (json.JSONEncoder().encode(target_part_data).encode() + (b'\0' * 2048))[0:2048]
        bin_file.write(layout)
        print(f"Đã thêm thông tin từ '{target_part_path}' vào cuối {bin_path}")

# Thêm hành động Post-Build để ghi thông tin vào file .bin
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", Config_hardware_info)
# Đường dẫn tới tệp targets.json
targets_json_path = os.path.join(env['PROJECT_DIR'], "hardware", "targets.json")
hardware_json_path = os.path.join(env['PROJECT_DIR'], "hardware", "hardware.json")


try:
    os.remove(env['PROJECT_BUILD_DIR'] + '/' + env['PIOENV'] +'/'+ env['PROGNAME'] + '.bin')
except FileNotFoundError:
    None
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
