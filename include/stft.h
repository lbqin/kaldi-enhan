// include/stft.h
// wujian@18.2.12

// Copyright 2018 Jian Wu

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.


#ifndef STFT_H_
#define STFT_H_

#include "matrix/matrix-lib.h"
#include "base/kaldi-common.h"
#include "feat/wave-reader.h"
#include "util/common-utils.h"

namespace kaldi {

struct ShortTimeFTOptions {
    BaseFloat frame_shift;
    BaseFloat frame_length;
    std::string window;

    bool normalize_input, enable_scale;
    bool volumn;
    bool apply_pow;
    bool apply_log;

    ShortTimeFTOptions(): frame_shift(256), frame_length(1024), 
        window("hamming"), normalize_input(false), enable_scale(false),
        apply_log(false), apply_pow(false) {}

    int32 PaddingLength() {
        return RoundUpToNearestPowerOfTwo(frame_length);
    }

    void Register(OptionsItf *opts) {
        opts->Register("frame-shift", &frame_shift, "Frame shift in number of samples");
        opts->Register("frame-length", &frame_length, "Frame length in number of samples");
        opts->Register("window", &window, "Type of window(\"hamming\"|\"hanning\"|\"blackman\"|\"rectangular\")");
        opts->Register("normalize-input", &normalize_input, "Scale samples into range [-1, 1], like MATLAB or librosa");
        opts->Register("enable-scale", &enable_scale, "Let infinite norm of sample vector to be one");
        opts->Register("apply-pow", &apply_pow, "Using power spectrum instead of magnitude spectrum. "
                                                "This options only works when computing (Power/Magnitude) spectrum"
                                                " and corresponding wave reconstruction(egs: wav-estimate).");
        opts->Register("apply-log", &apply_log, "Apply log on computed spectrum if needed.");
    }
};


class ShortTimeFTComputer {
public:
    ShortTimeFTComputer(const ShortTimeFTOptions &opts): 
        opts_(opts), frame_shift_(opts.frame_shift), frame_length_(opts.frame_length) {
        CacheWindow(opts_); 
        srfft_ = new SplitRadixRealFft<BaseFloat>(opts_.PaddingLength());
    }

    ~ShortTimeFTComputer() { delete srfft_; }

    // Run STFT to transform int16 samples into stft results(complex)
    // format of spectra: [r0, r(n/2-1), r1, i1, ... r(n/2-2), i(n/2-2)]
    // note that i0 == i(n/2-1) == 0, egs:
    // for 
    // array([ 0.99482657,  0.79233322,  0.22403132,  0.97833733,  0.18446946,
    //         0.95973959,  0.06612171,  0.99894346,  0.75699571,  0.86274655,
    //         0.19091095,  0.4701981 ,  0.45053227,  0.35169552,  0.34164015,
    //         0.65699885])
    // using rfft, then get
    // [ 9.28052077+0.j          0.03687047-0.69766529j  1.50647284-0.10351507j
    //  -0.04591224+0.08162118j  1.56411987+0.13796286j  0.08509277+0.27094417j
    //  0.72716829-0.08915424j  0.87527244-1.57259355j -2.86146448+0.j        ]
    void ShortTimeFT(const MatrixBase<BaseFloat> &wave, Matrix<BaseFloat> *stft);

    // using overlapadd to reconstruct waveform from realfft's complex results
    void InverseShortTimeFT(MatrixBase<BaseFloat> &stft, Matrix<BaseFloat> *wave, 
                            BaseFloat range = 0);

    // compute spectrogram from stft results, abs(i^2 + r^2) or i^2 + r^2...)
    void ComputeSpectrogram(MatrixBase<BaseFloat> &stft, Matrix<BaseFloat> *spectra);

    // compute phase angle from stft results
    void ComputePhaseAngle(MatrixBase<BaseFloat> &stft, Matrix<BaseFloat> *angle);

    // restore stft(complex) results using spectrogram(magnitude) & angle(phase angle) 
    void Polar(MatrixBase<BaseFloat> &spectra, MatrixBase<BaseFloat> &angle, 
               Matrix<BaseFloat> *stft);

    // compute stft stats from raw waveform, calls above internal
    void Compute(const MatrixBase<BaseFloat> &wave, Matrix<BaseFloat> *stft, 
                 Matrix<BaseFloat> *spectra, Matrix<BaseFloat> *angle); 


private:
    void CacheWindow(const ShortTimeFTOptions &opts);

    ShortTimeFTOptions opts_;
    SplitRadixRealFft<BaseFloat> *srfft_;

    Vector<BaseFloat> window_;

    BaseFloat frame_shift_;
    BaseFloat frame_length_;

    BaseFloat int16_max = static_cast<BaseFloat>(std::numeric_limits<int16>::max());
    BaseFloat float_inf = static_cast<BaseFloat>(std::numeric_limits<BaseFloat>::infinity());

    // keep same as Kaldi's
    int32 NumFrames(int32 num_samples) {
        return static_cast<int32>((num_samples - opts_.frame_length) / opts_.frame_shift) + 1;
    }

    int32 NumSamples(int32 num_frames) {
        return static_cast<int32>((num_frames - 1) * opts_.frame_shift + opts_.frame_length);
    }
};

}

#endif
