# BOXR

[![MIT licensed](https://img.shields.io/badge/license-MIT-red.svg)](LICENSE)
[![NCSA licensed](https://img.shields.io/badge/license-NCSA-blue.svg)](LICENSE)


<!-- <a href="https://youtu.be/GVcCW8WgEDY">
    <img
        alt="ILLIXR Simple Demo"
        src="https://img.youtube.com/vi/GVcCW8WgEDY/0.jpg"
        style="width: 320px"
        class="center"
    >
</a> -->

Illinois Extended Reality testbed or ILLIXR (pronounced like elixir) is
    the first fully open-source Extended Reality (XR) system and testbed.
The modular, extensible, and [OpenXR][26]-compatible ILLIXR runtime
    integrates state-of-the-art XR components into a complete XR system.
The testbed is part of the broader [ILLIXR consortium][37],
    an industry-supported community effort to democratize XR systems
    research, development, and benchmarking.

You can find the complete ILLIXR system [here][38].

ILLIXR also provides its components in standalone configurations to enable architects and
    system designers to research each component in isolation.
The standalone components are packaged together in the as of the [v3.1.0 release][39] of ILLIXR. 

ILLIXR's modular and extensible runtime allows adding new components and swapping different
    implementations of a given component.
ILLIXR currently contains the following components: 

-   *Perception*
    -   Eye Tracking
        1.  [RITNet][3] **
    -   Scene Reconstruction
        1.  [ElasticFusion][2] **
        2.  [KinectFusion][40] **
    -   Simultaneous Localization and Mapping
        1.  [OpenVINS][1] **
    -   Cameras and IMUs
        1.  [ZED Mini][42]
        2.  [Intel RealSense][41]

-   *Visual*
    -   [Chromatic aberration correction][5]
    -   [Computational holography for adaptive multi-focal displays][6] **
    -   [Lens distortion correction][5]
    -   [Asynchronous Reprojection (TimeWarp)][5]

-   *Aural*
    -   [Audio encoding][4] **
    -   [Audio playback][4] **

(** Source is hosted in an external repository under the [ILLIXR project][7].)

We continue to add more components (new components and new implementations). 

Many of the current components of ILLIXR were developed by domain experts and obtained from
    publicly available repositories.
They were modified for one or more of the following reasons: fixing compilation, adding features,
    or removing extraneous code or dependencies.
Each component not developed by us is available as a forked github repository for
    proper attribution to its authors.

# Papers, talks, demos, consortium

A [paper][8] with details on ILLIXR, including its components, runtime, telemetry support,
    and a comprehensive analysis of performance, power, and quality on desktop and embedded systems.

A [talk presented at NVIDIA GTC'21][42] describing ILLIXR and announcing the ILLIXR consortium:
    [Video][43].
    [Slides][44]. 

A [demo][45] of an OpenXR application running with ILLIXR.

For more up-to-date list of related papers, demos, and talks, please visit [illixr.org][37].

The [ILLIXR consortium][37] is an industry-supported community effort to democratize
    XR systems research, development, and benchmarking.
Visit our [web site][37] for more information.

The ILLIXR consortium is also holding a biweekly consortium meeting. For past meetings, for more information, past meeting recordings, and request for presenting, please visit [here][50]. Please join our [Discord][47] for announcement. 

# Citation

Please cite our following paper "BOXR: Body and head motion Optimization framework for eXtended Reality" when you use BOXR for a publication.

```

```

## Getting Started and Documentation



## Acknowledgements

We would like to extend our sincere gratitute to [Sarita Adve’s research group][4] for their [ILLIXR project][3]. Please also consider citing them for the original ILLIXR implementation.

BOXR's software part is developed based on the ILLIXR framework, and hardware part is based on the open-sourced [Northstar Next][1] Headmount Display.

## Licensing Structure

BOXR is available as open-source software under the [MIT License](LICENSE).
**The MIT license is limited to only this software**.
The external libraries and softwares included in BOXR each have their own licenses and must be used according to those licenses:

- [ILLIXR][3] \ [University of Illinois/NCSA Open Source License][2]

Please refer to [ILLIXR][3] for the nested license for the used component


## Get in Touch

Whether you are a computer architect, a compiler writer, a systems person, work on XR related algorithms
    or applications, or just anyone interested in XR research, development, or products,
    we would love to hear from you and hope you will contribute!
You can join
    the [ILLIXR consortium][37],
    [Discord][47],
    or [mailing list][48],
    or send us an [email][49],
    or just send us a pull request!


[//]: # (- References -)

[1]:    https://docs.projectnorthstar.org/project-north-star/northstar-next
[2]:    https://github.com/ILLIXR/ILLIXR/blob/master/LICENSE
[3]:    https://github.com/ILLIXR
[4]:    http://rsim.cs.illinois.edu

