"""Main python file."""

from summarize_video import get_summary_response
from gemini_agent import summarize_with_gemini, list_gemini_models
from text_to_speech import speak_text, list_voices
from camera_video import record_video

def main():
    """Main function to summarize video and speak the summary."""
    url = "assets/scenery.mp4"
    list_voices()  # List available TTS voices
    list_gemini_models()  # List available Gemini models
    while True:
        try:
            record_video()
            print("Getting video summary from Twelve Labs...")
            summary = get_summary_response(url)

            print("\nFull Video Summary:\n")
            print(summary)

            print("\nUsing Gemini to shorten the summary...\n")
            concise_summary = summarize_with_gemini(summary)
            concise_summary = concise_summary[concise_summary.index("$") + 1: -1].strip() if "$" in concise_summary else concise_summary

            print("\nFinal 2-Sentence Key Summary:\n")
            print(concise_summary)

            print("\nSpeaking the summary aloud...\n")
            speak_text(concise_summary)

        except Exception as e:
            print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
