-- Copyright 2012, 2013 Andre Pool
-- Licensed under the Apache License version 2.0
-- You may not use this file except in compliance with this License
-- You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

-- 32 bits signed integer square root function which makes use of a "simple" c model
-- instead a "complicated" rtl implemenation.

entity sqrt_int is
   port(
      clk : in boolean;
      rst : in boolean;
      input : in integer range 0 to 1024;
      output : out integer range 0 to 1024 := 0
   );
end;

architecture c_model of sqrt_int is
   attribute foreign : string;
   attribute foreign of c_model :
      -- call sqrt_int_init function from c_environemnt.so file
      architecture is "sqrt_int_init ../src/c_environment.so";
begin
   -- architecture of function in c model
end;

