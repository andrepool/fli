-- Copyright 2012, 2013 Andre Pool
-- Licensed under the Apache License version 2.0
-- You may not use this file except in compliance with this License
-- You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

-- Comparator function that compares a 32 bit integer value with a predefined value.
-- When the value is higher, the output is true, otherwise false.
-- The comparator does not toggle around a value but has a hysteresis.

entity comparator is
   port(
      clk : in boolean;
      rst : in boolean;
      input : in integer;
      cmp : out boolean := false
   );
end;

architecture rtl of comparator is
   signal cmp_q : boolean := false;
begin
   process( clk )
   begin
      if clk'event and clk then
         if rst then
            cmp_q <= false after 1 ns;
         elsif input > 10 then
            cmp_q <= true after 1 ns;
         elsif input < 7 then
            cmp_q <= false after 1 ns;
         end if;
      end if;
   end process;
   cmp <= cmp_q after 1 ns;
end;
