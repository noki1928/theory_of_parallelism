import argparse
import logging
import queue
import signal
import threading
from pathlib import Path

import cv2
import numpy as np

from classes import SensorX, SensorCam, WindowImage


LOG_DIR = Path(__file__).resolve().parents[1] / "log"
LOG_DIR.mkdir(exist_ok=True)
logging.basicConfig(
    filename=LOG_DIR / "task4.log",
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(threadName)s: %(message)s",
)


def parse_resolution(value):
    try:
        width_text, height_text = value.lower().split("x", 1)
        width, height = int(width_text), int(height_text)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("resolution must be WIDTHxHEIGHT, e.g. 1280x720") from exc
    if width <= 0 or height <= 0:
        raise argparse.ArgumentTypeError("resolution values must be positive")
    return width, height


def put_latest(out_queue, value):
    try:
        out_queue.put_nowait(value)
    except queue.Full:
        try:
            out_queue.get_nowait()
        except queue.Empty:
            pass
        out_queue.put_nowait(value)


def sensor_loop(sensor, out_queue, stop_event):
    while not stop_event.is_set():
        try:
            put_latest(out_queue, sensor.get())
        except Exception:
            logging.exception("Sensor loop failed")
            stop_event.set()


def drain_latest(in_queue, previous):
    latest = previous
    while True:
        try:
            latest = in_queue.get_nowait()
        except queue.Empty:
            return latest


def compose_image(frame, sensor_values):
    if frame is None:
        frame = np.zeros((480, 640, 3), dtype=np.uint8)

    image = frame.copy()
    panel_w, panel_h = 190, 100
    x0 = max(0, image.shape[1] - panel_w - 12)
    y0 = max(0, image.shape[0] - panel_h - 12)
    cv2.rectangle(image, (x0, y0), (x0 + panel_w, y0 + panel_h), (255, 255, 255), -1)
    cv2.rectangle(image, (x0, y0), (x0 + panel_w, y0 + panel_h), (0, 0, 0), 1)

    for idx, value in enumerate(sensor_values):
        text = f"Sensor{idx}: {value if value is not None else '-'}"
        cv2.putText(image, text, (x0 + 12, y0 + 28 + idx * 24), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 1)
    return image



def main():
    parser = argparse.ArgumentParser(description="Task4: threaded camera and SensorX visualization")
    parser.add_argument("--camera", default="0", help="camera device name, e.g. /dev/video0 or 0")
    parser.add_argument("--resolution", type=parse_resolution, default=parse_resolution("640x480"), 
                        help="camera resolution WIDTHxHEIGHT")
    parser.add_argument("--fps", type=float, default=30.0, help="display FPS")
    args = parser.parse_args()

    if args.fps <= 0:
        raise SystemExit("--fps must be positive")

    stop_event = threading.Event()
    signal.signal(signal.SIGINT, lambda *_: stop_event.set())
    signal.signal(signal.SIGTERM, lambda *_: stop_event.set())

    queues = [queue.Queue(maxsize=1) for _ in range(4)]
    sensors = [
        SensorCam(args.camera, args.resolution),
        SensorX(0.01),
        SensorX(0.1),
        SensorX(1)
    ]
    threads = [
        threading.Thread(target=sensor_loop, args=(sensors[idx], queues[idx], stop_event), 
                         name=f"sensor-{idx}", daemon=True)
        for idx in range(4)
    ]

    for thread in threads:
        thread.start()

    window = WindowImage(args.fps)
    latest_values = [None, None, None, None]

    try:
        while not stop_event.is_set():
            for idx, sensor_queue in enumerate(queues):
                latest_values[idx] = drain_latest(sensor_queue, latest_values[idx])
            image = compose_image(latest_values[0], latest_values[1:])
            if window.show(image) == ord("q"):
                stop_event.set()
    finally:
        stop_event.set()
        for thread in threads:
            thread.join(timeout=2.0)
        del window
        for sensor in sensors:
            del sensor

    return 0


if __name__ == "__main__":
    raise SystemExit(main())