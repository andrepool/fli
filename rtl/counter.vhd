-- Copyright 2012, 2013 Andre Pool
-- Licensed under the Apache License version 2.0
-- You may not use this file except in compliance with this License
-- You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

-- Counter with control signals.
-- There are two architectures available for this counter:
--    foreign, where the behavior is implemented in an external c implementation
--    rtl implementation

entity counter is
   port(
      clk : in boolean;
      rst : in boolean;
      set : in boolean;
      inc : in boolean;
      dec : in boolean;
      load : in integer;
      cnt : out integer range 0 to 1024 := 0
   );
end;

architecture c_model of counter is
   attribute foreign : string;
   attribute foreign of c_model :
      -- counter_init is called in c_environment.so
      architecture is "counter_init ../src/c_environment.so";
begin
   -- architecture of function in c model
end;

architecture rtl of counter is
   signal cnt_q : integer range 0 to 1024 := 0;
begin
   process( clk )
   begin
      if clk'event and clk then
         if rst then
            cnt_q <= 0 after 1 ns;
         elsif set then
            cnt_q <= load after 1 ns;
         elsif inc then
            cnt_q <= cnt_q + 1 after 1 ns;
         elsif dec then
            cnt_q <= cnt_q - 1 after 1 ns;
         end if;
      end if;
   end process;
   cnt <= cnt_q after 1 ns;
end;
