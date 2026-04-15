import streamlit as st
import cv2
import numpy as np
from PIL import Image
import tempfile
import os
from gpu_edge_detection import GPUEdgeDetector, CPUEdgeDetector

# Page configuration
st.set_page_config(
    page_title="GPU Edge Detection",
    page_icon="🎯",
    layout="wide",
    initial_sidebar_state="expanded"
)

# Custom CSS
st.markdown("""
    <style>
    .main-header {
        font-size: 3rem;
        font-weight: bold;
        text-align: center;
        background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
        -webkit-background-clip: text;
        -webkit-text-fill-color: transparent;
        padding: 1rem;
    }
    .metric-card {
        background-color: #f0f2f6;
        padding: 1rem;
        border-radius: 0.5rem;
        border-left: 4px solid #667eea;
    }
    .stButton>button {
        width: 100%;
        background-color: #667eea;
        color: white;
        border-radius: 0.5rem;
        padding: 0.5rem;
        font-weight: bold;
    }
    .stButton>button:hover {
        background-color: #764ba2;
    }
    </style>
""", unsafe_allow_html=True)

# Title
st.markdown('<h1 class="main-header">GPU-Accelerated Edge Detection</h1>', unsafe_allow_html=True)
st.markdown("### Process images and videos with CUDA-powered edge detection")

# Initialize session state
if 'gpu_detector' not in st.session_state:
    try:
        st.session_state.gpu_detector = GPUEdgeDetector()
        st.session_state.gpu_available = True
    except Exception as e:
        st.session_state.gpu_available = False
        st.error(f"GPU not available: {e}. Using CPU mode only.")

if 'cpu_detector' not in st.session_state:
    st.session_state.cpu_detector = CPUEdgeDetector()

# Sidebar
with st.sidebar:
    st.header("⚙️ Settings")

    # GPU Status
    if st.session_state.gpu_available:
        st.success("✅ GPU Available (NVIDIA RTX 4050)")
    else:
        st.warning("⚠️ GPU Not Available")

    st.markdown("---")

    # Mode selection
    mode = st.radio("Select Mode", ["Image Processing", "Video Processing"])

    st.markdown("---")

    # Edge detection method
    edge_method = st.selectbox(
        "Edge Detection Method",
        ["Sobel", "Canny", "Laplacian"]
    )

    st.markdown("---")

    # Processing mode
    if st.session_state.gpu_available:
        processing_mode = st.radio(
            "Processing Mode",
            ["GPU (CUDA)", "CPU", "Both (Compare)"]
        )
    else:
        processing_mode = "CPU"
        st.info("GPU not available, using CPU mode")

    st.markdown("---")

    # Method-specific parameters
    st.subheader("Parameters")

    if edge_method == "Sobel":
        threshold = st.slider("Threshold", 0, 255, 50, 5)
        low_threshold = None
        high_threshold = None
    elif edge_method == "Canny":
        low_threshold = st.slider("Low Threshold", 0, 255, 50, 5)
        high_threshold = st.slider("High Threshold", 0, 255, 150, 5)
        threshold = None
    else:  # Laplacian
        threshold = st.slider("Threshold", 0, 255, 30, 5)
        low_threshold = None
        high_threshold = None

# Main content
if mode == "Image Processing":
    st.header("📷 Image Processing")

    uploaded_file = st.file_uploader("Upload an image", type=["jpg", "jpeg", "png", "bmp"])

    if uploaded_file is not None:
        # Read image
        image = Image.open(uploaded_file)
        image_np = np.array(image)

        # Convert RGB to BGR for OpenCV
        if len(image_np.shape) == 3:
            image_cv = cv2.cvtColor(image_np, cv2.COLOR_RGB2BGR)
        else:
            image_cv = image_np

        # Display original image
        col1, col2 = st.columns(2)

        with col1:
            st.subheader("Original Image")
            st.image(image, use_container_width=True)
            st.caption(f"Size: {image_np.shape[1]}x{image_np.shape[0]}")

        # Process based on mode
        if processing_mode == "GPU (CUDA)" or processing_mode == "Both (Compare)":
            with st.spinner("Processing on GPU..."):
                if edge_method == "Sobel":
                    edges_gpu, time_gpu = st.session_state.gpu_detector.sobel_edge_detection(image_cv, threshold)
                elif edge_method == "Canny":
                    edges_gpu, time_gpu = st.session_state.gpu_detector.canny_edge_detection(
                        image_cv, low_threshold, high_threshold
                    )
                else:  # Laplacian
                    edges_gpu, time_gpu = st.session_state.gpu_detector.laplacian_edge_detection(image_cv, threshold)

        if processing_mode == "CPU" or processing_mode == "Both (Compare)":
            with st.spinner("Processing on CPU..."):
                if edge_method == "Sobel":
                    edges_cpu, time_cpu = st.session_state.cpu_detector.sobel_edge_detection(image_cv, threshold)
                elif edge_method == "Canny":
                    edges_cpu, time_cpu = st.session_state.cpu_detector.canny_edge_detection(
                        image_cv, low_threshold, high_threshold
                    )
                else:  # Laplacian
                    edges_cpu, time_cpu = st.session_state.cpu_detector.laplacian_edge_detection(image_cv, threshold)

        # Display results
        with col2:
            if processing_mode == "GPU (CUDA)":
                st.subheader("GPU Edge Detection")
                st.image(edges_gpu, use_container_width=True, clamp=True)
                st.markdown(f'<div class="metric-card">Processing Time: <b>{time_gpu*1000:.2f} ms</b></div>',
                          unsafe_allow_html=True)

            elif processing_mode == "CPU":
                st.subheader("CPU Edge Detection")
                st.image(edges_cpu, use_container_width=True, clamp=True)
                st.markdown(f'<div class="metric-card">Processing Time: <b>{time_cpu*1000:.2f} ms</b></div>',
                          unsafe_allow_html=True)

            else:  # Both
                st.subheader("GPU Edge Detection")
                st.image(edges_gpu, use_container_width=True, clamp=True)
                st.markdown(f'<div class="metric-card">GPU Time: <b>{time_gpu*1000:.2f} ms</b></div>',
                          unsafe_allow_html=True)

        if processing_mode == "Both (Compare)":
            col3, col4 = st.columns(2)
            with col3:
                st.subheader("CPU Edge Detection")
                st.image(edges_cpu, use_container_width=True, clamp=True)
                st.markdown(f'<div class="metric-card">CPU Time: <b>{time_cpu*1000:.2f} ms</b></div>',
                          unsafe_allow_html=True)

            with col4:
                st.subheader("Performance Comparison")
                speedup = time_cpu / time_gpu if time_gpu > 0 else 0
                st.metric("GPU Speedup", f"{speedup:.2f}x", f"{(speedup-1)*100:.1f}% faster")
                st.metric("GPU Time", f"{time_gpu*1000:.2f} ms")
                st.metric("CPU Time", f"{time_cpu*1000:.2f} ms")

        # Download button
        st.markdown("---")
        if processing_mode == "GPU (CUDA)" or processing_mode == "CPU":
            result_image = edges_gpu if processing_mode == "GPU (CUDA)" else edges_cpu
            result_pil = Image.fromarray(result_image)

            # Convert to bytes
            import io
            buf = io.BytesIO()
            result_pil.save(buf, format="PNG")
            byte_im = buf.getvalue()

            st.download_button(
                label="⬇️ Download Edge Detection Result",
                data=byte_im,
                file_name=f"edge_detection_{edge_method.lower()}.png",
                mime="image/png"
            )

