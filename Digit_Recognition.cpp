#include "stdafx.h"
#define NOMINMAX
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>

using namespace std;

// Feature extraction constants
const int FRAME_SIZE = 320;
const int FRAME_SHIFT = 80;
const int LPC_ORDER = 12;
const int CEPSTRAL_ORDER = 12;
const double PI_VAL = 3.14159265358979323846;

// Codebook constants
#define CODEBOOK_SIZE 64
#define EPSILON 0.03
#define DELTA 0.00001

const int NUM_STATES = 5;

// Global recording buffer (3 seconds @ 16025 Hz)
short int recordingBuffer[16025*3];

/* ---------------- Feature extraction helpers ---------------- */

inline void apply_hamming(vector<double>& frame) {
    int N = frame.size();
    for(int i=0; i<N; i++) {
        frame[i] *= (0.54 - 0.46 * cos(2*PI_VAL*i / (N-1)));
    }
}

inline void compute_autocorr(const vector<double>& frame, vector<double>& R) {
    R.assign(LPC_ORDER+1, 0.0);
    int N = frame.size();
    for(int k=0; k<=LPC_ORDER; k++) {
        for(int m= 0; m < N-k; m++) {
            R[k] += frame[m]*frame[m+k];
        }
    }
}

inline double durbin(const vector<double>& R, vector<double>& a) {
    double E = R[0];
    a.assign(LPC_ORDER+1, 0.0);
    vector<double> a_prev(LPC_ORDER+1, 0.0);
    long double k = 0.0L;

    for(int i=1; i<=LPC_ORDER; i++) {
        long double sum = 0.0L;
        for(int j=1; j<=i-1; j++) {
            sum += a_prev[j] * R[i-j];
        }

        k = (R[i]-sum) / E;
        a[i] = (double)k;

        for(int j=1; j<=i-1; j++) {
            a[j] = a_prev[j] - (double)k * a_prev[i-j];
        }

        E = (1 - k*k) * E;

        for(int j=1; j<=i; j++) {
            a_prev[j] = a[j];
        }
    }

    return E;
}

inline void compute_cepstral(const vector<double>& a, vector<double>& C, double gain) {
    C.assign(CEPSTRAL_ORDER+1, 0.0);
    if (gain > 0) {
        C[0] = log(gain);
    }
    else {
        C[0] = 0;
    }

    for(int m=1; m <= CEPSTRAL_ORDER; m++) {
        double sum = 0.0;
        for (int k=1; k<m; k++) {
            sum += (double)k / m * C[k] * a[m-k];
        }
        C[m] = (m <= LPC_ORDER) ? a[m] + sum : sum;
    }
}

inline void lifter(vector<double>& C) {
    for(int m=1; m<=CEPSTRAL_ORDER; m++)
    {
        C[m] *= (1 + (CEPSTRAL_ORDER/2.0) * sin(PI_VAL*m / CEPSTRAL_ORDER));
    }
}

// Extract cepstral feature vectors from raw samples
inline void extract_features(const vector<double>& input_samples, vector<vector<double> >& out_features) {
    if (input_samples.empty()) return;

    vector<double> samples = input_samples;

    double mean = 0.0;
    for(size_t i=0; i<samples.size(); i++) {
        mean += samples[i];
    }
    mean /= samples.size();

    for(size_t i=0; i<samples.size(); i++) {
        samples[i] -= mean;
    }

    double max_amp = 0.0;
    for(size_t i=0; i<samples.size(); i++) {
        if (abs(samples[i]) > max_amp) max_amp = abs(samples[i]);
    }

    if (max_amp > 0) {
        double scale = 5000.0 / max_amp;
        for(size_t i=0; i<samples.size(); i++) {
            samples[i] *= scale;
        }
    }

    int num_samples = static_cast<int>(samples.size());
    for(int start=0; start + FRAME_SIZE <= num_samples; start += FRAME_SHIFT) {
        vector<double> frame(FRAME_SIZE);
        for(int i=0; i<FRAME_SIZE; i++) {
            frame[i] = samples[start + i];
        }

        apply_hamming(frame);

        vector<double> R;
        compute_autocorr(frame, R);

        vector<double> a;
        double E = durbin(R, a);

        vector<double> C;
        compute_cepstral(a, C, E);

        lifter(C);

        vector<double> features;
        for(int i=1; i<=CEPSTRAL_ORDER; i++) {
            features.push_back(C[i]);
        }
        out_features.push_back(features);
    }
}

