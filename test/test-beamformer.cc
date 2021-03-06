// test-beamformer.cc
// wujian@2018

#include "util/common-utils.h"
#include "include/complex-base.h"
#include "include/beamformer.h"
#include "include/complex-vector.h"
#include "include/complex-matrix.h"

using namespace kaldi;

void create_hermite_cmatrix(CMatrix<BaseFloat> *cm, MatrixIndexT s) {
    cm->Resize(s, s);
    cm->SetRandn();
    for (MatrixIndexT i = 0; i < s; i++) {
        for (MatrixIndexT j = 0; j < i; j++) {
            (*cm)(j, i, kReal) = (*cm)(i, j, kReal);
            (*cm)(j, i, kImag) = -(*cm)(i, j, kImag);
        }
        (*cm)(i, i, kImag) = 0;
    }
}


void test_string_spliter() {
    std::string scp = "scp:CH1.scp,CH2.scp,CH3.scp";
    std::vector<std::string> tokens;    
    size_t found = scp.find_first_of(":", 0);
    if (found != std::string::npos)
        std::cout << scp.substr(0, found) << std::endl;
    SplitStringToVector(scp.substr(found + 1), ",", false, &tokens);
    for (std::string &s: tokens)
        std::cout << s << std::endl;
}

void test_estimate_psd() {
    for (int32 i = 0; i < 10; i++) {
        int32 f = Rand() % 6 + 4, t = Rand() % 6 + 4, c = Rand() % 5 + 3;
        CMatrix<BaseFloat> src_stft(f * t, c), psd; 
        Matrix<BaseFloat> mask(t, f);
        src_stft.SetRandn();
        mask.SetRandn();
        EstimatePsd(src_stft, mask, &psd, NULL);
        std::cout << "f = " << f << ", t = " << t << ", c = " << c << std::endl;
        for (int32 j = 0; j < f; j++) {
            SubCMatrix<BaseFloat> covar(psd, j * c, c, 0, c);
            KALDI_ASSERT(covar.IsHermitian());
        }
    }
}

void test_beamform() {
    for (int32 i = 0; i < 10; i++) {
        int32 f = Rand() % 6 + 4, t = Rand() % 6 + 4, c = Rand() % 5 + 3;
        CMatrix<BaseFloat> src_stft(f * t, c), weights(f, c), enh_stft; 
        src_stft.SetRandn();
        weights.SetRandn();
        weights.Conjugate();
        Beamform(src_stft, weights, &enh_stft);
        std::cout << "f = " << f << ", t = " << t << ", c = " << c << std::endl;
        std::cout << enh_stft;
    }
}

void test_estimate_steervector() {
    for (int32 i = 0; i < 10; i++) {
        int32 f = Rand() % 6 + 4, t = Rand() % 6 + 4, c = Rand() % 5 + 3;
        CMatrix<BaseFloat> psd(f * c, c), hmat, sv;
        for (int32 j = 0; j < f; j++) {
            create_hermite_cmatrix(&hmat, c);
            psd.RowRange(j * c, c).CopyFromMat(hmat);
        }
        std::cout << "f = " << f << ", t = " << t << ", c = " << c << std::endl;
        EstimateSteerVector(psd, &sv);
        std::cout << sv;
    }
}

void test_compute_mvdr_beamweights() {
    for (int32 i = 0; i < 10; i++) {
        int32 f = Rand() % 6 + 4, t = Rand() % 6 + 4, c = Rand() % 5 + 3;
        CMatrix<BaseFloat> psd(f * c, c), hmat, weights, sv(f, c);
        sv.SetRandn();
        for (int32 j = 0; j < f; j++) {
            create_hermite_cmatrix(&hmat, c);
            psd.RowRange(j * c, c).CopyFromMat(hmat);
        }
        std::cout << "f = " << f << ", t = " << t << ", c = " << c << std::endl;
        ComputeMvdrBeamWeights(psd, sv, &weights);
        std::cout << weights;
    }
}

void test_trim_stft() {
    for (int32 i = 0; i < 10; i++) {
        int32 f = Rand() % 6 + 4, t = Rand() % 6 + 4, c = Rand() % 4 + 2;
        CMatrix<BaseFloat> src_stft(t, f * c);
        src_stft.SetRandn();
        for (int32 j = 0; j < c; j++)
            std::cout << "CH " << j << " :\n" << src_stft.ColRange(j * f, f);
        CMatrix<BaseFloat> dst_stft;
        TrimStft(f, c, src_stft, &dst_stft);
        std::cout << dst_stft;
    }
}


int main() {
    // test_estimate_psd();
    // test_beamform();
    // test_estimate_steervector();
    // test_compute_mvdr_beamweights();
    // test_trim_stft();
    test_string_spliter();
    return 0;
}
