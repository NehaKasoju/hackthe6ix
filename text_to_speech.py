"""Text-to-Speech for Video Summarization"""

import json
import pyttsx3
import random

# Load Twelve Labs JSON output (simulate API response)
with open('twelve_labs_output.json', 'r') as f:
    data = json.load(f)

# Initialize Text-to-Speech engine
engine = pyttsx3.init()

# Optional: Set audio output to Bluetooth (usually system default is enough)
# For advanced use, uncomment and configure this part:
# voices = engine.getProperty('voices')
# for voice in voices:
#     print(voice.id)  # Find your AirPods or preferred audio device
# engine.setProperty('voice', 'insert_your_airpods_voice_id_here')

# Function to create natural phrases
def generate_spoken_phrase(description):
    templates = [
        "You are currently seeing {}.",
        "In front of you: {}.",
        "Detected: {}.",
        "Right now, there's {}.",
        "Scene: {}."
    ]
    template = random.choice(templates)
    return template.format(description)

# Iterate through the API results and speak each description
for scene in data["results"]:
    desc = scene["description"]
    phrase = generate_spoken_phrase(desc)

    print(f"Speaking: {phrase}")  # For debug
    engine.say(phrase)

# Speak all phrases sequentially
engine.runAndWait()