inline void features_from_file(const string& path, vector<vector<double> >& out_features) {
    ifstream f(path.c_str());
    if (!f.is_open()) return;
    vector<double> samples;
    double v;
    while(f >> v) {
        samples.push_back(v);
    }
    f.close();
    extract_features(samples, out_features);
}

/* ---------------- Codebook (LBG) ---------------- */

class VectorCodebook {
public:
    vector<vector<double> > centroids;

    double compute_distance(const vector<double>& v1, const vector<double>& v2) {
        double d = 0.0;
        for(size_t i=0; i<v1.size(); i++) {
            double diff = v1[i] - v2[i];
            d += diff*diff;
        }
        return d;
    }

    void Train(const vector<vector<double> >& universe) {
        if (universe.empty()) return;
        size_t dim = universe[0].size();

        vector<double> mean(dim, 0.0);
        for(size_t k=0; k<universe.size(); k++) {
            const vector<double>& v = universe[k];
            for(size_t i=0; i<dim; i++) {
                mean[i] += v[i];
            }
        }
        for(size_t i=0; i<dim; i++) {
			mean[i] /= universe.size();
		}

        centroids.clear();
        centroids.push_back(mean);

        while(centroids.size() < CODEBOOK_SIZE) {
            vector<vector<double> > next;
            next.reserve(centroids.size() * 2);
            for(size_t k=0; k<centroids.size(); k++) {
                vector<double> c1 = centroids[k];
                vector<double> c2 = centroids[k];
                for(size_t i=0; i<dim; i++) {
                    c1[i] *= (1.0 + EPSILON);
                    c2[i] *= (1.0 - EPSILON);
                }
                next.push_back(c1);
                next.push_back(c2);
            }
            centroids = next;

            double prev_dist = numeric_limits<double>::max();

            while(true) {
                vector<vector<double> > sums(centroids.size(), vector<double>(dim, 0.0));
                vector<int> counts(centroids.size(), 0);
                double cur_dist = 0.0;

                for(size_t k=0; k<universe.size(); k++) {
                    const vector<double>& v = universe[k];
                    int idx = -1;
                    double best = numeric_limits<double>::max();
                    for(size_t i=0; i<centroids.size(); i++) {
                        double dd = compute_distance(v, centroids[i]);
                        if (dd < best) {
                            best = dd;
                            idx = static_cast<int>(i);
                        }
                    }
                    cur_dist += best;
                    counts[idx]++;
                    for(size_t i=0; i<dim; i++) {
                        sums[idx][i] += v[i];
                    }
                }

                cur_dist /= universe.size();

                if (prev_dist != numeric_limits<double>::max()) {
                    double improvement = (prev_dist - cur_dist) / prev_dist;
                    if (improvement < DELTA) break;
                }
                prev_dist = cur_dist;

                for(size_t i=0; i<centroids.size(); i++) {
                    if (counts[i] > 0) {
                        for (size_t j=0; j<dim; j++) {
                            centroids[i][j] = sums[i][j] / counts[i];
                        }
                    }
                }
            }
        }
    }

    int Quantize(const vector<double>& feature) {
        int best_idx = -1;
        double best = numeric_limits<double>::max();
        for(size_t i=0; i<centroids.size(); i++) {
            double d = compute_distance(feature, centroids[i]);
            if (d < best) {
                best = d;
                best_idx = static_cast<int>(i);
            }
        }
        return best_idx;
    }

