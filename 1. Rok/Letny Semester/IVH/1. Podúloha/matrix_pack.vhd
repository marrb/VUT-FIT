library IEEE;
use IEEE.STD_LOGIC_1164.all;

package matrix_pack is

type DIRECTION_T is (DIR_LEFT, DIR_RIGHT);

function GETID(ROWS : integer := 0;
					COLS : integer := 0;
					X : integer := 0;
					Y : integer := 0) return integer;

end matrix_pack;

package body matrix_pack is

function GETID(ROWS : integer := 0;
					COLS : integer := 0;
					X : integer := 0;
					Y : integer := 0) return integer is
					
					variable CalcX : integer := 0;
					variable CalcY : integer := 0;
					variable Result_Adress : integer := 0;
					
begin

	CalcY := Y mod ROWS;
	CalcX := X mod COLS;
		
	Result_Adress := (CalcX*ROWS)+CalcY;
		
	return Result_Adress;
	
end;
 
end matrix_pack;
