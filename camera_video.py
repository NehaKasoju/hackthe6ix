import subprocess
import datetime
import os

def record_video(output_path='assets', duration_sec=5, filename='scenery'):
    """Records a video using Raspberry Pi Camera Module 3 and saves it as an MP4 file."""

    def clear_existing_videos(folder):
        """Remove all .mp4 files in the given folder."""
        if os.path.exists(folder):
            for file in os.listdir(folder):
                if file.endswith(".mp4"):
                    os.remove(os.path.join(folder, file))

    # Ensure output folder exists
    os.makedirs(output_path, exist_ok=True)

    # Clear previous video(s)
    clear_existing_videos(output_path)

    h264_path = os.path.join(output_path, f"{filename}.h264")
    mp4_path = os.path.join(output_path, f"{filename}.mp4")

    try:
        print(f"Recording video for {duration_sec} seconds...")
        subprocess.run([
            "libcamera-vid",
            "-t", str(duration_sec * 1000),  # duration in milliseconds
            "-o", h264_path
        ], check=True)

        print("Converting to MP4...")
        subprocess.run([
            "MP4Box",
            "-add", h264_path,
            mp4_path
        ], check=True)

        print(f"Video saved as: {mp4_path}")
        return mp4_path

    except subprocess.CalledProcessError as e:
        print("An error occurred during recording or conversion:", e)
        return None

    finally:
        if os.path.exists(h264_path):
            os.remove(h264_path)  # Clean up raw .h264 file
