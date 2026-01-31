# Quick Start Guide

## Installation (5 minutes)

### 1. Create and activate virtual environment
```bash
python -m venv venv
venv\Scripts\activate  # Windows
```

### 2. Install dependencies
```bash
pip install -r requirements.txt
```

### 3. Verify installation
```bash
python test_gpu.py
```

You should see all tests pass with ✓ marks.

## Running the Application

### Start the Streamlit app
```bash
streamlit run app.py
```

The app will open in your browser at `http://localhost:8501`

## First-Time Usage

### Image Processing
1. Select **"Image Processing"** in the sidebar
2. Choose **"Sobel"** edge detection method
3. Select **"GPU (CUDA)"** processing mode
4. Upload any image (JPG, PNG, etc.)
5. See instant results!

### Video Processing
1. Select **"Video Processing"** in the sidebar
2. Choose **"Sobel"** edge detection method
3. Upload a short video (MP4 recommended)
4. Click **"Process Video"**
5. Wait for processing to complete
6. Download the result!

## Recommended Settings

### For Best Performance (GPU)
- Method: **Sobel** (fastest)
- Mode: **GPU (CUDA)**
- Threshold: **50**

### For Best Quality
- Method: **Canny**
- Low Threshold: **50**
- High Threshold: **150**

### For Comparison
- Mode: **Both (Compare)**
- See GPU vs CPU performance side-by-side

## Sample Parameters

### Sobel
- **Threshold 30**: Detects most edges (noisy)
- **Threshold 50**: Balanced (recommended)
- **Threshold 100**: Only strong edges (clean)

### Canny
- **Low 30, High 100**: More edges
- **Low 50, High 150**: Balanced (recommended)
- **Low 100, High 200**: Fewer edges

### Laplacian
- **Threshold 20**: More edges
- **Threshold 30**: Balanced (recommended)
- **Threshold 50**: Fewer edges

## Troubleshooting

### "GPU not available" warning
1. Check if CUDA is installed: `nvidia-smi`
2. Reinstall CuPy: `pip install cupy-cuda12x --no-cache-dir`
3. Restart the application

### Application is slow
1. Try smaller images/videos first
2. Close other applications using GPU
3. Update NVIDIA drivers

### Import errors
```bash
pip install --upgrade -r requirements.txt
```

## Tips

- Start with **small images** (< 1MB) to test
- Use **Sobel** for fastest processing
- Compare GPU vs CPU to see performance gains
- Higher resolution images show more GPU speedup
- For videos, keep them **under 1 minute** for faster testing

## Need Help?

Check the full [README.md](README.md) for detailed documentation.
