# 🎬 AI Video Summarizer with Ultrasonic Sensor Integration

This project is a multi-modal application combining:

- 🎥 Webcam video capture
- 🧠 AI-powered video summarization (Twelve Labs)
- ✂️ Text summarization with Google Gemini
- 🔊 Text-to-speech output
- 📏 Ultrasonic proximity sensing on Raspberry Pi (for QNX systems)

---

## 📦 Features

- Records a short video clip using the webcam.
- Sends the video to **Twelve Labs** for intelligent content summarization.
- Uses **Google Gemini** to condense the full summary to a 2-sentence format.
- Reads the summary aloud using a Text-to-Speech (TTS) engine.
- Includes a QNX-compatible module for using **HC-SR04** ultrasonic sensors on Raspberry Pi:
  - Monitors distance from two sensors.
  - Triggers GPIO buzzers proportionally to proximity.

---

## 🛠️ Setup

### 1. Clone the Repo

```bash
git clone https://github.com/your-username/video-summary-qnx.git
cd video-summary-qnx
```

### 2. Python Environment

Ensure you're using **Python 3.7+**

### 3. Install Dependencies

```bash
pip install -r requirements.txt
```

If using on QNX or Raspberry Pi, ensure `rpi_gpio` or compatible GPIO package is available.

---

## 🚀 Usage

### Video Summary + Speech (Standard Desktop)

```bash
python main.py
```

### Proximity Sensor + GPIO Buzzers (QNX / Raspberry Pi)

```bash
python ultrasonic_sensor.py
```

---

## 🧠 AI & External Services

This project integrates:

- [Twelve Labs](https://www.twelvelabs.io/) – AI-powered video content understanding.
- [Google Gemini](https://deepmind.google/technologies/gemini/) – Language model summarization.
- TTS via `pyttsx3` or similar.

---

## 📁 File Overview

| File                  | Purpose                                                       |
|-----------------------|---------------------------------------------------------------|
| `main.py`             | Main loop to record video, summarize, and speak output        |
| `summarize_video.py`  | Handles API calls to Twelve Labs                              |
| `gemini_agent.py`     | Interfaces with Gemini API for text summarization             |
| `text_to_speech.py`   | Uses TTS to speak summary aloud                               |
| `camera_video.py`     | Records video from webcam                                     |
| `ultrasonic_sensor.py`| Handles ultrasonic distance measurement + GPIO buzzer control |

---

## ⚙️ Requirements

Add the following to your `requirements.txt`:

```text
opencv-python
pyttsx3
requests
google-generativeai
```

> If using QNX, `rpi_gpio` is expected to be pre-installed or available via QNX's package manager.

---

## 📜 License

This project is licensed under the **Apache License 2.0**.

```text
Copyright (c) 2025, BlackBerry Limited.
Licensed under the Apache License, Version 2.0
```

See [LICENSE](LICENSE) for details.

---

## 🙏 Credits

- **Twelve Labs** – Video AI APIs.
- **Google Gemini API** – Text summarization.
- **pyttsx3** – TTS engine.
- **OpenCV** – Webcam handling.
- **rpi_gpio (QNX)** – Raspberry Pi GPIO control on QNX OS.
- **BlackBerry Limited** – Sensor module integration.

---

## 🧪 Disclaimer

The ultrasonic sensor module is designed for **QNX on Raspberry Pi** and may require adaptation for Linux-based systems.

---

## 📞 Contact

For issues, suggestions, or contributions, feel free to open an issue or pull request.
