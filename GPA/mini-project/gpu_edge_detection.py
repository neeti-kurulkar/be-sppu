import cupy as cp
import numpy as np
import cv2
import time
from typing import Tuple, Optional

class GPUEdgeDetector:
    """GPU-accelerated edge detection using CuPy and CUDA"""

    def __init__(self):
        """Initialize GPU edge detector"""
        # Sobel kernels for edge detection
        self.sobel_x = cp.array([[-1, 0, 1],
                                  [-2, 0, 2],
                                  [-1, 0, 1]], dtype=cp.float32)

        self.sobel_y = cp.array([[-1, -2, -1],
                                  [0, 0, 0],
                                  [1, 2, 1]], dtype=cp.float32)

        # Gaussian kernel for smoothing
        self.gaussian_kernel = cp.array([[1, 2, 1],
                                         [2, 4, 2],
                                         [1, 2, 1]], dtype=cp.float32) / 16.0

        self.processing_time = 0.0

    def sobel_edge_detection(self, image: np.ndarray, threshold: int = 50) -> Tuple[np.ndarray, float]:
        """
        Perform Sobel edge detection on GPU

        Args:
            image: Input image (grayscale or color)
            threshold: Threshold for edge detection

        Returns:
            Tuple of (edge_image, processing_time)
        """
        start_time = time.time()

        # Convert to grayscale if needed
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image

        # Transfer to GPU
        img_gpu = cp.asarray(gray, dtype=cp.float32)

        # Apply Gaussian blur for noise reduction
        blurred = self._convolve2d_gpu(img_gpu, self.gaussian_kernel)

        # Apply Sobel operators
        grad_x = self._convolve2d_gpu(blurred, self.sobel_x)
        grad_y = self._convolve2d_gpu(blurred, self.sobel_y)

        # Calculate gradient magnitude
        magnitude = cp.sqrt(grad_x**2 + grad_y**2)

        # Normalize to 0-255
        magnitude = (magnitude / magnitude.max() * 255).astype(cp.uint8)

        # Apply threshold
        edges = cp.where(magnitude > threshold, 255, 0).astype(cp.uint8)

        # Transfer back to CPU
        result = cp.asnumpy(edges)

        self.processing_time = time.time() - start_time

        return result, self.processing_time

    def canny_edge_detection(self, image: np.ndarray, low_threshold: int = 50,
                            high_threshold: int = 150) -> Tuple[np.ndarray, float]:
        """
        Perform Canny edge detection with GPU acceleration

        Args:
            image: Input image
            low_threshold: Lower threshold for Canny
            high_threshold: Upper threshold for Canny

        Returns:
            Tuple of (edge_image, processing_time)
        """
        start_time = time.time()

        # Convert to grayscale if needed
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image

        # Transfer to GPU
        img_gpu = cp.asarray(gray, dtype=cp.float32)

        # Apply Gaussian blur
        blurred = self._convolve2d_gpu(img_gpu, self.gaussian_kernel)

        # Calculate gradients
        grad_x = self._convolve2d_gpu(blurred, self.sobel_x)
        grad_y = self._convolve2d_gpu(blurred, self.sobel_y)

        # Calculate magnitude and direction
        magnitude = cp.sqrt(grad_x**2 + grad_y**2)
        angle = cp.arctan2(grad_y, grad_x)

        # Non-maximum suppression (simplified version)
        edges = self._non_max_suppression(magnitude, angle)

        # Double threshold
        edges = self._double_threshold(edges, low_threshold, high_threshold)

        # Transfer back to CPU
        result = cp.asnumpy(edges)

        self.processing_time = time.time() - start_time

        return result, self.processing_time

    def laplacian_edge_detection(self, image: np.ndarray, threshold: int = 30) -> Tuple[np.ndarray, float]:
        """
        Perform Laplacian edge detection on GPU

        Args:
            image: Input image
            threshold: Threshold for edge detection

        Returns:
            Tuple of (edge_image, processing_time)
        """
        start_time = time.time()

        # Convert to grayscale if needed
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image

        # Laplacian kernel
        laplacian_kernel = cp.array([[0, 1, 0],
                                      [1, -4, 1],
                                      [0, 1, 0]], dtype=cp.float32)

        # Transfer to GPU
        img_gpu = cp.asarray(gray, dtype=cp.float32)

        # Apply Gaussian blur
        blurred = self._convolve2d_gpu(img_gpu, self.gaussian_kernel)

        # Apply Laplacian
        laplacian = self._convolve2d_gpu(blurred, laplacian_kernel)

        # Take absolute value and normalize
        edges = cp.abs(laplacian)
        edges = (edges / edges.max() * 255).astype(cp.uint8)

        # Apply threshold
        edges = cp.where(edges > threshold, 255, 0).astype(cp.uint8)

        # Transfer back to CPU
        result = cp.asnumpy(edges)

        self.processing_time = time.time() - start_time

        return result, self.processing_time

    def _convolve2d_gpu(self, image: cp.ndarray, kernel: cp.ndarray) -> cp.ndarray:
        """
        Perform 2D convolution on GPU using manual implementation

        Args:
            image: Input image on GPU
            kernel: Convolution kernel on GPU

        Returns:
            Convolved image on GPU
        """
        # Get dimensions
        img_h, img_w = image.shape
        ker_h, ker_w = kernel.shape
        pad_h, pad_w = ker_h // 2, ker_w // 2

        # Pad image
        padded = cp.pad(image, ((pad_h, pad_h), (pad_w, pad_w)), mode='edge')

        # Create output array
        output = cp.zeros_like(image)

        # Perform convolution using sliding window
        for i in range(ker_h):
            for j in range(ker_w):
                output += padded[i:i+img_h, j:j+img_w] * kernel[i, j]

        return output

    def _non_max_suppression(self, magnitude: cp.ndarray, angle: cp.ndarray) -> cp.ndarray:
        """
        Simplified non-maximum suppression on GPU

        Args:
            magnitude: Gradient magnitude
            angle: Gradient direction

        Returns:
            Suppressed edges
        """
        # Normalize angles to 0-180 degrees
        angle = cp.rad2deg(angle) % 180

        # Simplified: threshold based on magnitude
        result = magnitude.copy()
        result = (result / result.max() * 255).astype(cp.uint8)

        return result

    def _double_threshold(self, image: cp.ndarray, low: int, high: int) -> cp.ndarray:
        """
        Apply double thresholding

        Args:
            image: Input edge image
            low: Low threshold
            high: High threshold

        Returns:
            Thresholded image
        """
        strong = 255
        weak = 75

        result = cp.zeros_like(image)

        strong_edges = image >= high
        weak_edges = (image >= low) & (image < high)

        result[strong_edges] = strong
        result[weak_edges] = weak

        return result


