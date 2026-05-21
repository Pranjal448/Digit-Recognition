# Digit Recognition using HMM and LPC

A voice-based digit recognition system written in **C++** for Windows that recognizes spoken digits (`0–9`) using classical speech recognition techniques.

> Built completely from scratch without external machine learning or speech-recognition libraries.

---

## Features

- Real-time microphone digit recognition
- Hidden Markov Model (HMM) based classification
- LPC & Cepstral feature extraction
- Vector Quantization using LBG clustering
- File-based and live testing support
- Windows audio capture using WinMM APIs

---

## Recognition Pipeline

```text
Audio Input
    ↓
Preprocessing
(DC Shift Removal + Normalization)
    ↓
Frame Blocking + Hamming Window
    ↓
LPC & Cepstral Feature Extraction
    ↓
Vector Quantization (LBG)
    ↓
Observation Sequence
    ↓
Hidden Markov Models
    ↓
Digit Prediction
```

---

## Tech Stack

| Technology | Purpose |
|---|---|
| C++ | Core implementation |
| WinMM API | Audio recording |
| LPC | Feature extraction |
| HMM | Digit classification |
| LBG Algorithm | Vector quantization |
| Visual Studio | Development environment |

---

## Project Structure

| File / Folder | Description |
|---|---|
| `Digit_Recognition.cpp` | Main recognition pipeline |
| `Digits/English/txt/` | Training & testing dataset |
| `codebook.txt` | Generated VQ codebook |
| `model_digit_0.txt` → `model_digit_9.txt` | Trained HMM models |
| `Digit_Recognition.vcxproj` | Visual Studio project file |
| `stdafx.h`, `stdafx.cpp` | Precompiled header files |

---

## Build Instructions

### Requirements

- Visual Studio 2010
- Desktop Development with C++
- Windows SDK

---

### Build Using Visual Studio

Open:

```text
Digit_Recognition.vcxproj
```

Then build using:
- `Build -> Build Solution`

configuration.


---

## Menu Options

```text
1. Train Models
2. Test Using Saved Files
3. Test Using Microphone
4. Quit
```

### Train Models
- Builds the vector quantization codebook
- Trains one HMM for each digit (`0–9`)
- Saves trained models to disk

### Test Using Saved Files
- Evaluates saved digit samples
- Prints predictions and final accuracy

### Test Using Microphone
- Records live speech from microphone
- Performs real-time digit recognition

---

## Dataset Format

Training files inside:

```text
Digits/English/txt/
```

follow the naming convention:

```text
254101040_E_<digit>_<utterance>.txt
```

Example:

```text
254101040_E_3_15.txt
```

| Part | Meaning |
|---|---|
| `3` | Spoken digit |
| `15` | Utterance number |

> The dataset folder structure must remain unchanged for the program to locate training and testing files correctly.

---

## Concepts Implemented

This project demonstrates classical speech and pattern recognition concepts including:

- Linear Predictive Coding (LPC)
- Cepstral Analysis
- Hamming Windowing
- Tokhura Distance
- Vector Quantization
- LBG Clustering
- Hidden Markov Models (HMM)
- Baum-Welch Training
- Forward Probability Computation

---

## Output Files

After training, the following files are generated:

- `codebook.txt`
- `model_digit_0.txt`
- `model_digit_1.txt`
- `model_digit_2.txt`
- `model_digit_3.txt`
- `model_digit_4.txt`
- `model_digit_5.txt`
- `model_digit_6.txt`
- `model_digit_7.txt`
- `model_digit_8.txt`
- `model_digit_9.txt`

---

## Important Notes

- This project is Windows-only.
- A working microphone is required for live testing.
- Keep dataset paths unchanged.
- Train models before testing if model files are missing.

---
