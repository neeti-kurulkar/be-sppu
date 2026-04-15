"""
Test script to verify GPU setup and edge detection functionality
"""

import sys

def test_imports():
    """Test if all required packages can be imported"""
    print("Testing imports...")
    try:
        import cv2
        print("✓ OpenCV imported successfully")
    except ImportError as e:
        print(f"✗ OpenCV import failed: {e}")
        return False

    try:
        import numpy
        print("✓ NumPy imported successfully")
    except ImportError as e:
        print(f"✗ NumPy import failed: {e}")
        return False

    try:
        import streamlit
        print("✓ Streamlit imported successfully")
    except ImportError as e:
        print(f"✗ Streamlit import failed: {e}")
        return False

    try:
        import cupy as cp
        print("✓ CuPy imported successfully")
        return True
    except ImportError as e:
        print(f"✗ CuPy import failed: {e}")
        print("\nPlease install CuPy for your CUDA version:")
        print("  pip install cupy-cuda12x  # for CUDA 12.x")
        print("  pip install cupy-cuda11x  # for CUDA 11.x")
        return False

def test_gpu():
    """Test GPU availability and functionality"""
    print("\nTesting GPU...")
    try:
        import cupy as cp

        # Get GPU device info
        print(f"✓ GPU Device: {cp.cuda.Device()}")
        print(f"✓ GPU Name: {cp.cuda.runtime.getDeviceProperties(0)['name'].decode()}")

        # Get memory info
        free_mem, total_mem = cp.cuda.Device().mem_info
        print(f"✓ GPU Memory: {free_mem / 1024**3:.2f} GB free / {total_mem / 1024**3:.2f} GB total")

        # Test basic GPU operation
        a = cp.array([1, 2, 3])
        b = cp.array([4, 5, 6])
        c = a + b
        print(f"✓ GPU computation test passed: {c}")

        return True
    except Exception as e:
        print(f"✗ GPU test failed: {e}")
        return False

def test_edge_detection():
    """Test edge detection functionality"""
    print("\nTesting edge detection...")
    try:
        import numpy as np
        import cv2
        from gpu_edge_detection import GPUEdgeDetector, CPUEdgeDetector

        # Create a simple test image
        test_image = np.zeros((100, 100), dtype=np.uint8)
        test_image[25:75, 25:75] = 255  # White square on black background

        # Test GPU edge detection
        print("Testing GPU edge detection...")
        gpu_detector = GPUEdgeDetector()
        edges_gpu, time_gpu = gpu_detector.sobel_edge_detection(test_image, threshold=50)
        print(f"✓ GPU Sobel edge detection: {time_gpu*1000:.2f} ms")

        # Test CPU edge detection
        print("Testing CPU edge detection...")
        cpu_detector = CPUEdgeDetector()
        edges_cpu, time_cpu = cpu_detector.sobel_edge_detection(test_image, threshold=50)
        print(f"✓ CPU Sobel edge detection: {time_cpu*1000:.2f} ms")

        # Calculate speedup
        speedup = time_cpu / time_gpu if time_gpu > 0 else 0
        print(f"✓ GPU Speedup: {speedup:.2f}x")

        return True
    except Exception as e:
        print(f"✗ Edge detection test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run all tests"""
    print("=" * 60)
    print("GPU Edge Detection - System Test")
    print("=" * 60)

    results = []

    # Test imports
    results.append(("Imports", test_imports()))

    # Test GPU (only if imports succeeded)
    if results[0][1]:
        results.append(("GPU", test_gpu()))

        # Test edge detection (only if GPU test succeeded)
        if results[1][1]:
            results.append(("Edge Detection", test_edge_detection()))
    else:
        print("\nSkipping GPU and edge detection tests due to import failures.")

    # Print summary
    print("\n" + "=" * 60)
    print("Test Summary")
    print("=" * 60)
    for test_name, result in results:
        status = "PASSED" if result else "FAILED"
        symbol = "✓" if result else "✗"
        print(f"{symbol} {test_name}: {status}")

    # Overall result
    all_passed = all(result for _, result in results)
    print("\n" + "=" * 60)
    if all_passed:
        print("✓ All tests passed! You're ready to run the application.")
        print("\nTo start the application, run:")
        print("  streamlit run app.py")
    else:
        print("✗ Some tests failed. Please fix the issues before running the application.")
    print("=" * 60)

    return 0 if all_passed else 1

if __name__ == "__main__":
    sys.exit(main())