class CPUEdgeDetector:
    """CPU-based edge detection for comparison"""

    def __init__(self):
        self.processing_time = 0.0

    def sobel_edge_detection(self, image: np.ndarray, threshold: int = 50) -> Tuple[np.ndarray, float]:
        """
        Perform Sobel edge detection on CPU using OpenCV

        Args:
            image: Input image
            threshold: Threshold for edge detection

        Returns:
            Tuple of (edge_image, processing_time)
        """
        start_time = time.time()

        # Convert to grayscale if needed
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image

        # Apply Gaussian blur
        blurred = cv2.GaussianBlur(gray, (3, 3), 0)

        # Apply Sobel operators
        grad_x = cv2.Sobel(blurred, cv2.CV_64F, 1, 0, ksize=3)
        grad_y = cv2.Sobel(blurred, cv2.CV_64F, 0, 1, ksize=3)

        # Calculate magnitude
        magnitude = np.sqrt(grad_x**2 + grad_y**2)
        magnitude = (magnitude / magnitude.max() * 255).astype(np.uint8)

        # Apply threshold
        edges = np.where(magnitude > threshold, 255, 0).astype(np.uint8)

        self.processing_time = time.time() - start_time

        return edges, self.processing_time

    def canny_edge_detection(self, image: np.ndarray, low_threshold: int = 50,
                            high_threshold: int = 150) -> Tuple[np.ndarray, float]:
        """
        Perform Canny edge detection on CPU

        Args:
            image: Input image
            low_threshold: Lower threshold
            high_threshold: Upper threshold

        Returns:
            Tuple of (edge_image, processing_time)
        """
        start_time = time.time()

        # Convert to grayscale if needed
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image

        # Apply Canny edge detection
        edges = cv2.Canny(gray, low_threshold, high_threshold)

        self.processing_time = time.time() - start_time

        return edges, self.processing_time

    def laplacian_edge_detection(self, image: np.ndarray, threshold: int = 30) -> Tuple[np.ndarray, float]:
        """
        Perform Laplacian edge detection on CPU

        Args:
            image: Input image
            threshold: Threshold for edge detection

        Returns:
            Tuple of (edge_image, processing_time)
        """
        start_time = time.time()

        # Convert to grayscale if needed
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image

        # Apply Gaussian blur
        blurred = cv2.GaussianBlur(gray, (3, 3), 0)

        # Apply Laplacian
        laplacian = cv2.Laplacian(blurred, cv2.CV_64F, ksize=3)

        # Take absolute value and normalize
        edges = np.abs(laplacian)
        edges = (edges / edges.max() * 255).astype(np.uint8)

        # Apply threshold
        edges = np.where(edges > threshold, 255, 0).astype(np.uint8)

        self.processing_time = time.time() - start_time

        return edges, self.processing_time
