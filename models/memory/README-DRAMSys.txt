
------------------------------------------------------------------------------
Initial support to DRAMSys TLM2.0 memory models
------------------------------------------------------------------------------

DRAMSys which is a flexible DRAM subsystem design space exploration framework
that consists of models reflecting the DRAM functionality, power consumption,
retention time errors, etc.

Requirements to use this feature:

- Access to DRAMSys repository (open-sourcing process in progress).

- Provided that one has access to the repository, it should be cloned as
  follows:

$ cd models/memory
$ git clone --recursive <DRAMSys repo URL>

- Check DRAMSys project's documentation, follow the steps on README.md in
  order to get all the dependencies installed in your system and learn about
  the features provided.

- Two environment variables influence the build process and should be defined
  in order to use DRAMSys. They are:

export VP_USE_SYSTEMC=1
export VP_USE_SYSTEMC_DRAMSYS=1

Optionally, these variables can be added to an initialization file such as
~/.bashrc.

Currently, in this first implementation, users should adapt the source code to
load the desired DRAMSys configuration file and rebuild.

See "models/memory/ddr_sc_impl.cpp". You may want to double check or change
the values below:

define BYTES_PER_ACCESS            8
define ACCEPT_DELAY_PS             1000
string simulation_xml = resources + "simulations/ddr3-example.xml";

