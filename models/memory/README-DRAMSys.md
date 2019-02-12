
## Support to DRAMSys TLM2.0 memory models

[DRAMSys](https://www.jstage.jst.go.jp/article/ipsjtsldm/8/0/8_63/_article) is a flexible DRAM subsystem design space exploration framework that consists of models reflecting the DRAM functionality, power consumption, retention time errors, etc.

Requirements to use this feature:

Access to DRAMSys repository (open-sourcing process in progress).

Provided that one has access to the repository, it should be cloned as follows:

```bash
$ cd models/memory
$ git clone --recursive <DRAMSys repo URL>
```

Check DRAMSys project's documentation, follow the steps on README.md in order to get all the dependencies installed in your system and learn about the features provided.

Two environment variables influence the build process and should be defined in order to use DRAMSys. They are:

```bash
export VP_USE_SYSTEMC=1
export VP_USE_SYSTEMC_DRAMSYS=1
```

Optionally, these variables can be added to an initialization file such as ~/.bashrc.

Currently, in this first implementation, users should adapt the source code to load the desired DRAMSys configuration file and rebuild.

See *models/memory/ddr_sc_impl.cpp* for more details. You may want to double check or change the values below:

```
define BYTES_PER_ACCESS            8
define ACCEPT_DELAY_PS             1000
string simulation_xml = resources + "simulations/ddr3-example.xml";
```

## References

[1] TLM Modelling of 3D Stacked Wide I/O DRAM Subsystems, A Virtual Platform for Memory Controller Design Space Exploration
M. Jung, C. Weis, N. Wehn, K. Chandrasekar. International Conference on High-Performance and Embedded Architectures and Compilers 2013 (HiPEAC), Workshop on: Rapid Simulation and Performance Evaluation: Methods and Tools (RAPIDO), January, 2013, Berlin.

[2] DRAMSys: A flexible DRAM Subsystem Design Space Exploration Framework
M. Jung, C. Weis, N. Wehn. Accepted for publication, IPSJ Transactions on System LSI Design Methodology (T-SLDM), October, 2015.

