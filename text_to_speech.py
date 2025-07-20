import pyttsx3

def speak_text(text: str, rate: int = 175, volume: float = 1.0):
    """Convert text to speech with less monotone voice using pyttsx3."""
    engine = pyttsx3.init()

    # Set speaking rate and volume
    engine.setProperty('rate', rate)
    engine.setProperty('volume', volume)

    # Choose a more expressive voice (usually female)
    voices = engine.getProperty('voices')
    for voice in voices:
        if "zira" in voice.name.lower():
            engine.setProperty('voice', voice.id)
            break

    engine.say(text)
    engine.runAndWait()

def list_voices():
    """List available voices."""
    engine = pyttsx3.init()
    voices = engine.getProperty('voices')
    for idx, voice in enumerate(voices):
        print(f"Voice {idx}: {voice.name} - {voice.languages}")
