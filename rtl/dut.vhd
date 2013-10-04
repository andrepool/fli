-- Copyright 2012, 2013 Andre Pool
-- Licensed under the Apache License version 2.0
-- You may not use this file except in compliance with this License
-- You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

-- This is the top part of the design we want to verify.
-- The dut contains a:
--   counter
--   sqrt function
--   comparator

-- The counter generates a number, from this number the sqrt is calculated and
-- the result of the sqrt is compared with a fixed value in the comparator.
-- If the output of the sqrt is higher then the fixed value the comparator returns one,
-- otherwise zero.

-- To demonstrate the foreign function usage, the counter and sqrt are using a c_model
-- instead of an rtl model. For the counter also an rtl model is available.
-- To switch back the counter to an rtl model, select the rtl architecture of the counter.

-- The counter can be controlled from the dut top level and the result of the sqrt and
-- comparator is also available on the top level.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; -- for read_address cast

entity dut is
   port(
      clk : in boolean;
      rst : in boolean;
      set : in boolean;
      inc : in boolean;
      dec : in boolean;
      load : in integer;
      result : out integer;
      cmp : out boolean;
      read_data : out std_logic_vector(31 downto 0)
   );
end;

architecture rtl of dut is
  signal cnt : integer range 0 to 1024;
  signal sqrt : integer range 0 to 1024;
  signal read_address : std_logic_vector( 9 downto 0 );

begin
 
   -- c0 : entity work.counter(rtl) -- select for vhdl implementation usage
   c0 : entity work.counter(c_model) -- select for c model usage
      port map( clk, rst, set, inc, dec, load, cnt );
      
   -- sqrt foreign function
   s0 : entity work.sqrt_int(c_model)
      port map( clk, rst, cnt, sqrt );

   -- comparator
   p0 : entity work.comparator(rtl)
      port map( clk, rst, sqrt, cmp );

   -- memory
   read_address <= std_logic_vector( to_unsigned( cnt, 10 ) );
   m0 : entity work.ram(rtl)
      port map( clk, read_address, read_data );

   result <= sqrt;

end;

