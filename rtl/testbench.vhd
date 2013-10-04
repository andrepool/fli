-- Copyright 2012, 2013 Andre Pool
-- Licensed under the Apache License version 2.0
-- You may not use this file except in compliance with this License
-- You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

-- The testbench generates the stimuli signals for the dut and also analyses the generated
-- output data from the dut.
-- In this case these stimuli data and dut response is processed by a c implementation
-- rather then an rtl implementation.

entity testbench is
   port(
      clk : in boolean;
      rst : out boolean := false;
      set : out boolean := false;
      inc : out boolean := false;
      dec : out boolean := false;
      load : out integer := 0;
      result : in integer;
      cmp : in boolean;
      read_data : in integer
   );
end;

architecture c_model of testbench is
   attribute foreign : string;
      -- testbench_init is called in c_environment.so
      attribute foreign of c_model : architecture is "testbench_init ../src/c_environment.so";
begin
   -- architecture of function in c model
end;
