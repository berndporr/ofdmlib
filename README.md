# ofdmlib
Efficient, object-oriented OFDM library designed and implemented to work in real-time.

  <p align="center">
     

</div>

<!-- TOC -->
<details open="open">
  <summary><h2 style="display: inline-block">Contents</h2></summary>
  <ol>
    <li>
      <a href="#about">About</a>
      <ul>
        <li><a href="#software">Software</a></li>
        <li><a href="#roadmap">Roadmap</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
        <li><a href="#usage">Usage</a></li>
      </ul>
    </li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#acknowledgements">Acknowledgements</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

<!-- Project descirption -->
## About

ofdmlib is a flexible orthogonal-frequency division multiplexing(OFDM) C++ library implemented in object-oriented fashion
<br />
<br />

<div align="center">

[![Contributors](https://img.shields.io/github/contributors/krogk/ofdmlib.svg?style=for-the-badge)](https://github.com/krogk/ofdmlib/graphs/contributors)
[![Forks](https://img.shields.io/github/forks/krogk/ofdmlib.svg?style=for-the-badge)](https://github.com/krogk/ofdmlib/network/members)
[![Stars](https://img.shields.io/github/stars/krogk/ofdmlib.svg?style=for-the-badge)](https://github.com/krogk/ofdmlib/stargazers)
[![Issues](https://img.shields.io/github/issues/krogk/ofdmlib.svg?style=for-the-badge)](https://github.com/krogk/ofdmlib/issues)
[![License](https://img.shields.io/github/license/krogk/ofdmlib.svg?style=for-the-badge)](https://github.com/krogk/ofdmlib/blob/main/LICENSE)

<br />

</div>

### Software

Currently ofdmlib is in early development stages, you can view the [Roadmap](https://github.com/krogk/ofdmlib#roadmap) for formally planned features. 

[See Doxygen Documentation (Place holder for now)](https://krogk.github.io/ofdmlib/software/docs/Doxygen/html/index.html)
<br />
<br />


Below are a handfull of the features which are currently implemented as a part of the OFDM system and some which are planned for future release (If you have any suggestions just send me a message).

Quadrature Modulators:
  [x] Nyquist Modulator


Sub-Carrier Modulations:
  [x]  4-QAM
  []  16-QAM
  []  64-QAM
  []  256-QAM


Synchronization:
  [x] Fine Search using real valued pilot tones
  [] Schmidl & Cox [Method](https://core.ac.uk/download/pdf/193988246.pdf)


Detector:
 Correlator:
  [x] Time-Domain Correlator (Accumulator used to reduce execution time)
  []  Frequency-Domain Correlator (Cyclic Convolution)


Correlation Peak Search:
  [x] Robust Schmitt trigger like Thresholding 
  [] Statistical Methods - TBD (for example z-score is interesting)


Pilot Tone Cofnigurations:
  [] Block Type
  [x] Comb-Type
  [] Scattered
  [] Circular


Channel Estimation Methods:
  [] Adaptive filter (utilizing FIR Structure)
  [x] Linear Interpolation
  [] High-order Interpolations


FFT Algorithms Support
  [x] FFTW3
  [] kfr 


FEC:
  [] RS
  [] LDPC
  [] Convolution

Multiple Access Methods:
  []


[] Multi-threading Adaptation


### Roadmap

[Roadmap](https://github.com/krogk/ofdmlib/blob/main/docs/diagrams/roadmap/Roadmap.png)

<!-- Getting Started -->
## Getting Started

### Prerequisites
1. Clone the repository
```sh
git clone https://github.com/krogk/ofdmlib
```
2. Ubuntu Packages: 
```sh
chmod +x devsetup.sh
./devsetup.sh
```

### Installation

1.Build & Test:
```sh
mkdir build
cd build/
cmake ..
make test ARGS="-V"  <--- Its not neccesary but highly recommended to run the test suite
make
```

<!-- Usage -->
### Usage

1. See [Example]()
```sh
#include <ofdmcodec.h>
```

<!-- Contributing -->
## Contributing

See `CONTRIBUTING` for more information.

<!-- Acknowledgements -->
## Acknowledgements

Huge thank you to my supervisor [Dr Bernd Porr](https://github.com/berndporr) for the opportunity to work on this and the guidance through the project.
Dr Porr has published vast amount of educational material in form of [videos](https://www.youtube.com/watch?v=VhgkCoVYhBI&list=PLvUvEbh7a8u_Wqtn7VpVhh_eZALqmLCQh) on the subject of the OFDM which are highly reccomended.

<!-- License -->
## License

Distributed under the MIT License. See `LICENSE` for more information.
This Implementation is based on FFTW

<!-- Contact Info -->
## Contact

[Kamil Rog](https://github.com/krogk)

