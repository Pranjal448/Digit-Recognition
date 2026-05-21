Here is a polished, professional version of your README optimized for GitHub. I organized the sections logically, removed the repetitive horizontal rules, added standard GitHub formatting (like badges and blockquotes for notes), and grouped the setup instructions for better readability.

```markdown
# Digit Recognition using HMM and LPC

![C++](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)
![Windows](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white)
![Visual Studio](https://img.shields.io/badge/Visual_Studio-5C2D91?style=flat-square&logo=visual-studio&logoColor=white)

A voice-based digit recognition system written in C++ for Windows that recognizes spoken digits (`0–9`) using classical speech recognition techniques. 

This project implements the complete speech-recognition pipeline from scratch without relying on external machine learning or speech-processing libraries.

## Table of Contents
- [Features](#features)
- [Tech Stack](#tech-stack)
- [Speech Recognition Pipeline](#speech-recognition-pipeline)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Dataset Format](#dataset-format)
- [Concepts Implemented](#concepts-implemented)
- [Future Improvements](#future-improvements)

## Features
- **Speech-based digit recognition** (`0–9`)
- **LPC and cepstral feature extraction** for acoustic modeling
- **Vector Quantization** using LBG clustering
- **Hidden Markov Model (HMM) training** utilizing the Baum-Welch learning algorithm
- **Real-time microphone testing** using WinMM-based audio recording
- **File-based batch testing** for accuracy evaluation

## Tech Stack
- **Language:** C++
- **IDE:** Visual Studio
- **Audio API:** WinMM (Windows Multimedia API)
- **Core Algorithms:** Hidden Markov Models (HMM), Linear Predictive Coding (LPC), Vector Quantization (VQ)

## Speech Recognition Pipeline

```text
Audio Input
    ↓
Preprocessing (DC Shift Removal + Normalization)
    ↓
Frame Blocking + Hamming Window
    ↓
LPC Analysis
    ↓
Cepstral Feature Extraction
    ↓
Vector Quantization (LBG)
    ↓
Observation Sequence
    ↓
Hidden Markov Models
    ↓
Digit Prediction

```

## Project Structure

| File / Directory | Description |
| --- | --- |
| `Digit_Recognition.cpp` | Main application and recognition pipeline |
| `stdafx.h`, `stdafx.cpp` | Visual Studio precompiled headers |
| `targetver.h` | Windows target version configuration |
| `Digit_Recognition.vcxproj` | Visual Studio project file |
| `Digits/English/txt/` | Training and testing speech samples |
| `codebook.txt` | Trained vector quantization codebook |
| `model_digit_0.txt` ... `9` | Trained HMM models for each digit |

## Getting Started

### Prerequisites

To build and run this project, you will need:

* Visual Studio 2019 or 2022
* "Desktop Development with C++" workload installed
* Windows SDK

> **Note:** This project is **Windows-only** as it relies on the WinMM API for microphone access.

### Building the Project

**Option 1: Using Visual Studio**

1. Open `Digit_Recognition.vcxproj` in Visual Studio.
2. Select your desired configuration (`Debug` or `Release`).
3. Build the solution (`Ctrl + Shift + B`).

**Option 2: Using the Command Line**
Open a **Developer Command Prompt for Visual Studio** and run:

```bat
msbuild Digit_Recognition.vcxproj /p:Configuration=Release /p:Platform=Win32

```

*(Replace `Release` with `Debug` for debug builds).*

### Running the Application

Navigate to your build directory and run the executable:

```bat
Release\Digit_Recognition.exe

```

## Usage

Upon running the application, you will be presented with a CLI menu:

```text
1. Train Models
2. Test Using Saved Files
3. Test Using Microphone
4. Quit

```

1. **Train Models:** Reads training files from `Digits/English/txt/` and generates the `codebook.txt` and `model_digit_X.txt` files. *(If model files are missing, you must run this step first).*
2. **Test Using Saved Files:** Evaluates saved speech samples against the trained models, printing the predicted digits and overall accuracy.
3. **Test Using Microphone:** Records live audio from your default recording device and performs real-time digit recognition.

## Dataset Format

Training files in the `Digits/English/txt/` directory must follow this naming convention:
`254101040_E_<digit>_<utterance>.txt`

**Example:** `254101040_E_3_15.txt`

* **Digit:** `3`
* **Utterance:** `15`

> **Warning:** The dataset folder structure must remain unchanged for the program to locate the training and testing files.

## Concepts Implemented

This project serves as an educational implementation of classical speech and pattern recognition concepts:

* Linear Predictive Coding (LPC) & Cepstral Analysis
* Hamming Windowing
* Tokhura Distance
* Vector Quantization & LBG Clustering
* Hidden Markov Models (HMM)
* Baum-Welch Algorithm
* Forward Probability Computation

## Future Improvements

* [ ] Implement MFCC-based feature extraction for better noise robustness
* [ ] Add pre-processing for environmental noise reduction
* [ ] Develop a graphical user interface (GUI)
* [ ] Extend to speaker-independent recognition
* [ ] Compare against deep learning-based acoustic models

## Demo

*(Add screenshots or GIF recordings of terminal outputs here)*

```md
![Terminal Output](assets/demo.png)

```

---

**Author:** Developed as a speech and pattern recognition project exploring classical machine learning techniques in C++.

```

```
