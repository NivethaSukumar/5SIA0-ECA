# Copyright (c) 2012 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2006-2008 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Steve Reinhardt

# Simple test script
#
# "m5 test.py"

import os
import optparse
import sys

import m5
from m5.defines import buildEnv
from m5.objects import *
from m5.util import addToPath, fatal

addToPath('../common')
addToPath('../topologies')
import Options
import Simulation
import CacheConfig
from Caches import *
from cpu2000 import *
#from FUPool import FUPool

### Create the Options Parser
parser = optparse.OptionParser()
Options.addCommonOptions(parser)
Options.addSEOptions(parser)

### Parse for command line options
(options, args) = parser.parse_args()

### Override some options values to match the ARM Cortex A9 

options.cpu_type = "detailed"   # The A9 is an OutOfOrder CPU
options.cpu_clock = "2GHz"
options.num_cpus = 1

options.caches = 1              # Symmetric, L1 caches
options.cacheline_size = 64

options.l1i_size = "64B"
options.l1i_assoc = 1

options.l1d_size = "64B"
options.l1d_assoc = 1

options.l2cache = 0
options.l2_size = "16kB"
options.l2_assoc = 4

if args:
    print "Error: script doesn't take any positional arguments"
    sys.exit(1)

### Setup the workload to execute on the CPUs
multiprocesses = []
apps = []

if options.cmd:
    process = LiveProcess()
    process.executable = options.cmd
    process.cmd = [options.cmd] + options.options.split()
    multiprocesses.append(process)
else:
    print >> sys.stderr, "No workload specified. Exiting!\n"
    sys.exit(1)

### Optionally pipe output to a file

if options.input != "":
    process.input = options.input
if options.output != "":
    process.output = options.output
if options.errout != "":
    process.errout = options.errout

# By default, set workload to path of user-specified binary
workloads = options.cmd
numThreads = 1

if options.cpu_type == "detailed" or options.cpu_type == "inorder":
    #check for Simultaneous Multithreaded workload
    workloads = options.cmd.split(';')
    if len(workloads) > 1:
        process = []
        smt_idx = 0
        inputs = []
        outputs = []
        errouts = []

        if options.input != "":
            inputs = options.input.split(';')
        if options.output != "":
            outputs = options.output.split(';')
        if options.errout != "":
            errouts = options.errout.split(';')

        for wrkld in workloads:
            smt_process = LiveProcess()
            smt_process.executable = wrkld
            smt_process.cmd = wrkld + " " + options.options
            if inputs and inputs[smt_idx]:
                smt_process.input = inputs[smt_idx]
            if outputs and outputs[smt_idx]:
                smt_process.output = outputs[smt_idx]
            if errouts and errouts[smt_idx]:
                smt_process.errout = errouts[smt_idx]
            process += [smt_process, ]
            smt_idx += 1
    numThreads = len(workloads)

### Using the provided options, setup the CPU and cache configuration

(CPUClass, test_mem_mode, FutureClass) = Simulation.setCPUClass(options)
CPUClass.numThreads = numThreads;

### Select the CPU count

np = options.num_cpus

### Assemble the system

system = System(cpu = [CPUClass(cpu_id=i) for i in xrange(np)],
                physmem = SimpleMemory(range=AddrRange("512MB")),
                membus = SystemXBar(), mem_mode = test_mem_mode)

# Create a top-level voltage domain
system.voltage_domain = VoltageDomain(voltage = options.sys_voltage)

# Create a source clock for the system and set the clock period
system.clk_domain = SrcClockDomain(clock =  options.sys_clock,
                                   voltage_domain = system.voltage_domain)

# Create a CPU voltage domain
system.cpu_voltage_domain = VoltageDomain()

# Create a separate clock domain for the CPUs
system.cpu_clk_domain = SrcClockDomain(clock = options.cpu_clock,
                                       voltage_domain =
                                       system.cpu_voltage_domain)

### Reconfigure the CPU to match the Cortex A15 specs

cpu = system.cpu[0]
cpu.decodeWidth = 3     # 3 way superscalar frontend
cpu.fetchWidth = 3
cpu.issueWidth = 3
cpu.dispatchWidth = 3

#Functional Unit definitions taken from FuncUnitConfig.py, modified to match A15

class IntALU(FUDesc):
    opList = [ OpDesc(opClass='IntAlu') ]
    count = 2

class IntMult(FUDesc):
    opList = [ OpDesc(opClass='IntMult', opLat=3)]
    count = 1

class IntDiv(FUDesc):
    opList = [OpDesc(opClass='IntDiv', opLat=20)]
    count = 1 

class RdWrPort(FUDesc):
    opList = [ OpDesc(opClass='MemRead'), OpDesc(opClass='MemWrite') ]
    count = 1

# Attach the Functional units
cpu.fuPool = FUPool(FUList=[IntALU(), IntMult(), IntDiv(), RdWrPort()])

### Sanity checks

if options.fastmem and (options.caches or options.l2cache):
    fatal("You cannot use fastmem in combination with caches!")

for i in xrange(np):
    if len(multiprocesses) == 1:
        system.cpu[i].workload = multiprocesses[0]
    else:
        system.cpu[i].workload = multiprocesses[i]

    if options.fastmem:
        system.cpu[i].fastmem = True

    if options.checker:
        system.cpu[i].addCheckerCpu()

### Connect up system memory

system.system_port = system.membus.slave
system.physmem.port = system.membus.master
CacheConfig.config_cache(options, system)

### Create the root, instantiate the system, and run the simulation

root = Root(full_system = False, system = system)
Simulation.run(options, root, system, FutureClass)
