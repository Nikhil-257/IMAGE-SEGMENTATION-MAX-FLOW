# Image Segmentation using Max-Flow and Min-Cut

## Overview

This project implements image segmentation using the **Max-Flow Min-Cut theorem** and the **Ford–Fulkerson algorithm** in C++.

The image is represented as a graph where each pixel is a node. Source and sink nodes represent the foreground and background. The Ford–Fulkerson algorithm computes the maximum flow, and the corresponding minimum cut is used to segment the image.

---

## Features

- Image loading using STB Image
- Graph construction from image pixels
- Ford–Fulkerson algorithm for maximum flow
- Graph-cut based image segmentation
- Outputs a binary segmentation mask

---

## Technologies Used

- C++
- Ford–Fulkerson Algorithm
- Graph Theory
- STB Image Library

---

## Project Structure

```
Image-Segmentation-MaxFlow/
│
├── segmentation.cpp
├── stb_image.h
├── stb_image_write.h
├── Presentation.pdf
└── README.md
```

---

## How to Compile

```bash
g++ segmentation.cpp -o segmentation
```

---

## How to Run

```bash
./segmentation
```

Enter the input image name when prompted.

Example:

```
Enter input image name: input.png
```

The segmented image will be saved as:

```
output.png
```

---

## Algorithm

1. Load the input image.
2. Compute the mean background color.
3. Convert every pixel into a graph node.
4. Connect neighboring pixels using N-links.
5. Connect pixels to Source and Sink using T-links.
6. Run the Ford–Fulkerson algorithm.
7. Extract the minimum cut.
8. Save the segmented output image.

---

## Time Complexity

Ford–Fulkerson:

```
O(E × F)
```

where

- E = Number of edges
- F = Maximum flow

---

## Limitations

- Uses DFS-based Ford–Fulkerson.
- Works best for smaller images.
- Performs binary foreground/background segmentation.
- Uses mean image color as a simple background model.

---

## Future Improvements

- Edmonds–Karp implementation
- Dinic's Algorithm
- Boykov–Kolmogorov algorithm
- 8-neighbor connectivity
- Better foreground/background probability models
- User-selected foreground and background seeds

---

## References

- Ford, L. R., & Fulkerson, D. R. (1956). Maximal Flow Through a Network.
- STB Image Library: https://github.com/nothings/stb

---

## Author

Nikhil
IIT HYDERABAD