    void Save(const string& filename) {
        ofstream out(filename.c_str());
        if (!out.is_open()) {
            cerr << "Unable to open file: " << filename << "\n";
            return;
        }
        if (!centroids.empty()) {
            out << centroids.size() << " " << centroids[0].size() << endl;
            for(size_t k=0; k<centroids.size(); k++) {
                const vector<double>& c = centroids[k];
                for(size_t i=0; i<c.size(); i++) {
                    out << c[i] << (i == c.size() - 1 ? "" : " ");
                }
                out << endl;
            }
        }
        out.close();
    }

    void Load(const string& filename) {
        ifstream in(filename.c_str());
        if (!in.is_open()) {
            cerr << "Unable to open file: " << filename << "\n";
            return;
        }
        int size = 0, dim = 0;
        if (in >> size >> dim) {
            centroids.resize(size, vector<double>(dim));
            for(int i=0; i<size; i++) {
                for(int j=0; j<dim; j++) {
                    in >> centroids[i][j];
                }
            }
        }
        in.close();
    }
};

/* ---------------- HMM implementation ---------------- */

class HmmModel {
public:
    long double A[NUM_STATES][NUM_STATES];
    long double B[NUM_STATES][CODEBOOK_SIZE];
    long double Pi[NUM_STATES];

    HmmModel() {
        Initialize();
    }

    void Initialize() {
        for(int i=0; i<NUM_STATES; i++) {
            Pi[i] = (i == 0) ? 1.0L : 0.0L;
        }

        for(int i=0; i<NUM_STATES; i++) {
            for(int j=0; j<NUM_STATES; j++) A[i][j] = 0.0L;
        }

        for(int i=0; i<NUM_STATES; i++) {
            if (i < NUM_STATES-1) {
                A[i][i] = 0.5L;
                A[i][i+1] = 0.5L;
            }
            else {
                A[i][i] = 1.0L;
            }
        }

        long double uniform = 1.0L / CODEBOOK_SIZE;
        for(int i=0; i<NUM_STATES; i++) {
            for(int k=0; k<CODEBOOK_SIZE; k++) B[i][k] = uniform;
        }
    }

    long double Forward(const vector<int>& O, vector<vector<long double>>& alpha, vector<long double>& scale) {
        int T = O.size();
        alpha.assign(T, vector<long double>(NUM_STATES, 0.0L));
        scale.assign(T, 0.0L);

        for(int i=0; i<NUM_STATES; i++) {
            alpha[0][i] = Pi[i] * B[i][O[0]];
            scale[0] += alpha[0][i];
        }

        if (scale[0] <= 0) scale[0] = 1e-30L;
        for(int i=0; i<NUM_STATES; i++) alpha[0][i] /= scale[0];

        for(int t=1; t<T; t++) {
            scale[t] = 0.0L;
            for(int j=0; j<NUM_STATES; j++) {
                long double sum = 0.0L;
                for(int i=0; i<NUM_STATES; i++) {
                    sum += alpha[t - 1][i] * A[i][j];
                }
                alpha[t][j] = sum * B[j][O[t]];
                scale[t] += alpha[t][j];
            }
            if (scale[t] <= 0) scale[t] = 1e-30L;
            for(int j=0; j<NUM_STATES; j++) alpha[t][j] /= scale[t];
        }

        long double logLikelihood = 0.0L;
        for(int t=0; t<T; t++) logLikelihood += log(scale[t]);
        return logLikelihood;
    }

    void Backward(const vector<int>& O, vector<vector<long double>>& beta, const vector<long double>& scale) {
        int T = O.size();
        beta.assign(T, vector<long double>(NUM_STATES, 0.0L));

        for(int i=0; i<NUM_STATES; i++) beta[T-1][i] = 1.0L / scale[T-1];

        for(int t=T-2; t>=0; t--) {
            for(int i=0; i<NUM_STATES; i++) {
                long double sum = 0.0L;
                for(int j=0; j<NUM_STATES; j++) {
                    sum += A[i][j] * B[j][O[t+1]] * beta[t+1][j];
                }
                beta[t][i] = sum / scale[t];
            }
        }
    }

