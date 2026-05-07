import logging
import time

import cv2


class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")


class SensorX(Sensor):
    """Sensor X"""

    def __init__(self, delay: float):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data
    

class SensorCam(Sensor):
    def __init__(self, camera_name, resolution):
        source = int(camera_name) if camera_name.isdigit() else camera_name
        self._cam = cv2.VideoCapture(source)
        if not self._cam.isOpened():
            logging.error("Camera '%s' cannot be opened", camera_name)
            raise RuntimeError(f"Camera '{camera_name}' cannot be opened")

        width, height = resolution
        self._cam.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self._cam.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

    def get(self):
        ok, frame = self._cam.read()
        if not ok or frame is None:
            logging.error("Camera frame read failed")
            raise RuntimeError("Camera frame read failed")
        return frame

    def __del__(self):
        cam = self._cam
        cam.release()


class WindowImage:
    def __init__(self, fps, name="Task4 sensors"):
        self._name = name
        self._delay_ms = max(1, int(1000 / fps))
        try:
            cv2.namedWindow(self._name, cv2.WINDOW_NORMAL)
        except cv2.error as exc:
            logging.exception("Window initialization failed")
            raise RuntimeError("Window initialization failed") from exc

    def show(self, img):
        try:
            cv2.imshow(self._name, img)
            return cv2.waitKey(self._delay_ms) & 0xFF
        except cv2.error as exc:
            logging.exception("Window output failed")
            raise RuntimeError("Window output failed") from exc

    def __del__(self):
        try:
            cv2.destroyWindow(self._name)
        except cv2.error:
            pass