````md
# Digit Recognition using HMM and LPC

A voice-based digit recognition system written in C++ for Windows that recognizes spoken digits (`0–9`) using classical speech recognition techniques.

This project implements the complete speech-recognition pipeline from scratch without external machine learning or speech-processing libraries.

---

## Features

- Speech-based digit recognition (`0–9`)
- LPC and cepstral feature extraction
- Vector Quantization using LBG clustering
- Hidden Markov Model (HMM) training
- Baum-Welch learning algorithm
- Real-time microphone testing
- File-based batch testing
- WinMM-based audio recording

---

## Tech Stack

- C++
- Visual Studio
- WinMM API
- Hidden Markov Models (HMM)
- Linear Predictive Coding (LPC)
- Vector Quantization (VQ)

---

## Speech Recognition Pipeline

```text
Audio Input
    ↓
Preprocessing
(DC Shift Removal + Normalization)
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
````

---

## Project Structure

| File / Folder                              | Description                               |
| ------------------------------------------ | ----------------------------------------- |
| `Digit_Recognition.cpp`                    | Main application and recognition pipeline |
| `stdafx.h`, `stdafx.cpp`                   | Visual Studio precompiled headers         |
| `targetver.h`                              | Windows target version configuration      |
| `Digit_Recognition.vcxproj`                | Visual Studio project file                |
| `Digits/English/txt/`                      | Training and testing speech samples       |
| `codebook.txt`                             | Trained vector quantization codebook      |
| `model_digit_0.txt` to `model_digit_9.txt` | Trained HMM models                        |

---

## How It Works

### 1. Preprocessing

The program:

* Removes DC offset
* Normalizes amplitude
* Splits speech into frames
* Applies Hamming windowing

---

### 2. Feature Extraction

For each frame:

* LPC coefficients are computed
* Cepstral coefficients are generated
* Features are stored for training/testing

---

### 3. Vector Quantization

The system:

* Trains a 64-vector codebook using the LBG algorithm
* Converts feature vectors into observation symbols

---

### 4. HMM Training

One 5-state Hidden Markov Model is trained for each digit (`0–9`) using:

* Baum-Welch re-estimation
* Forward procedure
* Observation likelihood computation

---

### 5. Recognition

For testing:

* Speech samples are converted into observation sequences
* Likelihood is computed against all digit models
* The digit with highest probability is selected

---

## Build Instructions

### Prerequisites

Install:

* Visual Studio 2019 or 2022
* Desktop Development with C++
* Windows SDK

---

### Build Using Visual Studio

Open:

```text
Digit_Recognition.vcxproj
```

Then:

* Select `Debug` or `Release`
* Build the solution

---

### Build Using Command Line

Open a **Developer Command Prompt for Visual Studio** and run:

```bat
msbuild Digit_Recognition.vcxproj /p:Configuration=Release /p:Platform=Win32
```

For debug build:

```bat
msbuild Digit_Recognition.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

---

## Running the Project

Run the executable from:

```bat
Release\Digit_Recognition.exe
```

or

```bat
Debug\Digit_Recognition.exe
```

---

## Menu Options

```text
1. Train Models
2. Test Using Saved Files
3. Test Using Microphone
4. Quit
```

### Train Models

* Reads training files from:

  ```text
  Digits/English/txt/
  ```
* Generates:

  * `codebook.txt`
  * `model_digit_0.txt` to `model_digit_9.txt`

---

### Test Using Saved Files

* Evaluates saved speech samples
* Prints predicted digits and final accuracy

---

### Test Using Microphone

* Records live audio from microphone
* Performs real-time digit recognition

---

## Dataset Naming Format

Training files follow:

```text
254101040_E_<digit>_<utterance>.txt
```

Example:

```text
254101040_E_3_15.txt
```

Meaning:

* Digit = `3`
* Utterance number = `15`

---

## Important Notes

* This project is Windows-only because it uses WinMM APIs.
* The dataset folder structure must remain unchanged.
* If model files are missing, run training first.
* Microphone testing requires a working default recording device.

---

## Concepts Implemented

This project implements several classical speech-recognition concepts:

* Linear Predictive Coding (LPC)
* Cepstral Analysis
* Hamming Windowing
* Tokhura Distance
* Vector Quantization
* LBG Clustering
* Hidden Markov Models
* Baum-Welch Algorithm
* Forward Probability Computation

---

## Future Improvements

* MFCC-based feature extraction
* Noise reduction
* GUI interface
* Speaker-independent recognition
* Deep learning based acoustic models

---

## Demo

*Add screenshots or terminal outputs here.*

Example:

```md
![Demo](assets/demo.png)
```

---

## Author

Developed as a speech recognition and pattern recognition project using classical machine learning techniques in C++.

```
```
