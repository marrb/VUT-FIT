-- uart_fsm.vhd: UART controller - finite state machine
-- Author(s): 
--
library ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.ALL;

-------------------------------------------------
entity UART_FSM is
port(
   CLK    		:  in std_logic;
   RST    		:  in std_logic;
	D_IN   		:  in std_logic;
	CNT    		:  in std_logic_vector(4 downto 0);
	CNT_2  		:  in integer;
	CNT_EN 		:  out std_logic;
	REC_EN 		:  out std_logic;
	DATA_VLD		:  out std_logic
   );
end entity UART_FSM;

-------------------------------------------------
architecture behavioral of UART_FSM is
type state is (EXPECTING_START_BIT, EXPECTING_FIRST_BIT, RECEIVING_DATA, EXPECTING_STOP_BIT, TRANSFER_SUCESS);
signal STE: state := EXPECTING_START_BIT;
begin
	REC_EN <= '1' when STE = RECEIVING_DATA else '0';
	CNT_EN <= '1' when STE = EXPECTING_FIRST_BIT or STE = RECEIVING_DATA or STE = EXPECTING_STOP_BIT else '0';
	DATA_VLD <= '1' when STE = TRANSFER_SUCESS else '0';
	process(CLK)
	begin
	if(rising_edge(CLK)) then
		if(RST = '1') then
			STE <= EXPECTING_START_BIT;
		else
			case STE is
				when (EXPECTING_START_BIT) =>
					if(D_IN = '0') then
						STE <= EXPECTING_FIRST_BIT;
					end if;
				when (EXPECTING_FIRST_BIT) =>
					if((to_integer(unsigned(CNT))) = 23) then
						STE <= RECEIVING_DATA;
					end if;
				when (RECEIVING_DATA) =>
					if(CNT_2 = 8) then
						STE <= EXPECTING_STOP_BIT;
					end if;
				when (EXPECTING_STOP_BIT) =>
					if((to_integer(unsigned(CNT))) = 16 and D_IN = '1') then
						STE <= TRANSFER_SUCESS;
					end if;
				when (TRANSFER_SUCESS) =>
					STE <= EXPECTING_START_BIT;
				when others => null;
			end case;
		end if;
	end if;
	end process;

end behavioral;