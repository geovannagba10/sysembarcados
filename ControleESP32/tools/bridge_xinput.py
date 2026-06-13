import argparse
import signal
import sys
import threading
import time

try:
    import pywinusb.hid as hid
except ImportError:
    print("pywinusb nao esta instalado.")
    print("Instale com: py -m pip install pywinusb")
    sys.exit(1)

try:
    import vgamepad as vg
except ImportError:
    print("vgamepad nao esta instalado.")
    print("Instale com: py -m pip install vgamepad")
    sys.exit(1)


TARGET_NAME = "ESP32 Gamepad"
TARGET_VENDOR_ID = 0xE502
TARGET_PRODUCT_ID = 0xBBAB

INPUT_REPORT_ID = 3
RUMBLE_POLL_PERIOD_SECONDS = 0.01
STATUS_PERIOD_SECONDS = 2.0

BUTTON_MASK_1 = 0x01
BUTTON_MASK_2 = 0x02
BUTTON_MASK_3 = 0x04
BUTTON_MASK_4 = 0x08


def describe_device(device):
    product_name = device.product_name or "<sem nome>"
    vendor_name = device.vendor_name or "<sem fabricante>"
    vendor_id = getattr(device, "vendor_id", None)
    product_id = getattr(device, "product_id", None)
    vendor_id_text = "????" if vendor_id is None else f"{vendor_id:04X}"
    product_id_text = "????" if product_id is None else f"{product_id:04X}"

    return (
        f"{product_name} | "
        f"{vendor_name} | "
        f"VID=0x{vendor_id_text} PID=0x{product_id_text}"
    )


def list_devices(devices):
    print("Dispositivos HID encontrados:")

    if not devices:
        print("- nenhum dispositivo HID visivel para o pywinusb")
        return

    for device in devices:
        print("-", describe_device(device))


def find_target_device():
    devices = hid.HidDeviceFilter().get_devices()
    list_devices(devices)

    fallback_device = None

    for device in devices:
        product_name = device.product_name or ""
        vendor_id = getattr(device, "vendor_id", None)
        product_id = getattr(device, "product_id", None)

        if TARGET_NAME.lower() in product_name.lower():
            return device

        if (
            vendor_id == TARGET_VENDOR_ID and
            product_id == TARGET_PRODUCT_ID
        ):
            fallback_device = device

    return fallback_device


def get_report_buffer_length(report):
    if hasattr(report, "get_raw_data"):
        raw_data = report.get_raw_data()

        if raw_data is not None:
            return len(raw_data)

    if hasattr(report, "report_length"):
        report_length = getattr(report, "report_length")

        if report_length is not None:
            return int(report_length)

    return None


def clamp(value, minimum, maximum):
    return max(minimum, min(maximum, value))


def int16_from_le(data, low_index):
    value = data[low_index] | (data[low_index + 1] << 8)

    if value >= 0x8000:
        value -= 0x10000

    return value


def scale_axis_8_to_16(value):
    scaled = int((value / 127.0) * 32767)
    return clamp(scaled, -32768, 32767)