    void BaumWelch(const vector<vector<int>>& observations) {
        int num_iter = 30;
        int num_obs = observations.size();

        for(int iter=0; iter<num_iter; iter++) {
            long double expA_num[NUM_STATES][NUM_STATES] = {0};
            long double expA_den[NUM_STATES] = {0};
            long double expB_num[NUM_STATES][CODEBOOK_SIZE] = {0};
            long double expB_den[NUM_STATES] = {0};
            long double pi_acc[NUM_STATES] = {0};

            for(int r=0; r<num_obs; r++) {
                const vector<int>& O = observations[r];
                int T = O.size();
                vector<vector<long double>> alpha, beta;
                vector<long double> scale;

                Forward(O, alpha, scale);
                Backward(O, beta, scale);

                vector<vector<long double>> gamma(T, vector<long double>(NUM_STATES));
                for(int t=0; t<T; t++) {
                    long double denom = 0.0L;
                    for(int i=0; i<NUM_STATES; i++) denom += alpha[t][i] * beta[t][i];
                    for(int i=0; i<NUM_STATES; i++) gamma[t][i] = (alpha[t][i] * beta[t][i]) / denom;
                }

                for(int i=0; i<NUM_STATES; i++) pi_acc[i] += gamma[0][i];

                for(int t=0; t<T-1; t++) {
                    long double denom = 0.0L;
                    for(int i=0; i<NUM_STATES; i++) {
                        for(int j=0; j<NUM_STATES; j++) {
                            denom += alpha[t][i] * A[i][j] * B[j][O[t+1]] * beta[t+1][j];
                        }
                    }

                    for(int i=0; i<NUM_STATES; i++) {
                        expA_den[i] += gamma[t][i];
                        for(int j=0; j<NUM_STATES; j++) {
                            long double xi = (alpha[t][i] * A[i][j] * B[j][O[t+1]] * beta[t+1][j]) / denom;
                            expA_num[i][j] += xi;
                        }
                    }
                }

                for(int t=0; t<T; t++) {
                    for(int i=0; i<NUM_STATES; i++) {
                        expB_den[i] += gamma[t][i];
                        expB_num[i][O[t]] += gamma[t][i];
                    }
                }
            }

            for(int i=0; i<NUM_STATES; i++) Pi[i] = pi_acc[i] / num_obs;

            for(int i=0; i<NUM_STATES; i++) {
                for(int j=0; j<NUM_STATES; j++) {
                    if (expA_den[i] > 0) A[i][j] = expA_num[i][j] / expA_den[i];
                    else A[i][j] = (i == j) ? 1.0L : 0.0L;
                }
            }

            for(int i=0; i<NUM_STATES; i++) {
                for(int k=0; k<CODEBOOK_SIZE; k++) {
                    if (expB_den[i] > 0) {
                        B[i][k] = expB_num[i][k] / expB_den[i];
                        if (B[i][k] < 1e-30L) B[i][k] = 1e-30L;
                    }
                    else {
                        B[i][k] = 1.0L / CODEBOOK_SIZE;
                    }
                }
            }
        }
    }

    long double GetProbability(const vector<int>& O) {
        vector<vector<long double>> alpha;
        vector<long double> scale;
        return Forward(O, alpha, scale);
    }

    void Save(const string& filename) {
        ofstream out(filename.c_str());
        if (!out) { cerr << "Unable to save model: " << filename << "\n"; return; }
        out << scientific << setprecision(10);
        for(int i=0; i<NUM_STATES; i++) out << Pi[i] << " ";
        out << endl;
        for(int i=0; i<NUM_STATES; i++) {
            for(int j=0; j<NUM_STATES; j++) out << A[i][j] << " ";
            out << endl;
        }
        for(int i=0; i<NUM_STATES; i++) {
            for(int k=0; k<CODEBOOK_SIZE; k++) out << B[i][k] << " ";
            out << endl;
        }
        out.close();
    }

