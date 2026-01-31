# GPU-Accelerated Edge Detection System

A high-performance edge detection system leveraging NVIDIA CUDA GPU acceleration through CuPy, built with Streamlit for an intuitive web interface. This project demonstrates GPU programming concepts for image and video processing.

## Features

- **GPU Acceleration**: Utilizes NVIDIA CUDA for high-performance edge detection
- **Multiple Edge Detection Methods**:
  - Sobel Edge Detection
  - Canny Edge Detection
  - Laplacian Edge Detection
- **Image & Video Processing**: Support for both image and video files
- **Performance Comparison**: Compare GPU vs CPU processing speeds
- **Interactive Web Interface**: User-friendly Streamlit application
- **Real-time Visualization**: View results instantly with adjustable parameters

## System Requirements

### Hardware
- NVIDIA GPU with CUDA support (Tested on RTX 4050)
- Minimum 4GB GPU memory recommended

### Software
- Python 3.8 or higher
- NVIDIA CUDA Toolkit 12.x
- NVIDIA GPU Driver (572.62 or higher)

## Installation

### Step 1: Verify CUDA Installation

Check if CUDA is installed and GPU is available:

```bash
nvidia-smi
```

You should see your GPU information and CUDA version.

### Step 2: Create Virtual Environment

```bash
# Create virtual environment
python -m venv venv

# Activate virtual environment
# On Windows:
venv\Scripts\activate
# On Linux/Mac:
source venv/bin/activate
```

### Step 3: Install Dependencies

```bash
pip install -r requirements.txt
```

**Note**: CuPy installation requires CUDA toolkit. The `cupy-cuda12x` package is for CUDA 12.x. If you have a different CUDA version:
- CUDA 11.x: `pip install cupy-cuda11x`
- CUDA 12.x: `pip install cupy-cuda12x` (already in requirements.txt)

### Troubleshooting Installation

If CuPy installation fails:

1. **Install CUDA Toolkit** from [NVIDIA CUDA Downloads](https://developer.nvidia.com/cuda-downloads)

2. **Verify CUDA installation**:
   ```bash
   nvcc --version
   ```

3. **Install CuPy manually**:
   ```bash
   pip install cupy-cuda12x --no-cache-dir
   ```

## Usage

### Starting the Application

```bash
streamlit run app.py
```

The application will open in your default web browser at `http://localhost:8501`

### Image Processing

1. Select **"Image Processing"** mode from the sidebar
2. Choose an edge detection method (Sobel, Canny, or Laplacian)
3. Select processing mode:
   - **GPU (CUDA)**: Process on GPU only
   - **CPU**: Process on CPU only
   - **Both (Compare)**: Compare GPU vs CPU performance
4. Adjust parameters using the sliders
5. Upload an image using the file uploader
6. View results and download the processed image

### Video Processing

1. Select **"Video Processing"** mode from the sidebar
2. Choose an edge detection method and parameters
3. Upload a video file (MP4, AVI, MOV, MKV)
4. Click "Process Video" button
5. Watch the progress bar as frames are processed
6. Download the processed video

## Edge Detection Methods

### 1. Sobel Edge Detection
- Detects edges using gradient magnitude
- Best for general-purpose edge detection
- **Parameter**: Threshold (0-255)
  - Lower values: More edges detected
  - Higher values: Only strong edges

### 2. Canny Edge Detection
- Multi-stage edge detection algorithm
- Best for precise edge detection with noise reduction
- **Parameters**:
  - Low Threshold: Weak edge threshold
  - High Threshold: Strong edge threshold

### 3. Laplacian Edge Detection
- Second-order derivative method
- Good for detecting rapid intensity changes
- **Parameter**: Threshold (0-255)

## Performance

The GPU acceleration provides significant speedups, especially for:
- Large images (>1920x1080)
- High-resolution videos
- Real-time processing requirements

### Expected Speedup
- **Small images** (640x480): 2-5x faster
- **HD images** (1920x1080): 5-15x faster
- **4K images** (3840x2160): 10-30x faster

## Project Structure

```
mini-project/
│
├── app.py                    # Main Streamlit application
├── gpu_edge_detection.py     # GPU and CPU edge detection implementations
├── requirements.txt          # Python dependencies
└── README.md                # This file
```

## Technical Details

### GPU Implementation
- **Library**: CuPy (CUDA-accelerated NumPy)
- **Kernels**: Custom convolution operations on GPU
- **Memory Management**: Automatic GPU memory allocation and cleanup

### Algorithms Implemented
1. **Sobel Operator**:
   - Convolves image with Sobel kernels (x and y directions)
   - Computes gradient magnitude: √(Gx² + Gy²)
   - Applies thresholding

2. **Canny Edge Detection**:
   - Gaussian blur for noise reduction
   - Gradient calculation using Sobel
   - Non-maximum suppression
   - Double thresholding

3. **Laplacian Edge Detection**:
   - Second-order derivative operator
   - Detects zero-crossings
   - Sensitive to noise (Gaussian blur applied)

## Example Use Cases

### Academic Projects
- Demonstrating GPU programming concepts
- Parallel computing performance analysis
- Image processing algorithm implementation

### Practical Applications
- Object boundary detection
- Feature extraction for computer vision
- Preprocessing for machine learning models
- Medical image analysis

## Troubleshooting

### GPU Not Detected
- Verify NVIDIA drivers are installed: `nvidia-smi`
- Check CUDA installation: `nvcc --version`
- Ensure CuPy is installed for correct CUDA version

### Out of Memory Error
- Reduce image/video resolution
- Process fewer frames at once
- Close other GPU-intensive applications

### Slow Performance
- Ensure GPU drivers are up to date
- Check if other applications are using GPU
- Verify CUDA is properly installed

## Future Enhancements

- [ ] Add more edge detection algorithms (Prewitt, Roberts Cross)
- [ ] Implement custom CUDA kernels for even better performance
- [ ] Add batch processing for multiple files
- [ ] Support for real-time webcam processing
- [ ] Export performance metrics to CSV
- [ ] Add 3D visualization of edge detection results

## Credits

**Course**: GPU Programming
**Project**: Mini Project - Image and Video Edge Detection
**GPU**: NVIDIA GeForce RTX 4050
**CUDA Version**: 12.8

## License

This project is for educational purposes as part of a GPU Programming course.

## References

- [CuPy Documentation](https://docs.cupy.dev/)
- [NVIDIA CUDA Programming Guide](https://docs.nvidia.com/cuda/)
- [OpenCV Edge Detection](https://docs.opencv.org/4.x/da/d22/tutorial_py_canny.html)
- [Streamlit Documentation](https://docs.streamlit.io/)
