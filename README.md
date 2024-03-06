[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<br />
<div align="center">
  <a href="https://github.com/zones-convolution/zones_convolver">
    <img src="images/zones_icon.png" alt="Logo" width="80" height="80">
  </a>

<h3 align="center">Zones Convolver</h3>

  <p align="center">

Convolution library used in the upcoming Zones Convolution plugin. Implements a non-uniform partitioned convolution (
NUPC) scheme with modified Garcia optimal partitioning and time distributed transforms. This is able to run on a single
thread without large spikes in load for variable block sizes without additional latency.

Currently, the library is in a first draft state and has not yet seen real use, therefore it **should be
used with caution**. When writing the library we have often prioritised readability at the cost of performance and we
aim to improve on this in the future.
<br />
<a href="https://github.com/zones-convolution/zones_convolver/"><strong>Explore the docs »</strong></a>
<br />
<a href="https://github.com/zones-convolution/zones_convolver/issues">Report Bug</a>
·
<a href="https://github.com/zones-convolution/zones_convolver/issues">Request Feature</a>
  </p>
</div>

## Usage

---

## Contributing

We would really welcome any contributions, feel free to open up a PR and we'll review it as soon as possible.

We're looking to expand the library in the future to be as comprehensive as possible.

Our current roadmap includes,

* Radix 4 decompositions and corresponding scheduling
* Real only transforms
* Micro-optimisations across the library
* Multi-threaded NUPC implementation
* Moving away from JUCE to be more flexible in other use cases

---

## License

Distributed under the MIT License. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

## Contact

Feel free to get in touch,

**Leon Paterson-Stephens** - leon@leonps.com

**Micah Strange** - micahstrange16@gmail.com

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

## Acknowledgments / References

* **Frank Wefers** - Partitioned convolution algorithms for real-time auralization
* **Jeffrey R. Hurchalla** - A Time Distributed FFT for Efficient Low Latency Convolution
* **Eric Battenberg, Rimas Avizienis** - IMPLEMENTING REAL-TIME PARTITIONED CONVOLUTION ALGORITHMS ON CONVENTIONAL
  OPERATING SYSTEMS
* **Guillermo García** - Optimal Filter Partition for Efficient Convolution with Short Input/Output Delay
* **Graham Barab** - [RTCONVOLVE](https://github.com/grahman/RTConvolve)
* **JUCE** - [juce::dsp::Convolution](https://docs.juce.com/master/classdsp_1_1Convolution.html)
* **Jan Wilczek
  ** - [Fast Convolution: FFT-based, Overlap-Add, Overlap-Save, Partitioned](https://thewolfsound.com/fast-convolution-fft-based-overlap-add-overlap-save-partitioned/#the-convolution-series)
* [Readme Boilerplate](https://github.com/othneildrew/Best-README-Template)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

[contributors-shield]: https://img.shields.io/github/contributors/github_username/repo_name.svg?style=for-the-badge

[contributors-url]: https://github.com/zones-convolution/zones_convolver/graphs/contributors

[forks-shield]: https://img.shields.io/github/forks/github_username/repo_name.svg?style=for-the-badge

[forks-url]: https://github.com/zones-convolution/zones_convolver/network/members

[stars-shield]: https://img.shields.io/github/stars/github_username/repo_name.svg?style=for-the-badge

[stars-url]: https://github.com/zones-convolution/zones_convolver/stargazers

[issues-shield]: https://img.shields.io/github/issues/github_username/repo_name.svg?style=for-the-badge

[issues-url]: https://github.com/zones-convolution/zones_convolver/issues

[license-shield]: https://img.shields.io/github/license/github_username/repo_name.svg?style=for-the-badge

[license-url]: https://github.com/zones-convolution/zones_convolver/blob/main/LICENSE
