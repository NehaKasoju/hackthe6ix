"""Main python file."""

from summarize_video import get_summary_response

def main():
    url = "assets/test_video.mp4"
    try:
        summary = get_summary_response(url)
        print("\nFinal Aggregated Summary:\n")
        print(summary)
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