class Esp32XInputBridge:
    def __init__(self, mode):
        self.mode = mode
        self.device = None
        self.output_report = None
        self.output_report_length = None
        self.virtual_gamepad = None
        self.running = False
        self.lock = threading.Lock()
        self.last_rumble_value = None
        self.last_input_timestamp = 0.0

    def open(self):
        self.device = find_target_device()

        if self.device is None:
            print(f"Controle '{TARGET_NAME}' nao encontrado.")
            print("Verifique se o ESP32 esta pareado e conectado.")
            sys.exit(1)

        print("Dispositivo alvo selecionado:")
        print("-", describe_device(self.device))

        self.device.open()
        self.device.set_raw_data_handler(self.handle_raw_input)

        output_reports = self.device.find_output_reports()

        if output_reports:
            self.output_report = output_reports[0]
            self.output_report_length = get_report_buffer_length(
                self.output_report
            )

            print(
                "Output Report encontrado para rumble remoto. "
                f"Report ID: {self.output_report.report_id}"
            )
            print(
                "Tamanho do Output Report: "
                f"{self.output_report_length}"
            )
        else:
            print("Nenhum Output Report encontrado. Sem rumble remoto.")

        self.virtual_gamepad = vg.VX360Gamepad()
        self.virtual_gamepad.register_notification(
            self.handle_virtual_rumble
        )

        self.running = True

    def close(self):
        self.running = False

        if self.virtual_gamepad is not None:
            try:
                self.virtual_gamepad.unregister_notification()
            except Exception:
                pass

        if self.device is not None:
            self.device.close()

    def run(self):
        self.open()

        try:
            print("Ponte iniciada.")
            print(f"Modo atual: {self.mode}")
            print("Pressione Ctrl+C para encerrar.")

            while self.running:
                now = time.time()

                if (
                    self.last_input_timestamp > 0 and
                    now - self.last_input_timestamp >= STATUS_PERIOD_SECONDS
                ):
                    print("Aguardando novos relatorios do ESP32...")
                    self.last_input_timestamp = now

                time.sleep(RUMBLE_POLL_PERIOD_SECONDS)
        finally:
            self.close()

    def handle_raw_input(self, data):
        if not self.running or data is None:
            return

        if len(data) < 6:
            return

        report_id = data[0]

        if report_id != INPUT_REPORT_ID:
            return

        self.last_input_timestamp = time.time()

        buttons = data[1]
        joystick_x = int16_from_le(data, 2)
        joystick_y = int16_from_le(data, 4)

        with self.lock:
            self.apply_input_mapping(
                buttons=buttons,
                joystick_x=joystick_x,
                joystick_y=joystick_y,
            )

    def apply_input_mapping(self, buttons, joystick_x, joystick_y):
        self.virtual_gamepad.reset()

        x16 = scale_axis_8_to_16(joystick_x)
        y16 = scale_axis_8_to_16(joystick_y)

        if self.mode == "racing":
            self.virtual_gamepad.left_joystick(
                x_value=x16,
                y_value=0
            )

            if joystick_y < 0:
                throttle = int(
                    clamp((-joystick_y / 127.0) * 255, 0, 255)
                )
                brake = 0
            else:
                throttle = 0
                brake = int(
                    clamp((joystick_y / 127.0) * 255, 0, 255)
                )

            self.virtual_gamepad.right_trigger(value=throttle)
            self.virtual_gamepad.left_trigger(value=brake)
        else:
            self.virtual_gamepad.left_joystick(
                x_value=x16,
                y_value=y16
            )

        if buttons & BUTTON_MASK_1:
            self.virtual_gamepad.press_button(
                button=vg.XUSB_BUTTON.XUSB_GAMEPAD_A
            )

        if buttons & BUTTON_MASK_2:
            self.virtual_gamepad.press_button(
                button=vg.XUSB_BUTTON.XUSB_GAMEPAD_B
            )

        if buttons & BUTTON_MASK_3:
            self.virtual_gamepad.press_button(
                button=vg.XUSB_BUTTON.XUSB_GAMEPAD_X
            )

        if buttons & BUTTON_MASK_4:
            self.virtual_gamepad.press_button(
                button=vg.XUSB_BUTTON.XUSB_GAMEPAD_Y
            )

        self.virtual_gamepad.update()

    def handle_virtual_rumble(
        self,
        client,
        target,
        large_motor,
        small_motor,
        led_number,
        user_data,
    ):
        del client
        del target
        del led_number
        del user_data

        rumble_value = max(
            int(large_motor),
            int(small_motor)
        )

        if rumble_value == self.last_rumble_value:
            return

        self.last_rumble_value = rumble_value
        self.send_rumble_to_esp32(rumble_value)

    def send_rumble_to_esp32(self, intensity):
        if self.output_report is None or self.output_report_length is None:
            return

        raw_data = [0] * self.output_report_length

        if self.output_report_length > 1:
            raw_data[1] = clamp(intensity, 0, 255)
        else:
            raw_data[0] = clamp(intensity, 0, 255)

        try:
            self.output_report.set_raw_data(raw_data)
            self.output_report.send()
            print(f"Rumble enviado ao ESP32: {intensity}")
        except Exception as error:
            print(f"Falha ao enviar rumble ao ESP32: {error}")


def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Cria uma ponte entre o ESP32 Gamepad via HID e um "
            "controle virtual XInput no Windows."
        )
    )
    parser.add_argument(
        "--mode",
        choices=["gamepad", "racing"],
        default="gamepad",
        help=(
            "gamepad: mapeia o joystick do ESP32 para o stick esquerdo. "
            "racing: usa X para direcao e Y para acelerador/freio."
        ),
    )
    return parser.parse_args()


def main():
    args = parse_args()
    bridge = Esp32XInputBridge(mode=args.mode)

    def stop_bridge(signum, frame):
        del signum
        del frame
        bridge.running = False

    signal.signal(signal.SIGINT, stop_bridge)
    bridge.run()


if __name__ == "__main__":
    main()
