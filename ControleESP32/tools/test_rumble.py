import sys
import time

try:
    import pywinusb.hid as hid
except ImportError:
    print("pywinusb nao esta instalado.")
    print("Instale com: py -m pip install pywinusb")
    sys.exit(1)


TARGET_NAME = "ESP32 Gamepad"
TARGET_VENDOR_ID = 0xE502
TARGET_PRODUCT_ID = 0xBBAB
RUMBLE_ON = 255
RUMBLE_OFF = 0
RUMBLE_DURATION_SECONDS = 2


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


def main():
    device = find_target_device()

    if device is None:
        print(f"Controle '{TARGET_NAME}' nao encontrado.")
        print("Verifique se o ESP32 esta pareado e conectado.")
        print(
            "Se o controle nao apareceu na lista acima, "
            "o pywinusb provavelmente nao esta enxergando esse BLE HID."
        )
        sys.exit(1)

    print("Dispositivo alvo selecionado:")
    print("-", describe_device(device))
    device.open()

    try:
        reports = device.find_output_reports()

        if not reports:
            print("Nenhum Output Report encontrado para este dispositivo.")
            print("O Windows pode nao estar expondo o report de rumble.")
            sys.exit(1)

        report = reports[0]
        report_length = get_report_buffer_length(report)

        if report_length is None:
            print("Nao foi possivel descobrir o tamanho do Output Report.")
            print("Atributos disponiveis no objeto HidReport:")
            print(sorted(dir(report)))
            sys.exit(1)

        print(f"Output Report encontrado. Report ID: {report.report_id}")
        print(f"Tamanho do buffer: {report_length}")

        rumble_on_buffer = [0] * report_length

        if report_length > 1:
            rumble_on_buffer[1] = RUMBLE_ON
            rumble_index = 1
        else:
            rumble_on_buffer[0] = RUMBLE_ON
            rumble_index = 0

        print(
            "Enviando rumble maximo "
            f"(indice {rumble_index}, valor {RUMBLE_ON})..."
        )
        report.set_raw_data(rumble_on_buffer)
        report.send()

        time.sleep(RUMBLE_DURATION_SECONDS)

        rumble_off_buffer = [0] * report_length
        print("Enviando rumble zero...")
        report.set_raw_data(rumble_off_buffer)
        report.send()

        print("Teste concluido.")
    finally:
        device.close()


if __name__ == "__main__":
    main()
