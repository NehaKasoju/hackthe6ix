"""Summarize a video by URL using Twelve Labs API."""

from dotenv import load_dotenv
import os
import time
import requests
from urllib.parse import urlparse
from twelvelabs import TwelveLabs
from twelvelabs.models.task import Task

from prompts import TwelveLabPrompt

INDEX_NAMES = ["test_videos", "scenery_videos"]  # Hard coded index names for simplicity

load_dotenv()
api_key = os.getenv("TWELVE_LABS_API_KEY")
if not api_key:
    raise ValueError("Twelve Labs API Key did not load.")

client = TwelveLabs(api_key=api_key)

models = [
    {
        "name": "pegasus1.2",
        "options": ["visual", "audio"]
    }
]

def get_or_create_index(name):
    try:
        index = next(idx for idx in client.index.list() if idx.name == name)
        print(f"Reusing index: id={index.id}")
        return index
    except StopIteration:
        print("Index not found, creating...")
        return client.index.create(
            name=name,
            models=[{"name": "pegasus1.2", "options": ["visual", "audio"]}]
        )

index = get_or_create_index(INDEX_NAMES[0])

def is_url(string: str) -> bool:
    try:
        result = urlparse(string)
        return all([result.scheme, result.netloc])
    except ValueError:
        return False

def wait_for_task_completion(task_id: str):
    print(f"Waiting for task {task_id} to complete...")
    while True:
        response = requests.get(
            f"https://api.twelvelabs.io/v1.3/tasks/{task_id}",
            headers={"x-api-key": api_key}
        )
        task = response.json()
        status = task.get("status")
        print(f"Status = {status}")
        if status in ["ready", "failed"]:
            return task
        time.sleep(5)

def get_summary_response(video_source: str, prompt: str = TwelveLabPrompt) -> str:
    """Analyze a video from a local file or URL and return a summary."""
    if is_url(video_source):
        print("Using remote URL for task creation.")
        task = client.task.create(index_id=index.id, url=video_source)
        task.wait_for_done(sleep_interval=5)
        if task.status != "ready":
            raise RuntimeError(f"Indexing failed with status {task.status}")
        video_id = task.video_id

    elif os.path.isfile(video_source):
        print("Uploading local video file.")
        with open(video_source, "rb") as f:
            response = requests.post(
                "https://api.twelvelabs.io/v1.3/tasks",
                headers={"x-api-key": api_key},
                data={
                    "index_id": index.id,
                    "language": "en",
                    "name": os.path.basename(video_source)
                },
                files={"video_file": f}
            )
        print(f"Upload status code: {response.status_code}")
        print(f"Upload response body: {response.text}")

        if response.status_code not in [200, 201]:
            raise RuntimeError(f"Failed to upload video: {response.status_code} - {response.text}")

        task_info = response.json()
        task_id = task_info["_id"]

        task_info = wait_for_task_completion(task_id)

        if task_info["status"] != "ready":
            raise RuntimeError(f"Indexing failed with status {task_info['status']}")
        video_id = task_info["video_id"]

    else:
        raise ValueError(f"Invalid video source: {video_source}")

    print(f"Video ID: {video_id}")

    text_stream = client.analyze_stream(video_id=video_id, prompt=prompt)

    for chunk in text_stream:
        print(chunk)

    # Uncomment below if you want the full aggregated text printed separately
    # print(f"\nAggregated Text:\n{text_stream.aggregated_text}")

    return text_stream.aggregated_text
