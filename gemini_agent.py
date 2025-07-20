import google.generativeai as genai
import os
from dotenv import load_dotenv

from prompts import GeminiPrompt

load_dotenv()
gemini_api_key = os.getenv("GEMINI_API_KEY")

if not gemini_api_key:
    raise ValueError("Gemini API Key did not load.")

genai.configure(api_key=gemini_api_key)

def summarize_with_gemini(input_text: str) -> str:
    """Use Gemini to summarize input_text into 2 key-detail sentences."""
    model = genai.GenerativeModel("gemini-2.5-pro")

    prompt = GeminiPrompt + "\n\n" + input_text

    try:
        response = model.generate_content(prompt)
        return response.text.strip()
    except Exception as e:
        return f"Gemini summarization failed: {e}"

def list_gemini_models():
    """List available Gemini models."""
    models = [model.name for model in genai.list_models()]
    print(models)