else:  # Video Processing
    st.header("🎥 Video Processing")

    uploaded_video = st.file_uploader("Upload a video", type=["mp4", "avi", "mov", "mkv"])

    if uploaded_video is not None:
        # Save uploaded video to temp file
        tfile = tempfile.NamedTemporaryFile(delete=False, suffix='.mp4')
        tfile.write(uploaded_video.read())
        video_path = tfile.name

        # Display original video
        st.subheader("Original Video")
        st.video(video_path)

        # Process video button
        if st.button("🚀 Process Video"):
            # Open video
            cap = cv2.VideoCapture(video_path)

            # Get video properties
            fps = int(cap.get(cv2.CAP_PROP_FPS))
            width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

            st.info(f"Video Info: {width}x{height} @ {fps} FPS, {total_frames} frames")

            # Create output video
            output_path = tempfile.NamedTemporaryFile(delete=False, suffix='.mp4').name
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')
            out = cv2.VideoWriter(output_path, fourcc, fps, (width, height), isColor=False)

            # Progress bar
            progress_bar = st.progress(0)
            status_text = st.empty()

            total_time = 0
            frame_count = 0

            try:
                while cap.isOpened():
                    ret, frame = cap.read()
                    if not ret:
                        break

                    # Process frame
                    if processing_mode == "GPU (CUDA)" and st.session_state.gpu_available:
                        if edge_method == "Sobel":
                            edges, proc_time = st.session_state.gpu_detector.sobel_edge_detection(frame, threshold)
                        elif edge_method == "Canny":
                            edges, proc_time = st.session_state.gpu_detector.canny_edge_detection(
                                frame, low_threshold, high_threshold
                            )
                        else:  # Laplacian
                            edges, proc_time = st.session_state.gpu_detector.laplacian_edge_detection(frame, threshold)
                    else:
                        if edge_method == "Sobel":
                            edges, proc_time = st.session_state.cpu_detector.sobel_edge_detection(frame, threshold)
                        elif edge_method == "Canny":
                            edges, proc_time = st.session_state.cpu_detector.canny_edge_detection(
                                frame, low_threshold, high_threshold
                            )
                        else:  # Laplacian
                            edges, proc_time = st.session_state.cpu_detector.laplacian_edge_detection(frame, threshold)

                    # Write frame
                    out.write(edges)

                    total_time += proc_time
                    frame_count += 1

                    # Update progress
                    progress = frame_count / total_frames
                    progress_bar.progress(progress)
                    status_text.text(f"Processing: Frame {frame_count}/{total_frames} "
                                   f"({progress*100:.1f}%) - "
                                   f"Avg: {(total_time/frame_count)*1000:.2f} ms/frame")

                # Release resources
                cap.release()
                out.release()

                # Display results
                progress_bar.progress(1.0)
                st.success("✅ Video processing complete!")

                st.subheader("Processed Video")
                st.video(output_path)

                # Performance metrics
                col1, col2, col3 = st.columns(3)
                with col1:
                    st.metric("Total Frames", frame_count)
                with col2:
                    st.metric("Avg Processing Time", f"{(total_time/frame_count)*1000:.2f} ms/frame")
                with col3:
                    st.metric("Total Time", f"{total_time:.2f} s")

                # Download button
                with open(output_path, 'rb') as f:
                    st.download_button(
                        label="⬇️ Download Processed Video",
                        data=f,
                        file_name=f"edge_detection_video_{edge_method.lower()}.mp4",
                        mime="video/mp4"
                    )

            except Exception as e:
                st.error(f"Error processing video: {e}")
            finally:
                cap.release()
                out.release()

# Footer
st.markdown("---")
st.markdown("""
    <div style='text-align: center; color: #666;'>
        <p>GPU-Accelerated Edge Detection System</p>
        <p>Built with Streamlit, CuPy, and OpenCV | Powered by NVIDIA CUDA</p>
    </div>
""", unsafe_allow_html=True)