    void Load(const string& filename) {
        ifstream in(filename.c_str());
        if (!in) { cerr << "Unable to load model: " << filename << "\n"; return; }
        for(int i=0; i<NUM_STATES; i++) in >> Pi[i];
        for(int i=0; i<NUM_STATES; i++) {
            for(int j=0; j<NUM_STATES; j++) in >> A[i][j];
        }
        for(int i=0; i<NUM_STATES; i++) {
            for(int k=0; k<CODEBOOK_SIZE; k++) in >> B[i][k];
        }
        in.close();
    }
};

/* ---------------- Utilities ---------------- */

void record_audio() {
    const int NUMPTS = 16025 * 3;
    int sampleRate = 16025;
    HWAVEIN hWaveIn;
    MMRESULT result;
    WAVEFORMATEX pFormat;
    pFormat.wFormatTag = WAVE_FORMAT_PCM;
    pFormat.nChannels = 1;
    pFormat.nSamplesPerSec = sampleRate;
    pFormat.nAvgBytesPerSec = sampleRate * 2;
    pFormat.nBlockAlign = 2;
    pFormat.wBitsPerSample = 16;
    pFormat.cbSize = 0;

    result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);
    WAVEHDR WaveInHdr;
    WaveInHdr.lpData = (LPSTR)recordingBuffer;
    WaveInHdr.dwBufferLength = NUMPTS * 2;
    WaveInHdr.dwBytesRecorded = 0;
    WaveInHdr.dwUser = 0L;
    WaveInHdr.dwFlags = 0L;
    WaveInHdr.dwLoops = 0L;
    waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    result = waveInStart(hWaveIn);
    cout << "Recording from microphone for 3 seconds...\n";
    Sleep(3 * 1000);
    waveInReset(hWaveIn);
    waveInUnprepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    waveInClose(hWaveIn);
}

string make_filename(int digit, int utterance) {
    stringstream ss;
    ss << "Digits/English/txt/254101040_E_" << digit << "_" << setfill('0') << setw(2) << utterance << ".txt";
    return ss.str();
}

vector<double> read_signal(const string& filename) {
    vector<double> signal;
    ifstream f(filename.c_str());
    if (!f.is_open()) return signal;
    double v;
    while (f >> v) signal.push_back(v);
    f.close();
    return signal;
}

/* ---------------- Main UI ---------------- */
 
