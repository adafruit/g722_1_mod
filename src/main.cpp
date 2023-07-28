#include <pybind11/pybind11.h>
#include <algorithm>

extern "C" {
#include "au/au_header.c"
#include "au/endianess_t.c"
#include "au/wav_header.c"
#include "common/basop32.c"
#include "common/common.c"
#include "common/count.c"
#include "common/huff_tab.c"
#include "common/tables.c"
#include "encode/dct4_a.c"
#include "encode/encode.c"
#include "encode/encoder.c"
#include "encode/sam2coef.c"
}

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#if defined(__linux__)
#include <endian.h>
#else
// all others assumed to be little-endian
#define htole16(x) (x)
#define le16toh(x) (x)
#endif

namespace py = pybind11;

#define MAX_SAMPLE_RATE 32000
#define MAX_FRAMESIZE   (MAX_SAMPLE_RATE/50)

py::bytes encode(py::bytes bytes_in, size_t input_frame_size=320, size_t output_frame_size=80) {
    if(input_frame_size > MAX_FRAMESIZE) {
        throw py::value_error("Invalid input_frame_size");
    }
    std::string data_in{bytes_in};
    Word16 mlt_coefs[MAX_FRAMESIZE];
    Word16 input[MAX_FRAMESIZE];
    Word16 history[MAX_FRAMESIZE]{};
    Word16 out_words[MAX_BITS_PER_FRAME / 16];
    std::string result;

    for(size_t i=0; i<std::size(data_in); i+=2*input_frame_size) {
        size_t e = std::min(std::size(data_in), i + 2*input_frame_size);
        std::fill(std::copy(reinterpret_cast<Word16*>(&data_in[i]), reinterpret_cast<Word16*>(&data_in[e]), input), std::end(input), 0);

        for(size_t i=0; i < MAX_FRAMESIZE; i++)
            input[i] = le16toh(input[i]);

        auto mag_shift = samples_to_rmlt_coefs(input, history, mlt_coefs, input_frame_size);

        /* Encode the mlt coefs */
        encoder(output_frame_size * 8,
                NUMBER_OF_REGIONS,
                mlt_coefs,
                mag_shift,
                out_words);

        for(size_t i=0; i < MAX_BITS_PER_FRAME / 16; i++)
            out_words[i] = htole16(out_words[i]);

        result.append(reinterpret_cast<char*>(&out_words[0]), output_frame_size);
    }

    return py::bytes(result);
}

PYBIND11_MODULE(g722_1_mod, m) {
    m.doc() = R"pbdoc(
        Modified G722.1 Encoder
        -----------------------

        .. currentmodule:: g722_1_mod

        .. autosummary::
           :toctree: _generate

           encode
    )pbdoc";

    m.def("encode", &encode,
            py::arg("data"),
            py::arg("input_frame_size") = 320,
            py::arg("output_frame_size") = 80,
            R"pbdoc(Encode raw S16LE audio into modified G722.1 format)pbdoc");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
