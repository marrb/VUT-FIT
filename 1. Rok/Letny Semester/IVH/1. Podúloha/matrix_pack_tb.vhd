LIBRARY ieee;
use ieee.std_logic_1164.ALL;

use work.matrix_pack.ALL;

ENTITY matrix_pack_tb IS
END matrix_pack_tb;

ARCHITECTURE behavior OF matrix_pack_tb IS 

signal Int_Comp : integer;
signal result : integer;

signal ROWS : integer := 2;
signal COLS : integer := 3;

signal X : integer := -1;
signal Y : integer := -1;

BEGIN

process is
begin

X <= -1;
Y <= -1;
Int_Comp <= 5;
wait for 10 ns;

X <= 0;
Y <= 0;
Int_Comp <= 0;
wait for 10 ns;

X <= 1;
Y <= 2;
Int_Comp <= 2;
wait for 10 ns;

X <= 2;
Y <= 0;
Int_Comp <= 4;
wait for 10 ns;

X <= 1;
Y <= -1;
Int_Comp <= 3;
wait for 10 ns;

X <= 3;
Y <= 2;
Int_Comp <= 0;
wait;

end process;

result <= GETID(ROWS,COLS,X,Y);
assert (result /= Int_Comp) report "Check passed, Adress = "& integer'image(result) severity note;


END;