int _tmain(int argc, _TCHAR* argv[]) {
    int option;
    VectorCodebook cb;
    HmmModel hmms[10];
    bool modelsLoaded = false;

    while (true) {
        cout << "\n--- Voice Digit Recognizer ---\n";
        cout << "1. Train Models\n";
        cout << "2. Test Using Saved Files\n";
        cout << "3. Test Using Microphone\n";
        cout << "4. Quit\n";
        cout << "Choose an option: ";
        cin >> option;

        if (option == 1) {
            vector<vector<double> > allFeatures;
            cout << "Getting feature data for the codebook...\n";

            for(int d=0; d<=9; d++) {
                for(int u=1; u<=30; u++) {
                    string filename = make_filename(d, u);
                    vector<double> signal = read_signal(filename);
                    if (signal.empty()) continue;
                    vector<vector<double> > features;
                    extract_features(signal, features);
                    allFeatures.insert(allFeatures.end(), features.begin(), features.end());
                }
            }

            if (allFeatures.empty()) {
                cout << "Could not find any feature data. Please check if the files exist.\n";
                continue;
            }

            cout << "Training the codebook using " << allFeatures.size() << " features...\n";
            cb.Train(allFeatures);
            cb.Save("codebook.txt");
            cout << "Codebook saved successfully.\n";

            cout << "Training HMM models for all digits...\n";
            for(int d=0; d<=9; d++) {
                vector<vector<int> > observations;
                for(int u=1; u<=30; u++) {
                    string filename = make_filename(d, u);
                    vector<double> signal = read_signal(filename);
                    if (signal.empty()) continue;
                    vector<vector<double> > features;
                    extract_features(signal, features);
                    vector<int> obsSeq;
                    for(size_t i=0; i<features.size(); i++) {
                        obsSeq.push_back(cb.Quantize(features[i]));
                    }
                    observations.push_back(obsSeq);
                }

                if (observations.empty()) continue;

                hmms[d].BaumWelch(observations);
                stringstream ss;
                ss << "model_digit_" << d << ".txt";
                hmms[d].Save(ss.str());
                cout << "Model for digit " << d << " is ready.\n";
            }
            modelsLoaded = true;
        }
        else if (option == 2) {
            if (!modelsLoaded) {
                cb.Load("codebook.txt");
                for(int d=0; d<=9; d++) {
                    stringstream ss;
                    ss << "model_digit_" << d << ".txt";
                    hmms[d].Load(ss.str());
                }
                modelsLoaded = true;
            }

            int totalTests = 0;
            int correct = 0;

            for(int d=0; d<=9; d++) {
                for(int u=31; u<=40; u++) {
                    string filename = make_filename(d, u);
                    vector<double> signal = read_signal(filename);
                    if (signal.empty()) continue;
                    vector<vector<double> > features;
                    extract_features(signal, features);
                    vector<int> obsSeq;
                    for(size_t i=0; i<features.size(); i++) {
                        obsSeq.push_back(cb.Quantize(features[i]));
                    }

                    double maxProb = -1e308;
                    int recognizedDigit = -1;

                    for(int m=0; m<=9; m++) {
                        double prob = (double)hmms[m].GetProbability(obsSeq);
                        if (prob > maxProb) {
                            maxProb = prob;
                            recognizedDigit = m;
                        }
                    }

                    cout << "Checked: " << filename << "  Found: " << recognizedDigit << "  Expected: " << d << "\n";
                    if (recognizedDigit == d) correct++;
                    totalTests++;
                }
            }
            if (totalTests > 0) cout << "Final accuracy: " << (double)correct / totalTests * 100.0 << "%\n";
            else cout << "No test files available.\n";
        }
        else if (option == 3) {
            if (!modelsLoaded) {
                cb.Load("codebook.txt");
                for(int d=0; d<=9; d++) {
                    stringstream ss;
                    ss << "model_digit_" << d << ".txt";
                    hmms[d].Load(ss.str());
                }
                modelsLoaded = true;
            }

            record_audio();

            vector<double> speechSamples;
            int frameSize = 100;
            long long energyThreshold = 100000;
            int totalSamples = 16025 * 3;
            int numFrames = totalSamples / frameSize;

            for(int i=0; i<numFrames; i++) {
                long long sum = 0;
                for(int j=0; j<frameSize; j++) {
                    int idx = i*frameSize + j;
                    sum += (long long)recordingBuffer[idx] * recordingBuffer[idx];
                }
                long long avgEnergy = sum / frameSize;
                if (avgEnergy > energyThreshold) {
                    for(int j=0; j<frameSize; j++) {
                        speechSamples.push_back((double)recordingBuffer[i*frameSize + j]);
                    }
                }
            }

            if (speechSamples.size() < 2500) {
                cout << "Sorry, I couldn't hear any clear speech. Try speaking a bit louder.\n";
                continue;
            }

            vector<vector<double> > features;
            extract_features(speechSamples, features);
            vector<int> obsSeq;
            for(size_t i=0; i<features.size(); i++) {
                obsSeq.push_back(cb.Quantize(features[i]));
            }

            double maxProb = -1e308;
            int recognizedDigit = -1;

            for(int m=0; m<=9; m++) {
                double prob = (double)hmms[m].GetProbability(obsSeq);
                if (prob > maxProb) {
                    maxProb = prob;
                    recognizedDigit = m;
                }
            }

            cout << "Detected digit: " << recognizedDigit << "\n";
        }
        else if (option == 4) {
            break;
        }
    }
    return 0;
}
