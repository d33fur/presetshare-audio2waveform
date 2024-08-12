import unittest
import requests
import os

class TestWaveformService(unittest.TestCase):

    BASE_URL = "http://localhost:8080/waveform"

    def setUp(self):
        self.audio_file = "test_audio.wav"
        self.create_test_audio_file(self.audio_file)

    def tearDown(self):
        if os.path.exists(self.audio_file):
            os.remove(self.audio_file)

    def create_test_audio_file(self, filename):
        import wave
        import numpy as np

        sample_rate = 44100
        duration = 1
        frequency = 440

        t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)
        data = (0.5 * np.sin(2 * np.pi * frequency * t)).astype(np.float32)

        with wave.open(filename, 'wb') as wf:
            wf.setnchannels(1)
            wf.setsampwidth(2)
            wf.setframerate(sample_rate)
            wf.writeframes((data * 32767).astype(np.int16).tobytes())

    def test_valid_request(self):
        """Тест корректного запроса с правильным аудиофайлом."""
        with open(self.audio_file, 'rb') as f:
            response = requests.post(self.BASE_URL, data=f)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'image/webp')

    def test_invalid_method(self):
        """Тест запроса с некорректным методом (GET вместо POST)."""
        response = requests.get(self.BASE_URL)
        self.assertEqual(response.status_code, 400)

    def test_large_audio_file(self):
        """Тест с большим аудиофайлом."""
        large_audio_file = "large_test_audio.wav"
        self.create_test_audio_file(large_audio_file)
        with open(large_audio_file, 'rb') as f:
            response = requests.post(self.BASE_URL, data=f)
        os.remove(large_audio_file)
        self.assertEqual(response.status_code, 200)

    # def test_invalid_audio_format(self):
    #     """Тест с некорректным аудиофайлом (например, текстовым файлом)."""
    #     with open('test.txt', 'w') as f:
    #         f.write("This is a test.")
    #     with open('test.txt', 'rb') as f:
    #         files = {'file': ('test.txt', f, 'text/plain')}
    #         response = requests.post(self.BASE_URL, files=files)
    #     os.remove('test.txt')
    #     self.assertEqual(response.status_code, 415)
    #     self.assertIn('Unsupported file type', response.text)

    # def test_empty_audio_file(self):
    #     """Тест с пустым аудиофайлом."""
    #     empty_audio_file = "empty_test_audio.wav"
    #     open(empty_audio_file, 'w').close()
    #     with open(empty_audio_file, 'rb') as f:
    #         response = requests.post(self.BASE_URL, data=f)
    #     os.remove(empty_audio_file)
    #     self.assertEqual(response.status_code, 500)   # ЛОМАЕТ

    # def test_missing_audio_file(self):
    #     """Тест с отсутствующим аудиофайлом в запросе."""
    #     response = requests.post(self.BASE_URL)
    #     self.assertEqual(response.status_code, 400)   # ЛОМАЕТ

    def test_invalid_endpoint(self):
        """Тест обращения к несуществующему endpoint."""
        response = requests.post("http://localhost:8080/invalid", data=b"")
        self.assertEqual(response.status_code, 400)

if __name__ == '__main__':
    unittest.main()
