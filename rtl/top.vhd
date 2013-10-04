-- Copyright 2012, 2013 Andre Pool
-- Licensed under the Apache License version 2.0
-- You may not use this file except in compliance with this License
-- You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

-- The top level is the highest part in the verification level where the DUT is connected
-- with the stimuli generator and data analyser.

-- Because it is the highest level, it does not have any outside connections.

-- The clock of 100MHz is generated at this "time aware" vhdl level instead of the
-- timing "un aware" c level.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; -- for the read-data cast

entity top is

end top;

architecture sim of top is

  signal clk : boolean := true;
  signal rst : boolean;
  signal set : boolean;
  signal inc : boolean;
  signal dec : boolean;
  signal load : integer;
  signal result : integer;
  signal cmp : boolean;
  signal read_data : std_logic_vector(31 downto 0);
  signal read_data_int : integer;

begin
   clk <= not clk after 5 ns;
  
   -- testbench
   t0 : entity work.testbench(c_model)
      port map( clk, rst, set, inc, dec, load, result, cmp, read_data_int );

   -- dut              
   d0 : entity work.dut(rtl)
      port map( clk, rst, set, inc, dec, load, result, cmp, read_data );
   
   -- cast the std_logic_vector to integer with intermedia unsigned type
   read_data_int <= to_integer( unsigned ( read_data ) );

end;
