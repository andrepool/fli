-- Copyright 2012, 2013 Andre Pool
-- Licensed under the Apache License version 2.0
-- You may not use this file except in compliance with this License
-- You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

-- synchronous ram (currently only rom, which will lateron be filled through fli interface)

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity ram is
   port(
      clk : in boolean;
      read_address : in std_logic_vector( 9 downto 0 );
      read_data : out std_logic_vector( 31 downto 0 ) := (others => '0' )
   );
end;

architecture rtl of ram is
   type ram_t is array ( 0 to 1023 ) of std_logic_vector( 31 downto 0 );
   signal ram_s : ram_t := (others => (others => '0'));
begin
   process( clk )
   begin
      if clk'event and clk then
         read_data <= ram_s(  to_integer( unsigned( read_address ))) after 1 ns;
      end if;
   end process;
end;
